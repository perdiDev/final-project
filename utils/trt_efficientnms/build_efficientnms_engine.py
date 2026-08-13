#!/usr/bin/env python3
"""Build a TensorRT engine with GPU-parallel EfficientNMS.

The input ONNX file is never modified.  By default the generated engine is
written next to it as ``<model>_efficientnms.engine``; this keeps an existing
baseline engine available for comparison.

The preferred input to EfficientNMS is a tensor with shape
``[batch, candidates, 4 + classes]`` containing decoded ``xyxy`` boxes followed
by one score per class.  Ultralytics YOLO exports in this repository contain
that tensor before their final ``max/argmax`` reduction, so it is discovered
and connected automatically.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Any, Iterable, Optional, Sequence, Tuple


DEFAULT_INPUT_SIZE = 640


def _load_tensor_rt() -> Tuple[Any, Any, Any]:
    """Import TensorRT lazily so ``--help`` works on non-Jetson machines."""

    try:
        import numpy as np
        import tensorrt as trt_module
    except ImportError as exc:  # pragma: no cover - depends on deployment host
        raise RuntimeError(
            "TensorRT Python dan NumPy wajib terpasang. Jalankan script ini "
            "di host NVIDIA/Jetson dengan TensorRT yang kompatibel."
        ) from exc

    trt: Any = trt_module
    logger = trt.Logger(trt.Logger.WARNING)
    if not trt.init_libnvinfer_plugins(logger, ""):
        raise RuntimeError("Gagal menginisialisasi plugin TensorRT bawaan.")
    return trt, np, logger


def _shape(tensor: Any) -> Tuple[int, ...]:
    return tuple(int(dimension) for dimension in tensor.shape)


def _all_tensors(network: Any) -> Iterable[Any]:
    """Yield every layer output, including tensors that are not graph outputs."""

    for layer_index in range(network.num_layers):
        layer = network.get_layer(layer_index)
        for output_index in range(layer.num_outputs):
            tensor = layer.get_output(output_index)
            if tensor is not None:
                yield tensor


def _find_tensor(network: Any, name: str) -> Any:
    for tensor in _all_tensors(network):
        if tensor.name == name:
            return tensor
    for input_index in range(network.num_inputs):
        tensor = network.get_input(input_index)
        if tensor.name == name:
            return tensor
    raise ValueError(f"Tensor '{name}' tidak ditemukan di network TensorRT.")


def _network_outputs(network: Any) -> list[Any]:
    return [network.get_output(index) for index in range(network.num_outputs)]


def _find_full_scores_tensor(
    network: Any, num_classes: int
) -> Tuple[Optional[Any], Optional[str]]:
    """Find [B,N,4+C] (or [B,4+C,N]) before YOLO's class reduction."""

    expected_channels = 4 + num_classes
    candidates: list[Tuple[int, Any, str]] = []

    for layer_index in range(network.num_layers):
        layer = network.get_layer(layer_index)
        for output_index in range(layer.num_outputs):
            tensor = layer.get_output(output_index)
            if tensor is None or len(_shape(tensor)) != 3:
                continue
            dimensions = _shape(tensor)
            if dimensions[2] == expected_channels and dimensions[1] > 0:
                # Prefer [B,N,C], which is what EfficientNMS_TRT consumes.
                candidates.append((0, tensor, "bnc"))
            elif dimensions[1] == expected_channels and dimensions[2] > 0:
                candidates.append((1, tensor, "bcn"))

    if not candidates:
        return None, None

    # A bnc candidate is preferred. For equal layouts, the latest tensor is
    # closest to the exported detection head and avoids obsolete branches.
    bnc = [candidate for candidate in candidates if candidate[2] == "bnc"]
    selected = bnc[-1] if bnc else candidates[-1]
    return selected[1], selected[2]


def _set_static_input_shape(
    network: Any, batch: int, requested_height: Optional[int], requested_width: Optional[int]
) -> Tuple[Any, int, int, int]:
    """Make the engine shape static for reliable Slice/plugin shape inference."""

    if network.num_inputs != 1:
        raise ValueError(
            f"Model harus memiliki tepat satu input, ditemukan {network.num_inputs}."
        )

    input_tensor = network.get_input(0)
    original = _shape(input_tensor)
    if len(original) != 4:
        raise ValueError(f"Input model harus NCHW, shape ditemukan: {original}.")

    original_batch, channels, original_height, original_width = original
    if original_batch > 0 and original_batch != batch:
        raise ValueError(
            f"Model memiliki batch statis {original_batch}, tetapi --batch={batch}."
        )
    if channels <= 0:
        raise ValueError("Dimensi channel input harus statis.")

    height = requested_height or (
        original_height if original_height > 0 else DEFAULT_INPUT_SIZE
    )
    width = requested_width or (original_width if original_width > 0 else DEFAULT_INPUT_SIZE)
    if height <= 0 or width <= 0:
        raise ValueError("Ukuran input harus lebih besar dari nol.")
    if original_height > 0 and original_height != height:
        raise ValueError(
            f"Model memiliki tinggi input statis {original_height}, tetapi --height={height}."
        )
    if original_width > 0 and original_width != width:
        raise ValueError(
            f"Model memiliki lebar input statis {original_width}, tetapi --width={width}."
        )

    try:
        input_tensor.shape = (batch, channels, height, width)
    except Exception as exc:
        raise ValueError(
            f"Tidak dapat menetapkan shape input {(batch, channels, height, width)}."
        ) from exc
    return input_tensor, channels, height, width


def _slice(
    network: Any, tensor: Any, start: Sequence[int], length: int
) -> Any:
    dimensions = _shape(tensor)
    if len(dimensions) != 3 or any(dimension <= 0 for dimension in dimensions):
        raise ValueError(
            f"Slice membutuhkan shape statis [B,N,C], tetapi ditemukan {dimensions}."
        )
    output = network.add_slice(
        tensor,
        tuple(int(value) for value in start),
        (dimensions[0], dimensions[1], length),
        (1, 1, 1),
    )
    if output is None:
        raise RuntimeError(f"TensorRT gagal membuat Slice untuk tensor '{tensor.name}'.")
    return output.get_output(0)


def _transpose_bcn_to_bnc(network: Any, tensor: Any) -> Any:
    shuffle = network.add_shuffle(tensor)
    if shuffle is None:
        raise RuntimeError("TensorRT gagal membuat transpose tensor detection head.")
    shuffle.first_transpose = (0, 2, 1)
    return shuffle.get_output(0)


def _make_decoded6_scores(
    network: Any, trt: Any, np: Any, decoded: Any, num_classes: int
) -> Tuple[Any, Any]:
    """Convert [xyxy, score, class_id] to [boxes, B,N,C] for EfficientNMS."""

    dimensions = _shape(decoded)
    if len(dimensions) != 3 or dimensions[2] != 6:
        raise ValueError(
            "Format decoded6 membutuhkan output [batch, candidates, 6] "
            f"([x1,y1,x2,y2,score,class_id]); ditemukan {dimensions}."
        )
    boxes = _slice(network, decoded, (0, 0, 0), 4)
    scores_one = _slice(network, decoded, (0, 0, 4), 1)
    class_ids = _slice(network, decoded, (0, 0, 5), 1)

    # EfficientNMS accepts scores per class, not a separate class-id tensor.
    # Build a one-hot score matrix on GPU. Non-matching classes receive a value
    # below any practical threshold and are therefore ignored by the plugin.
    score_columns = []
    constant_shape = (dimensions[0], dimensions[1], 1)
    for class_id in range(num_classes):
        class_constant = network.add_constant(
            constant_shape,
            np.full(constant_shape, float(class_id), dtype=np.float32),
        )
        negative_score = network.add_constant(
            constant_shape,
            np.full(constant_shape, -10000.0, dtype=np.float32),
        )
        equal = network.add_elementwise(
            class_ids, class_constant.get_output(0), trt.ElementWiseOperation.EQUAL
        )
        selected = network.add_select(
            equal.get_output(0), scores_one, negative_score.get_output(0)
        )
        score_columns.append(selected.get_output(0))

    concatenate = network.add_concatenation(score_columns)
    concatenate.axis = 2
    return boxes, concatenate.get_output(0)


def _create_plugin(
    trt: Any,
    np: Any,
    score_threshold: float,
    iou_threshold: float,
    max_output_boxes: int,
    scores_are_logits: bool,
    box_coding: int,
    class_agnostic: bool,
) -> Any:
    registry = trt.get_plugin_registry()
    creator = registry.get_plugin_creator("EfficientNMS_TRT", "1", "")
    if creator is None:
        raise RuntimeError(
            "Plugin EfficientNMS_TRT tidak tersedia. Pastikan libnvinfer_plugin "
            "dengan plugin TensorRT terpasang dan sudah diinisialisasi."
        )

    available = {field.name for field in creator.field_names}

    def field(name: str, value: Any, field_type: Any) -> Any:
        if name not in available:
            raise RuntimeError(
                f"Plugin EfficientNMS_TRT tidak memiliki field '{name}'. "
                f"Field tersedia: {sorted(available)}"
            )
        if field_type == trt.PluginFieldType.FLOAT32:
            data = np.asarray([value], dtype=np.float32)
        else:
            data = np.asarray([value], dtype=np.int32)
        return trt.PluginField(name, data, field_type)

    fields = [
        field("score_threshold", score_threshold, trt.PluginFieldType.FLOAT32),
        field("iou_threshold", iou_threshold, trt.PluginFieldType.FLOAT32),
        field("max_output_boxes", max_output_boxes, trt.PluginFieldType.INT32),
        field("background_class", -1, trt.PluginFieldType.INT32),
        field(
            "score_activation",
            int(scores_are_logits),
            trt.PluginFieldType.INT32,
        ),
        field("box_coding", box_coding, trt.PluginFieldType.INT32),
    ]
    if "class_agnostic" in available:
        fields.append(
            field("class_agnostic", int(class_agnostic), trt.PluginFieldType.INT32)
        )

    plugin = creator.create_plugin(
        "efficient_nms", trt.PluginFieldCollection(fields)
    )
    if plugin is None:
        raise RuntimeError("TensorRT gagal membuat instance plugin EfficientNMS_TRT.")
    return plugin


def _prepare_detection_inputs(
    network: Any,
    trt: Any,
    np: Any,
    num_classes: int,
    input_format: str,
    output_name: Optional[str],
    boxes_name: Optional[str],
    scores_name: Optional[str],
) -> Tuple[Any, Any, str]:
    if input_format in ("auto", "full_scores"):
        full_scores, layout = _find_full_scores_tensor(network, num_classes)
        if full_scores is not None:
            if layout == "bcn":
                full_scores = _transpose_bcn_to_bnc(network, full_scores)
            boxes = _slice(network, full_scores, (0, 0, 0), 4)
            scores = _slice(network, full_scores, (0, 0, 4), num_classes)
            return boxes, scores, f"full_scores:{full_scores.name}"
        if input_format == "full_scores":
            raise ValueError(
                f"Tidak menemukan tensor [B,N,{4 + num_classes}] pada model."
            )

    if input_format in ("auto", "decoded6"):
        if output_name:
            decoded = _find_tensor(network, output_name)
        else:
            outputs = _network_outputs(network)
            if not outputs:
                raise ValueError("Model tidak memiliki output.")
            decoded = outputs[0]
        if len(_shape(decoded)) == 3 and _shape(decoded)[2] == 6:
            boxes, scores = _make_decoded6_scores(
                network, trt, np, decoded, num_classes
            )
            return boxes, scores, f"decoded6:{decoded.name}"
        if input_format == "decoded6":
            raise ValueError(
                "Format decoded6 membutuhkan output [B,N,6], "
                f"tetapi ditemukan {_shape(decoded)}."
            )

    if input_format == "boxes_scores":
        if not boxes_name or not scores_name:
            raise ValueError(
                "--boxes-name dan --scores-name wajib diisi untuk --input-format boxes_scores."
            )
        boxes = _find_tensor(network, boxes_name)
        scores = _find_tensor(network, scores_name)
        if _shape(boxes)[-1] != 4:
            raise ValueError(f"Boxes harus berakhir dengan 4, ditemukan {_shape(boxes)}.")
        if _shape(scores)[-1] != num_classes:
            raise ValueError(
                f"Scores harus berakhir dengan {num_classes}, ditemukan {_shape(scores)}."
            )
        return boxes, scores, f"boxes_scores:{boxes.name},{scores.name}"

    raise ValueError(
        "Tidak dapat menemukan input EfficientNMS otomatis. Gunakan --input-format "
        "full_scores, decoded6, atau boxes_scores."
    )


def _load_network(trt: Any, logger: Any, model_path: Path) -> Tuple[Any, Any, Any]:
    builder = trt.Builder(logger)
    flags = 1 << int(trt.NetworkDefinitionCreationFlag.EXPLICIT_BATCH)
    network = builder.create_network(flags)
    parser = trt.OnnxParser(network, logger)
    if not parser.parse_from_file(str(model_path)):
        errors = []
        for index in range(parser.num_errors):
            errors.append(str(parser.get_error(index)))
        details = "\n".join(errors) or "pesan parser tidak tersedia"
        raise RuntimeError(f"Gagal parse ONNX '{model_path}':\n{details}")
    return builder, network, parser


def _inspect(args: argparse.Namespace) -> int:
    trt, _, logger = _load_tensor_rt()
    _, network, _ = _load_network(trt, logger, args.model)
    input_tensor, channels, height, width = _set_static_input_shape(
        network, args.batch, args.height, args.width
    )
    print(f"input       : {input_tensor.name} {_shape(input_tensor)}")
    print("network outputs:")
    for tensor in _network_outputs(network):
        print(f"  - {tensor.name}: {_shape(tensor)}")
    full_scores, layout = _find_full_scores_tensor(network, args.num_classes)
    if full_scores is not None:
        print(
            f"full scores : {full_scores.name} {_shape(full_scores)} layout={layout} "
            f"(classes={args.num_classes})"
        )
    else:
        print("full scores : tidak ditemukan")
    print(
        f"input size  : {channels}x{height}x{width}, batch={args.batch}\n"
        "Tidak ada engine yang ditulis (mode --inspect)."
    )
    return 0


def _build(args: argparse.Namespace) -> int:
    if args.output.resolve() == args.model.resolve():
        raise ValueError("Output engine tidak boleh menimpa file ONNX sumber.")
    if args.output.exists() and not args.force:
        raise FileExistsError(
            f"Output '{args.output}' sudah ada. Gunakan --force jika memang ingin "
            "mengganti file tersebut; baseline tidak diubah secara default."
        )

    trt, np, logger = _load_tensor_rt()
    builder, network, _ = _load_network(trt, logger, args.model)
    input_tensor, _, _, _ = _set_static_input_shape(
        network, args.batch, args.height, args.width
    )

    boxes, scores, source = _prepare_detection_inputs(
        network,
        trt,
        np,
        args.num_classes,
        args.input_format,
        args.output_name,
        args.boxes_name,
        args.scores_name,
    )
    if len(_shape(boxes)) != 3 or len(_shape(scores)) != 3:
        raise ValueError(
            f"EfficientNMS membutuhkan boxes/scores rank 3; "
            f"ditemukan {_shape(boxes)} dan {_shape(scores)}."
        )
    if _shape(boxes)[0:2] != _shape(scores)[0:2]:
        raise ValueError(
            f"Jumlah candidate boxes dan scores tidak sama: {_shape(boxes)} vs {_shape(scores)}."
        )

    plugin = _create_plugin(
        trt,
        np,
        args.score_threshold,
        args.iou_threshold,
        args.max_output_boxes,
        args.scores_are_logits,
        args.box_coding,
        args.class_agnostic,
    )
    nms_layer = network.add_plugin_v2([boxes, scores], plugin)
    if nms_layer is None:
        raise RuntimeError("TensorRT gagal menambahkan layer EfficientNMS_TRT.")
    if nms_layer.num_outputs < 4:
        raise RuntimeError(
            f"EfficientNMS_TRT mengembalikan {nms_layer.num_outputs} output, minimal 4."
        )

    # Remove all original model outputs. The final engine exposes only the
    # standard EfficientNMS tuple and therefore no CPU NMS is needed.
    for old_output in _network_outputs(network):
        network.unmark_output(old_output)
    output_names = (
        "num_detections",
        "detection_boxes",
        "detection_scores",
        "detection_classes",
    )
    for index, name in enumerate(output_names):
        output = nms_layer.get_output(index)
        output.name = name
        network.mark_output(output)

    config = builder.create_builder_config()
    config.profiling_verbosity = trt.ProfilingVerbosity.DETAILED 
    workspace_bytes = int(args.workspace_gib * (1024**3))
    if workspace_bytes <= 0:
        raise ValueError("--workspace-gib harus lebih besar dari nol.")
    config.set_memory_pool_limit(trt.MemoryPoolType.WORKSPACE, workspace_bytes)
    if args.fp16:
        config.set_flag(trt.BuilderFlag.FP16)

    print(
        f"Build EfficientNMS: model={args.model.name}, source={source}, "
        f"input={_shape(input_tensor)}, classes={args.num_classes}, "
        f"precision={'FP16' if args.fp16 else 'FP32'}"
    )
    serialized = builder.build_serialized_network(network, config)
    if serialized is None:
        raise RuntimeError(
            "TensorRT gagal membangun engine. Periksa log TensorRT, GPU, dan kecocokan plugin."
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(bytes(serialized))
    print(f"Engine EfficientNMS tersimpan: {args.output}")
    print("ONNX dan engine baseline tetap dipertahankan.")
    return 0


def _default_output(model: Path) -> Path:
    return model.with_name(f"{model.stem}_efficientnms.engine")


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Tambahkan EfficientNMS_TRT GPU-parallel ke model ONNX tanpa mengubah baseline."
    )
    parser.add_argument("model", type=Path, help="Path model ONNX sumber.")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Path engine keluaran (default: <model>_efficientnms.engine).",
    )
    parser.add_argument(
        "--num-classes",
        type=int,
        default=3,
        help="Jumlah kelas detection (default: 3 untuk model KITTI repo ini).",
    )
    parser.add_argument("--batch", type=int, default=1, help="Batch engine statis (default: 1).")
    parser.add_argument("--height", type=int, help="Tinggi input; default memakai shape ONNX.")
    parser.add_argument("--width", type=int, help="Lebar input; default memakai shape ONNX.")
    parser.add_argument(
        "--input-format",
        choices=("auto", "full_scores", "decoded6", "boxes_scores"),
        default="auto",
        help="Format input ke NMS; auto mendahulukan tensor head YOLO [B,N,4+C].",
    )
    parser.add_argument(
        "--output-name",
        help="Nama tensor [B,N,6] untuk format decoded6 (default: output pertama).",
    )
    parser.add_argument("--boxes-name", help="Nama tensor boxes [B,N,4].")
    parser.add_argument("--scores-name", help="Nama tensor scores [B,N,C].")
    parser.add_argument(
        "--scores-are-logits",
        action="store_true",
        help="Aktifkan sigmoid di plugin; default menganggap scores sudah probabilitas.",
    )
    parser.add_argument("--score-threshold", type=float, default=0.05)
    parser.add_argument("--iou-threshold", type=float, default=0.45)
    parser.add_argument("--max-output-boxes", type=int, default=300)
    parser.add_argument(
        "--box-coding",
        type=int,
        choices=(0, 1),
        default=0,
        help="0=corner/xyxy (default), 1=center-size.",
    )
    parser.add_argument(
        "--class-agnostic",
        action="store_true",
        help="NMS lintas kelas; default NMS dilakukan per kelas.",
    )
    parser.add_argument("--workspace-gib", type=float, default=4.0)
    precision = parser.add_mutually_exclusive_group()
    precision.add_argument("--fp16", dest="fp16", action="store_true", default=True)
    precision.add_argument("--fp32", dest="fp16", action="store_false")
    parser.add_argument(
        "--inspect",
        action="store_true",
        help="Parse dan tampilkan tensor detection tanpa membangun/menulis engine.",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Izinkan mengganti output yang sudah ada; tidak pernah mengubah ONNX sumber.",
    )
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)
    if not args.model.is_file():
        parser.error(f"Model tidak ditemukan: {args.model}")
    if args.num_classes <= 0:
        parser.error("--num-classes harus lebih besar dari nol.")
    if args.batch <= 0:
        parser.error("--batch harus lebih besar dari nol.")
    if args.max_output_boxes <= 0:
        parser.error("--max-output-boxes harus lebih besar dari nol.")
    if not 0.0 <= args.score_threshold:
        parser.error("--score-threshold tidak boleh negatif.")
    if not 0.0 <= args.iou_threshold <= 1.0:
        parser.error("--iou-threshold harus berada di antara 0 dan 1.")
    args.output = args.output or _default_output(args.model)

    try:
        return _inspect(args) if args.inspect else _build(args)
    except (FileExistsError, RuntimeError, ValueError, OSError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

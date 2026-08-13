"""Hitung mAP "as-deployed" dari dump deteksi mentah pipeline DeepStream.

Alur:
  1. scripts/prepare_eval_video.sh  -> data/eval/kitti_val.mp4 + manifest.csv
  2. ./build/app --dump-detections  -> <model>_detections.jsonl (satu run per model)
  3. script ini                    -> convert ke COCO json + pycocotools.COCOeval

Bbox pada detections.jsonl berada dalam koordinat kanvas nvstreammux (default
1280x720, lihat kStreamWidth/kStreamHeight di src/main.cpp) karena nvstreammux
melakukan stretch (bukan letterbox) ke resolusi tersebut. Script ini
merescale bbox tersebut kembali ke resolusi gambar KITTI asli per frame
(dari manifest.csv) sebelum dibandingkan ke ground truth.

Lihat docs/03_deployment_pipeline.md §3.4 dan docs/05_accuracy_results.md.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path
from typing import Dict, List, Optional, Sequence


def load_manifest(path: Path) -> List[Dict]:
    rows = []
    with path.open("r", newline="") as f:
        for row in csv.DictReader(f):
            rows.append(
                {
                    "frame_index": int(row["frame_index"]),
                    "filename": row["filename"],
                    "orig_width": int(row["orig_width"]),
                    "orig_height": int(row["orig_height"]),
                }
            )
    rows.sort(key=lambda r: r["frame_index"])
    return rows


def load_detections(path: Path) -> Dict[int, List[Dict]]:
    detections_by_frame: Dict[int, List[Dict]] = {}
    with path.open("r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            record = json.loads(line)
            detections_by_frame[record["frame_num"]] = record["detections"]
    return detections_by_frame


def load_classes(labels_file: Path) -> List[str]:
    with labels_file.open("r") as f:
        return [line.strip() for line in f if line.strip()]


def load_yolo_label(label_path: Path, orig_width: int, orig_height: int) -> List[Dict]:
    if not label_path.exists():
        return []

    boxes = []
    with label_path.open("r") as f:
        for line in f:
            parts = line.split()
            if len(parts) < 5:
                continue
            class_id = int(float(parts[0]))
            cx, cy, w, h = (float(v) for v in parts[1:5])

            abs_w = w * orig_width
            abs_h = h * orig_height
            abs_x = (cx * orig_width) - (abs_w / 2.0)
            abs_y = (cy * orig_height) - (abs_h / 2.0)
            boxes.append({"class_id": class_id, "bbox": [abs_x, abs_y, abs_w, abs_h]})
    return boxes


def build_coco_ground_truth(
    manifest: List[Dict], labels_dir: Path, classes: List[str]
) -> Dict:
    images = []
    annotations = []
    categories = [{"id": i, "name": name} for i, name in enumerate(classes)]

    annotation_id = 1
    for row in manifest:
        image_id = row["frame_index"]
        images.append(
            {
                "id": image_id,
                "file_name": row["filename"],
                "width": row["orig_width"],
                "height": row["orig_height"],
            }
        )

        label_path = labels_dir / (Path(row["filename"]).stem + ".txt")
        for box in load_yolo_label(label_path, row["orig_width"], row["orig_height"]):
            x, y, w, h = box["bbox"]
            annotations.append(
                {
                    "id": annotation_id,
                    "image_id": image_id,
                    "category_id": box["class_id"],
                    "bbox": [x, y, w, h],
                    "area": w * h,
                    "iscrowd": 0,
                }
            )
            annotation_id += 1

    return {"images": images, "annotations": annotations, "categories": categories}


def build_coco_detections(
    manifest: List[Dict],
    detections_by_frame: Dict[int, List[Dict]],
    stream_width: int,
    stream_height: int,
) -> List[Dict]:
    results = []
    missing_frames = 0
    for row in manifest:
        image_id = row["frame_index"]
        frame_detections = detections_by_frame.get(image_id)
        if frame_detections is None:
            missing_frames += 1
            continue

        scale_x = row["orig_width"] / stream_width
        scale_y = row["orig_height"] / stream_height
        for det in frame_detections:
            results.append(
                {
                    "image_id": image_id,
                    "category_id": det["class_id"],
                    "bbox": [
                        det["left"] * scale_x,
                        det["top"] * scale_y,
                        det["width"] * scale_x,
                        det["height"] * scale_y,
                    ],
                    "score": det["confidence"],
                }
            )

    if missing_frames:
        print(
            f"[WARN] {missing_frames} frame di manifest tidak ada di detections.jsonl "
            "(dianggap tanpa deteksi apapun -- cek jumlah frame video vs jumlah gambar).",
            file=sys.stderr,
        )
    return results


def run_coco_eval(gt: Dict, detections: List[Dict], classes: List[str]) -> None:
    try:
        from pycocotools.coco import COCO
        from pycocotools.cocoeval import COCOeval
    except ImportError as error:
        raise SystemExit(
            "pycocotools tidak terpasang. Install dengan: pip install pycocotools"
        ) from error

    coco_gt = COCO()
    coco_gt.dataset = gt
    coco_gt.createIndex()

    if not detections:
        raise SystemExit("Tidak ada deteksi untuk dievaluasi (detections.jsonl kosong?).")
    coco_dt = coco_gt.loadRes(detections)

    print("\n=== Keseluruhan (semua kelas) ===")
    coco_eval = COCOeval(coco_gt, coco_dt, iouType="bbox")
    coco_eval.evaluate()
    coco_eval.accumulate()
    coco_eval.summarize()

    for class_id, class_name in enumerate(classes):
        print(f"\n=== Per kelas: {class_name} (class_id={class_id}) ===")
        per_class_eval = COCOeval(coco_gt, coco_dt, iouType="bbox")
        per_class_eval.params.catIds = [class_id]
        per_class_eval.evaluate()
        per_class_eval.accumulate()
        per_class_eval.summarize()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True, help="manifest.csv dari prepare_eval_video.sh")
    parser.add_argument("--detections", type=Path, required=True, help="<model>_detections.jsonl dari --dump-detections")
    parser.add_argument("--labels-dir", type=Path, required=True, help="Folder label YOLO txt val KITTI (ground truth)")
    parser.add_argument(
        "--classes-file",
        type=Path,
        default=Path("labels/labels_kitti_custom.txt"),
        help="File daftar nama kelas, satu per baris (default: labels/labels_kitti_custom.txt)",
    )
    parser.add_argument("--stream-width", type=int, default=1280, help="Lebar kanvas nvstreammux (kStreamWidth di main.cpp)")
    parser.add_argument("--stream-height", type=int, default=720, help="Tinggi kanvas nvstreammux (kStreamHeight di main.cpp)")
    parser.add_argument("--save-coco-gt", type=Path, default=None, help="Opsional: simpan ground truth COCO json ke path ini")
    parser.add_argument("--save-coco-dt", type=Path, default=None, help="Opsional: simpan deteksi COCO json ke path ini")
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)

    manifest = load_manifest(args.manifest)
    detections_by_frame = load_detections(args.detections)
    classes = load_classes(args.classes_file)

    gt = build_coco_ground_truth(manifest, args.labels_dir, classes)
    detections = build_coco_detections(
        manifest, detections_by_frame, args.stream_width, args.stream_height
    )

    if args.save_coco_gt is not None:
        args.save_coco_gt.write_text(json.dumps(gt))
    if args.save_coco_dt is not None:
        args.save_coco_dt.write_text(json.dumps(detections))

    run_coco_eval(gt, detections, classes)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

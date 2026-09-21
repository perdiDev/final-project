#!/bin/bash
# ==============================================================================
# Build TensorRT EfficientNMS engines (FP16 & FP32) untuk model KITTI
# ------------------------------------------------------------------------------
# Wrapper di atas utils/trt_efficientnms/build_efficientnms_engine.py.
#
# - HARUS dijalankan di Jetson (butuh TensorRT + Python bindings + GPU).
# - ONNX sumber di models/ TIDAK PERNAH diubah (dijamin oleh script python).
# - Idempotent: jika file .engine tujuan SUDAH ADA, build dilewati (skip) agar
#   script ini aman dijalankan berulang kali tanpa membuang waktu build ulang.
#   Gunakan --force untuk sengaja membangun ulang (menimpa) engine yang sudah ada.
#
# Penamaan engine mengikuti config/pgie_*_efficientnms*.txt:
#   models/<model>_efficientnms.engine        -> FP16 (baseline)
#   models/<model>_efficientnms_fp32.engine   -> FP32 (pembanding)
#
# Contoh pemakaian:
#   ./scripts/build_efficientnms_engines.sh                       # build semua (fp16+fp32) yang belum ada
#   ./scripts/build_efficientnms_engines.sh --list                # lihat status tiap kombinasi lalu keluar
#   ./scripts/build_efficientnms_engines.sh --model yolov8n_kitti # hanya satu model, kedua precision
#   ./scripts/build_efficientnms_engines.sh --precision fp32      # hanya precision fp32, semua model
#   ./scripts/build_efficientnms_engines.sh --force                # build ulang walau sudah ada
# ==============================================================================

set -u

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

MODELS_DIR="models"
BUILD_SCRIPT="utils/trt_efficientnms/build_efficientnms_engine.py"

# Model KITTI yang punya varian EfficientNMS, lihat config/pgie_<model>_efficientnms*.txt
MODELS=("yolov8n_kitti" "yolov9t_kitti")
PRECISIONS=("fp16" "fp32")

FORCE=0
ONLY_MODEL=""
ONLY_PRECISION=""
LIST_ONLY=0

print_usage() {
    cat <<EOF
Penggunaan: $0 [opsi]

  --model <nama>            Hanya proses model tertentu (contoh: yolov8n_kitti)
  --precision <fp16|fp32>   Hanya proses precision tertentu
  --force                   Build ulang walau file engine sudah ada (menimpa)
  --list                    Tampilkan status tiap kombinasi model/precision lalu keluar
  -h, --help                Tampilkan bantuan ini

Model yang didukung: ${MODELS[*]}
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --model) ONLY_MODEL="${2:-}"; shift 2 ;;
        --precision) ONLY_PRECISION="${2:-}"; shift 2 ;;
        --force) FORCE=1; shift ;;
        --list) LIST_ONLY=1; shift ;;
        -h|--help) print_usage; exit 0 ;;
        *) echo "[ERROR] Opsi tidak dikenal: $1"; print_usage; exit 1 ;;
    esac
done

if [ -n "$ONLY_PRECISION" ] && [[ "$ONLY_PRECISION" != "fp16" && "$ONLY_PRECISION" != "fp32" ]]; then
    echo "[ERROR] --precision harus 'fp16' atau 'fp32'."
    exit 1
fi

if [ -n "$ONLY_MODEL" ]; then
    KNOWN=0
    for m in "${MODELS[@]}"; do
        [ "$m" == "$ONLY_MODEL" ] && KNOWN=1
    done
    if [ "$KNOWN" -eq 0 ]; then
        echo "[ERROR] Model tidak dikenal: $ONLY_MODEL"
        echo "        Model yang didukung: ${MODELS[*]}"
        exit 1
    fi
fi

# ==============================================================================
# HELPER
# ==============================================================================
engine_path_for() {
    local model="$1"
    local precision="$2"
    if [ "$precision" == "fp16" ]; then
        echo "$MODELS_DIR/${model}_efficientnms.engine"
    else
        echo "$MODELS_DIR/${model}_efficientnms_${precision}.engine"
    fi
}

# ==============================================================================
# MODE --list
# ==============================================================================
if [ "$LIST_ONLY" -eq 1 ]; then
    echo "Status engine EfficientNMS:"
    for MODEL in "${MODELS[@]}"; do
        if [ -n "$ONLY_MODEL" ] && [ "$MODEL" != "$ONLY_MODEL" ]; then
            continue
        fi
        for PRECISION in "${PRECISIONS[@]}"; do
            if [ -n "$ONLY_PRECISION" ] && [ "$PRECISION" != "$ONLY_PRECISION" ]; then
                continue
            fi
            ENGINE_FILE="$(engine_path_for "$MODEL" "$PRECISION")"
            if [ -f "$ENGINE_FILE" ]; then
                echo "  [ADA]     $ENGINE_FILE"
            else
                echo "  [BELUM]   $ENGINE_FILE"
            fi
        done
    done
    exit 0
fi

# ==============================================================================
# BUILD
# ==============================================================================
BUILT_COUNT=0
SKIPPED_COUNT=0
FAILED_COUNT=0

for MODEL in "${MODELS[@]}"; do
    if [ -n "$ONLY_MODEL" ] && [ "$MODEL" != "$ONLY_MODEL" ]; then
        continue
    fi

    ONNX_FILE="$MODELS_DIR/${MODEL}.onnx"
    if [ ! -f "$ONNX_FILE" ]; then
        echo "[SKIP] $MODEL: ONNX sumber tidak ditemukan ($ONNX_FILE)"
        continue
    fi

    for PRECISION in "${PRECISIONS[@]}"; do
        if [ -n "$ONLY_PRECISION" ] && [ "$PRECISION" != "$ONLY_PRECISION" ]; then
            continue
        fi

        ENGINE_FILE="$(engine_path_for "$MODEL" "$PRECISION")"

        echo "----------------------------------------------------------------------"
        if [ -f "$ENGINE_FILE" ] && [ "$FORCE" -eq 0 ]; then
            echo "[SKIP] $ENGINE_FILE sudah ada. Gunakan --force untuk membangun ulang."
            SKIPPED_COUNT=$((SKIPPED_COUNT + 1))
            continue
        fi

        echo "[BUILD] model=$MODEL precision=$PRECISION"
        echo "        onnx   = $ONNX_FILE"
        echo "        output = $ENGINE_FILE"

        PY_ARGS=("$ONNX_FILE" --output "$ENGINE_FILE" "--$PRECISION")
        if [ -f "$ENGINE_FILE" ] && [ "$FORCE" -eq 1 ]; then
            PY_ARGS+=(--force)
        fi

        if python3 "$BUILD_SCRIPT" "${PY_ARGS[@]}"; then
            echo "[OK] $ENGINE_FILE berhasil dibangun."
            BUILT_COUNT=$((BUILT_COUNT + 1))
        else
            echo "[ERROR] Gagal membangun $ENGINE_FILE."
            FAILED_COUNT=$((FAILED_COUNT + 1))
        fi
    done
done

echo "----------------------------------------------------------------------"
echo "Selesai. Dibangun: $BUILT_COUNT, Dilewati (sudah ada): $SKIPPED_COUNT, Gagal: $FAILED_COUNT"

if [ "$FAILED_COUNT" -gt 0 ]; then
    exit 1
fi
exit 0

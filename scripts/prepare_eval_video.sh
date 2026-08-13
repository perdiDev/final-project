#!/bin/bash
# ==============================================================================
# ADAS Perception - Persiapan Video Evaluasi mAP As-Deployed
# ------------------------------------------------------------------------------
# Mengubah folder gambar val KITTI (mis. hasil ekspor dari notebook Kaggle)
# menjadi SATU file video lossless + manifest.csv, supaya bisa diputar lewat
# `--input file` pada aplikasi utama (lihat src/main.cpp) dan dideteksi dengan
# `--dump-detections`, lalu dievaluasi dengan utils/eval_map/eval_deepstream_map.py.
#
# Urutan frame dijaga deterministik (sort alfabetis nama file) dan dicatat di
# manifest.csv supaya bisa dipetakan balik ke ground truth per gambar.
#
# Kenapa video, bukan image-sequence langsung: aplikasi utama sudah mendukung
# `--input file` via uridecodebin tanpa perubahan; ini menghindari perlu
# membangun dukungan input baru di pipeline C++.
#
# Contoh pemakaian:
#   ./scripts/prepare_eval_video.sh --images-dir data/eval/kitti_val/images
#   ./scripts/prepare_eval_video.sh --images-dir data/eval/kitti_val/images \
#       --output-video data/eval/kitti_val.mp4 --manifest data/eval/manifest.csv
# ==============================================================================

set -eu

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

IMAGES_DIR=""
OUTPUT_VIDEO="data/eval/kitti_val.mp4"
MANIFEST="data/eval/manifest.csv"
FRAMERATE=5

print_usage() {
    cat <<EOF
Penggunaan: $0 --images-dir <folder> [opsi]

  --images-dir <folder>   Folder berisi gambar val (.jpg/.jpeg/.png), wajib diisi
  --output-video <path>   Path video output (default: $OUTPUT_VIDEO)
  --manifest <path>       Path manifest CSV output (default: $MANIFEST)
  --framerate <fps>       Framerate video kontainer, tidak mempengaruhi kualitas
                          deteksi per-frame (default: $FRAMERATE)
  -h, --help              Tampilkan bantuan ini

Manifest CSV berisi: frame_index,filename,orig_width,orig_height
- frame_index dimulai dari 0, urut sesuai frame_num yang dilaporkan aplikasi utama.
- orig_width/orig_height dipakai utils/eval_map/eval_deepstream_map.py untuk
  merescale bbox dari kanvas streammux (1280x720) balik ke resolusi gambar asli.
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --images-dir) IMAGES_DIR="${2:-}"; shift 2 ;;
        --output-video) OUTPUT_VIDEO="${2:-}"; shift 2 ;;
        --manifest) MANIFEST="${2:-}"; shift 2 ;;
        --framerate) FRAMERATE="${2:-}"; shift 2 ;;
        -h|--help) print_usage; exit 0 ;;
        *) echo "[ERROR] Opsi tidak dikenal: $1"; print_usage; exit 1 ;;
    esac
done

if [ -z "$IMAGES_DIR" ]; then
    echo "[ERROR] --images-dir wajib diisi."
    print_usage
    exit 1
fi
if [ ! -d "$IMAGES_DIR" ]; then
    echo "[ERROR] Folder gambar tidak ditemukan: $IMAGES_DIR"
    exit 1
fi

command -v ffmpeg >/dev/null 2>&1 || { echo "[ERROR] ffmpeg tidak ditemukan di PATH."; exit 1; }
command -v ffprobe >/dev/null 2>&1 || { echo "[ERROR] ffprobe tidak ditemukan di PATH."; exit 1; }

mkdir -p "$(dirname "$OUTPUT_VIDEO")"
mkdir -p "$(dirname "$MANIFEST")"

# ==============================================================================
# 1. Kumpulkan & urutkan gambar secara deterministik (sort alfabetis)
# ==============================================================================
mapfile -t IMAGE_FILES < <(find "$IMAGES_DIR" -maxdepth 1 -type f \
    \( -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' \) | sort)

if [ ${#IMAGE_FILES[@]} -eq 0 ]; then
    echo "[ERROR] Tidak ada gambar .jpg/.jpeg/.png di: $IMAGES_DIR"
    exit 1
fi

echo "[INFO] Ditemukan ${#IMAGE_FILES[@]} gambar di $IMAGES_DIR"

# ==============================================================================
# 2. Tulis manifest.csv (frame_index,filename,orig_width,orig_height) sekaligus
#    siapkan concat-list ffmpeg dengan urutan yang identik
# ==============================================================================
CONCAT_LIST="$(mktemp)"
trap 'rm -f "$CONCAT_LIST"' EXIT

echo "frame_index,filename,orig_width,orig_height" > "$MANIFEST"

INDEX=0
for IMG in "${IMAGE_FILES[@]}"; do
    DIMS="$(ffprobe -v error -select_streams v:0 -show_entries stream=width,height \
        -of csv=s=x:p=0 "$IMG")"
    WIDTH="${DIMS%x*}"
    HEIGHT="${DIMS#*x}"
    BASENAME="$(basename "$IMG")"

    echo "$INDEX,$BASENAME,$WIDTH,$HEIGHT" >> "$MANIFEST"
    printf "file '%s'\nduration %s\n" "$(realpath "$IMG")" "$(awk -v fr="$FRAMERATE" 'BEGIN{printf "%.6f", 1/fr}')" >> "$CONCAT_LIST"

    INDEX=$((INDEX + 1))
done
# ffmpeg concat demuxer butuh entry file terakhir diulang tanpa duration
printf "file '%s'\n" "$(realpath "${IMAGE_FILES[-1]}")" >> "$CONCAT_LIST"

echo "[INFO] Manifest ditulis: $MANIFEST ($INDEX gambar)"

# ==============================================================================
# 3. Encode ke video H.264 lossless (crf 0) - kompatibel dengan nvv4l2decoder
#    di Jetson dan tidak menambah artefak kompresi baru pada gambar.
# ==============================================================================
echo "[INFO] Meng-encode video lossless: $OUTPUT_VIDEO"
ffmpeg -y -f concat -safe 0 -i "$CONCAT_LIST" \
    -vsync vfr -c:v libx264 -preset veryslow -crf 0 -pix_fmt yuv420p \
    "$OUTPUT_VIDEO"

echo ""
echo "=== SELESAI ==="
echo "- Video   : $OUTPUT_VIDEO"
echo "- Manifest: $MANIFEST"
echo ""
echo "Jalankan pipeline evaluasi, contoh:"
echo "  ./build/app --config config/pgie_yolov8n_kitti.txt --input file \\"
echo "    --input-file $OUTPUT_VIDEO --output file --output-file /tmp/discard.mp4 \\"
echo "    --dump-detections data/eval/yolov8n_kitti_detections.jsonl"

#!/bin/bash
# ==============================================================================
# ADAS Perception - Model Benchmark Runner
# ------------------------------------------------------------------------------
# Menjalankan SATU model/config setiap kali dipanggil (sengaja tidak otomatis
# batch semua model), supaya Anda punya kendali penuh kapan/bagaimana setiap
# run dieksekusi. Untuk membandingkan N model, jalankan script ini N kali
# (interaktif pilih model berbeda, atau pakai --model tiap kali).
#
# Setiap run merekam:
#   1. FPS + latensi per-frame dari aplikasi (via --benchmark, ditulis app)
#   2. Hardware/power dari tegrastats (RAM, GPU%, CPU/core, power rails)
#   3. Metadata run (model, config, input/output, durasi, git commit, dst)
#
# Hasil disimpan terpisah per model & per timestamp, TIDAK PERNAH menimpa
# hasil run sebelumnya:
#   data/benchmark/<model>/<timestamp>/fps.csv
#   data/benchmark/<model>/<timestamp>/hardware_analysis.csv
#   data/benchmark/<model>/<timestamp>/run_info.txt
#   data/benchmark/<model>/<timestamp>/pipeline_output.mp4   (jika --output file)
#
# Contoh pemakaian:
#   ./scripts/run_benchmark.sh                          # menu pilih model interaktif
#   ./scripts/run_benchmark.sh --list                   # lihat model yang tersedia
#   ./scripts/run_benchmark.sh --model yolov8n_kitti
#   ./scripts/run_benchmark.sh --model yolov8n_kitti --duration 180
#   ./scripts/run_benchmark.sh --model yolov8n_kitti --input zed --camera-fps 30 --output monitor
# ==============================================================================

set -u

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

CONFIG_DIR="config"
BENCH_ROOT="data/benchmark"
TEGRA_INTERVAL_MS=1000

# ---- Default (mengikuti protokol benchmark: sumber input konsisten & terkontrol) ----
MODEL_NAME=""
CONFIG_FILE=""
TRACKER_CONFIG=""
INPUT_MODE="file"
INPUT_FILE="data/input/video_testing.mp4"
CAMERA_FPS="30"
OUTPUT_MODE="file"
DURATION=""
LIST_ONLY=0

# ==============================================================================
# HELPER
# ==============================================================================
find_executable() {
    local name="$1"
    if [ -x "./$name" ]; then
        echo "./$name"
    elif [ -x "./build/$name" ]; then
        echo "./build/$name"
    else
        echo ""
    fi
}

list_models() {
    for f in "$CONFIG_DIR"/pgie_*.txt; do
        [ -e "$f" ] || continue
        local base
        base="$(basename "$f")"
        base="${base#pgie_}"
        base="${base%.txt}"
        echo "$base"
    done
}

print_usage() {
    cat <<EOF
Penggunaan: $0 [opsi]

  --model <nama>                Nama model, lihat --list (contoh: yolov8n_kitti)
  --config <path>                Path config pgie kustom (override --model)
  --tracker-config <path>        Path config tracker kustom (default: profil NvDCF perf bawaan DeepStream)
  --input <zed|file>              Sumber input (default: $INPUT_MODE)
  --input-file <path>            File video jika --input file (default: $INPUT_FILE)
  --camera-fps <15|30|60|100|120> FPS kamera ZED jika --input zed (default: $CAMERA_FPS)
  --output <rtsp|monitor|file>   Mode output (default: $OUTPUT_MODE)
  --duration <detik>             Auto-stop setelah N detik (default: jalan sampai Ctrl+C)
  --list                         Tampilkan daftar model yang tersedia lalu keluar
  -h, --help                     Tampilkan bantuan ini

Setiap run tersimpan terpisah (tidak menimpa run lain) di:
  data/benchmark/<model>/<timestamp>/{fps.csv,hardware_analysis.csv,run_info.txt}
EOF
}

# ==============================================================================
# PARSE ARGUMEN
# ==============================================================================
while [ $# -gt 0 ]; do
    case "$1" in
        --model) MODEL_NAME="${2:-}"; shift 2 ;;
        --config) CONFIG_FILE="${2:-}"; shift 2 ;;
        --tracker-config) TRACKER_CONFIG="${2:-}"; shift 2 ;;
        --input) INPUT_MODE="${2:-}"; shift 2 ;;
        --input-file) INPUT_FILE="${2:-}"; shift 2 ;;
        --camera-fps) CAMERA_FPS="${2:-}"; shift 2 ;;
        --output) OUTPUT_MODE="${2:-}"; shift 2 ;;
        --duration) DURATION="${2:-}"; shift 2 ;;
        --list) LIST_ONLY=1; shift ;;
        -h|--help) print_usage; exit 0 ;;
        *) echo "[ERROR] Opsi tidak dikenal: $1"; print_usage; exit 1 ;;
    esac
done

if [ "$LIST_ONLY" -eq 1 ]; then
    echo "Model tersedia (dari $CONFIG_DIR/pgie_*.txt):"
    list_models | sed 's/^/  - /'
    exit 0
fi

if [[ "$INPUT_MODE" != "zed" && "$INPUT_MODE" != "file" ]]; then
    echo "[ERROR] --input harus 'zed' atau 'file'."
    exit 1
fi
if [[ "$OUTPUT_MODE" != "rtsp" && "$OUTPUT_MODE" != "monitor" && "$OUTPUT_MODE" != "file" ]]; then
    echo "[ERROR] --output harus 'rtsp', 'monitor', atau 'file'."
    exit 1
fi

# ==============================================================================
# PILIH MODEL (interaktif jika belum ditentukan lewat --model/--config)
# ==============================================================================
if [ -z "$CONFIG_FILE" ]; then
    if [ -z "$MODEL_NAME" ]; then
        mapfile -t AVAILABLE_MODELS < <(list_models)
        if [ ${#AVAILABLE_MODELS[@]} -eq 0 ]; then
            echo "[ERROR] Tidak ada file $CONFIG_DIR/pgie_*.txt ditemukan."
            exit 1
        fi
        echo "=== Pilih Model untuk Benchmark ==="
        for i in "${!AVAILABLE_MODELS[@]}"; do
            printf "  [%d] %s\n" "$((i + 1))" "${AVAILABLE_MODELS[$i]}"
        done
        read -rp "Masukkan nomor model: " CHOICE
        if ! [[ "$CHOICE" =~ ^[0-9]+$ ]] || [ "$CHOICE" -lt 1 ] || [ "$CHOICE" -gt "${#AVAILABLE_MODELS[@]}" ]; then
            echo "[ERROR] Pilihan tidak valid."
            exit 1
        fi
        MODEL_NAME="${AVAILABLE_MODELS[$((CHOICE - 1))]}"
    fi
    CONFIG_FILE="$CONFIG_DIR/pgie_${MODEL_NAME}.txt"
fi

if [ ! -f "$CONFIG_FILE" ]; then
    echo "[ERROR] Config tidak ditemukan: $CONFIG_FILE"
    echo "        Jalankan '$0 --list' untuk melihat model yang tersedia."
    exit 1
fi

if [ -z "$MODEL_NAME" ]; then
    MODEL_NAME="$(basename "$CONFIG_FILE" .txt)"
    MODEL_NAME="${MODEL_NAME#pgie_}"
fi

if [ "$INPUT_MODE" == "file" ] && [ ! -f "$INPUT_FILE" ]; then
    echo "[ERROR] Input file tidak ditemukan: $INPUT_FILE"
    exit 1
fi

# ==============================================================================
# SIAPKAN FOLDER HASIL PER-MODEL & PER-RUN (tidak pernah menimpa run lain)
# ==============================================================================
RUN_ID="$(date +%Y%m%d_%H%M%S)"
RUN_DIR="$BENCH_ROOT/$MODEL_NAME/$RUN_ID"
mkdir -p "$RUN_DIR"

FPS_CSV="$RUN_DIR/fps.csv"
RAW_LOG="$RUN_DIR/raw_hw.log"
HW_CSV="$RUN_DIR/hardware_analysis.csv"
RUN_INFO="$RUN_DIR/run_info.txt"
OUTPUT_VIDEO_FILE="$RUN_DIR/pipeline_output.mp4"

# ==============================================================================
# CARI EXECUTABLE
# ==============================================================================
EXEC_PATH="$(find_executable DeepStreamZedyoloRTSP)"
if [ -z "$EXEC_PATH" ]; then
    echo "[ERROR] Executable DeepStreamZedyoloRTSP tidak ditemukan di ./ atau ./build/."
    echo "        Jalankan './scripts/build.sh' terlebih dahulu."
    exit 1
fi

PARSER_EXEC="$(find_executable LogParser)"
if [ -z "$PARSER_EXEC" ]; then
    echo "[WARN] LogParser tidak ditemukan (./ atau ./build/). Log hardware mentah tidak akan"
    echo "       diproses otomatis jadi CSV, tapi benchmark tetap berjalan."
fi

# ==============================================================================
# SUSUN ARGUMEN APLIKASI
# ==============================================================================
APP_ARGS=(--config "$CONFIG_FILE" --benchmark "$FPS_CSV" --input "$INPUT_MODE" --output "$OUTPUT_MODE")

if [ -n "$TRACKER_CONFIG" ]; then
    APP_ARGS+=(--tracker-config "$TRACKER_CONFIG")
fi

if [ "$INPUT_MODE" == "file" ]; then
    APP_ARGS+=(--input-file "$INPUT_FILE")
else
    APP_ARGS+=(--camera-fps "$CAMERA_FPS")
fi

if [ "$OUTPUT_MODE" == "file" ]; then
    APP_ARGS+=(--output-file "$OUTPUT_VIDEO_FILE")
fi

# ==============================================================================
# METADATA RUN (penting untuk reproducibility di skripsi - lihat docs/04)
# ==============================================================================
{
    echo "model                  : $MODEL_NAME"
    echo "config_file            : $CONFIG_FILE"
    echo "tracker_config         : ${TRACKER_CONFIG:-<default DeepStream NvDCF perf profile>}"
    echo "input_mode             : $INPUT_MODE"
    [ "$INPUT_MODE" == "file" ] && echo "input_file             : $INPUT_FILE"
    [ "$INPUT_MODE" == "zed" ] && echo "camera_fps             : $CAMERA_FPS"
    echo "output_mode            : $OUTPUT_MODE"
    echo "duration_limit_s       : ${DURATION:-manual (Ctrl+C)}"
    echo "run_id                 : $RUN_ID"
    echo "started_at             : $(date '+%Y-%m-%d %H:%M:%S')"
    echo "git_commit             : $(git rev-parse --short HEAD 2>/dev/null || echo 'n/a')"
    echo "nvpmodel_mode          : $(nvpmodel -q 2>/dev/null | tr '\n' ' ' || echo 'n/a')"
    echo "jetson_clocks_status   : $(jetson_clocks --show 2>/dev/null | tr '\n' ' ' || echo 'n/a')"
    echo "tegrastats_interval_ms : $TEGRA_INTERVAL_MS"
    echo "latency_measurement    : NVDS_ENABLE_LATENCY_MEASUREMENT=1 (diaktifkan otomatis)"
} > "$RUN_INFO"

# ==============================================================================
# CLEANUP (dipanggil sekali saja, baik lewat Ctrl+C, timeout, maupun selesai normal)
# ==============================================================================
CLEANED_UP=0
cleanup() {
    if [ "$CLEANED_UP" -eq 1 ]; then
        return
    fi
    CLEANED_UP=1

    echo -e "\n[INFO] Menghentikan perekam hardware..."
    if [ -n "${TEGRA_PID:-}" ]; then
        kill "$TEGRA_PID" 2>/dev/null
        wait "$TEGRA_PID" 2>/dev/null
    fi

    parse_hardware_data

    echo "finished_at            : $(date '+%Y-%m-%d %H:%M:%S')" >> "$RUN_INFO"

    echo ""
    echo "=== BENCHMARK SELESAI: $MODEL_NAME ($RUN_ID) ==="
    echo "- Folder hasil     : $RUN_DIR"
    echo "- Log FPS+Latency  : $FPS_CSV"
    echo "- Log Hardware     : $HW_CSV"
    echo "- Info run         : $RUN_INFO"
}
trap cleanup SIGINT SIGTERM EXIT

parse_hardware_data() {
    if [ -z "$PARSER_EXEC" ] || [ ! -f "$RAW_LOG" ]; then
        return
    fi
    echo "[INFO] Memproses data hardware mentah menjadi CSV..."
    if "$PARSER_EXEC" "$RAW_LOG" "$HW_CSV"; then
        rm -f "$RAW_LOG"
    else
        echo "[ERROR] Gagal memproses data log. Log mentah tetap ada di: $RAW_LOG"
    fi
}

# ==============================================================================
# EKSEKUSI UTAMA
# ==============================================================================
echo "=== MEMULAI BENCHMARK: $MODEL_NAME ==="
echo "[INFO] Config  : $CONFIG_FILE"
echo "[INFO] Input   : $INPUT_MODE ${INPUT_FILE:+($INPUT_FILE)}"
echo "[INFO] Output  : $OUTPUT_MODE"
echo "[INFO] Hasil   : $RUN_DIR"

tegrastats --interval "$TEGRA_INTERVAL_MS" 2>/dev/null | \
    awk '{ print strftime("%Y-%m-%d %H:%M:%S"), $0; fflush(); }' > "$RAW_LOG" &
TEGRA_PID=$!
echo "[INFO] tegrastats berjalan di background (PID: $TEGRA_PID)"

export NVDS_ENABLE_LATENCY_MEASUREMENT=1

echo "[INFO] Menjalankan: $EXEC_PATH ${APP_ARGS[*]}"
if [ -n "$DURATION" ]; then
    echo "[INFO] Durasi dibatasi otomatis: ${DURATION}s (SIGINT dikirim ke aplikasi agar shutdown mulus)"
    timeout -s SIGINT "$DURATION" "$EXEC_PATH" "${APP_ARGS[@]}"
else
    echo "[INFO] Tekan Ctrl+C satu kali untuk menghentikan..."
    "$EXEC_PATH" "${APP_ARGS[@]}"
fi

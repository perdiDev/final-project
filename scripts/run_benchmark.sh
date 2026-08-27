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
# Real-time target: ≥30 FPS (ADAS safety-critical standard; ZED camera HD capability)
MODEL_NAME=""
CONFIG_FILE=""
TRACKER=""
TRACKER_PATH=""
INPUT_MODE="file"
INPUT_FILE="data/input/video-testing/realtime-1.mp4"
CAMERA_FPS="30"
OUTPUT_MODE="file"
DURATION=""
LIST_ONLY=0
TESTING_MODE=0

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

list_trackers() {
    for f in "$CONFIG_DIR"/tracker_*.yml "$CONFIG_DIR"/tracker_*.yaml; do
        [ -f "$f" ] || continue
        local base
        base="$(basename "$f")"
        base="${base#tracker_}"
        base="${base%.yml}"
        base="${base%.yaml}"
        echo "$base"
    done | sort -u
}

tracker_config_for() {
    local tracker_name="$1"
    local candidate
    for candidate in "$CONFIG_DIR/tracker_${tracker_name}.yml" "$CONFIG_DIR/tracker_${tracker_name}.yaml"; do
        if [ -f "$candidate" ]; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

default_tracker() {
    if [ -f "$CONFIG_DIR/tracker_nvdcf.yml" ] || [ -f "$CONFIG_DIR/tracker_nvdcf.yaml" ]; then
        echo "nvdcf"
    else
        list_trackers | sed -n '1p'
    fi
}

print_usage() {
    cat <<EOF
Penggunaan: $0 [opsi]

  --model <nama>                Nama model, lihat --list (contoh: yolov8n_kitti)
  --config <path>                Path config pgie kustom (override --model)
  --tracker <path>              Path file YAML tracker (default dipilih otomatis dari config/tracker_*.yml)
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
        --tracker) TRACKER="${2:-}"; shift 2 ;;
        --input) INPUT_MODE="${2:-}"; shift 2 ;;
        --input-file) INPUT_FILE="${2:-}"; shift 2 ;;
        --camera-fps) CAMERA_FPS="${2:-}"; shift 2 ;;
        --output) OUTPUT_MODE="${2:-}"; shift 2 ;;
        --duration) DURATION="${2:-}"; shift 2 ;;
        --testing) TESTING_MODE=1; shift ;;
        --list) LIST_ONLY=1; shift ;;
        -h|--help) print_usage; exit 0 ;;
        *) echo "[ERROR] Opsi tidak dikenal: $1"; print_usage; exit 1 ;;
    esac
done

if [ "$LIST_ONLY" -eq 1 ]; then
    echo "Model tersedia (dari $CONFIG_DIR/pgie_*.txt):"
    list_models | sed 's/^/  - /'
    echo "Tracker tersedia (dari $CONFIG_DIR/tracker_*.yml atau *.yaml):"
    list_trackers | sed 's/^/  - /'
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
# PILIH TRACKER (interaktif jika belum ditentukan lewat --tracker)
# ==============================================================================
if [ -z "$TRACKER" ]; then
    mapfile -t AVAILABLE_TRACKERS < <(list_trackers)
    if [ ${#AVAILABLE_TRACKERS[@]} -eq 0 ]; then
        echo "[ERROR] Tidak ada file $CONFIG_DIR/tracker_*.yml atau *.yaml ditemukan."
        exit 1
    fi
    echo ""
    echo "=== Pilih Tracker untuk Benchmark ==="
    for i in "${!AVAILABLE_TRACKERS[@]}"; do
        printf "  [%d] %s\n" "$((i + 1))" "${AVAILABLE_TRACKERS[$i]}"
    done
    read -rp "Masukkan nomor tracker: " T_CHOICE
    if ! [[ "$T_CHOICE" =~ ^[0-9]+$ ]] || [ "$T_CHOICE" -lt 1 ] || [ "$T_CHOICE" -gt "${#AVAILABLE_TRACKERS[@]}" ]; then
        echo "[ERROR] Pilihan tracker tidak valid."
        exit 1
    fi
    TRACKER="${AVAILABLE_TRACKERS[$((T_CHOICE - 1))]}"
    TRACKER_PATH="$(tracker_config_for "$TRACKER")"
else
    # Jika --tracker diberikan via argumen terminal, cek apakah itu path langsung atau nama
    if [ -f "$TRACKER" ]; then
        TRACKER_PATH="$TRACKER"
    else
        TRACKER_PATH="$(tracker_config_for "$TRACKER")"
    fi
fi

if [ -z "$TRACKER_PATH" ] || [ ! -f "$TRACKER_PATH" ]; then
    echo "[ERROR] Config tracker tidak ditemukan: ${TRACKER_PATH:-$TRACKER}"
    echo "        Pilihan file yang tersedia:"
    list_trackers | sed 's/^/  - /'
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
HW_ERROR_LOG="$RUN_DIR/hardware_recorder_error.log"
HW_FIFO="$RUN_DIR/.tegrastats.fifo"
RUN_INFO="$RUN_DIR/run_info.txt"
OUTPUT_VIDEO_FILE="$RUN_DIR/pipeline_output.mp4"

# ==============================================================================
# CARI EXECUTABLE
# ==============================================================================
EXEC_PATH="$(find_executable app)"
if [ -z "$EXEC_PATH" ]; then
    echo "[ERROR] Executable app tidak ditemukan di ./ atau ./build/."
    echo "        Jalankan './scripts/build.sh' terlebih dahulu."
    exit 1
fi

PARSER_EXEC="$(find_executable parser)"
if [ -z "$PARSER_EXEC" ]; then
    echo "[WARN] parser tidak ditemukan (./ atau ./build/). Log hardware mentah tidak akan"
    echo "       diproses otomatis jadi CSV, tapi benchmark tetap berjalan."
fi

TEGRASTATS_EXEC="$(command -v tegrastats 2>/dev/null || true)"
if [ -z "$TEGRASTATS_EXEC" ]; then
    echo "[WARN] tegrastats tidak ditemukan di PATH. Hardware benchmark tidak dapat direkam."
    echo "       Jalankan benchmark pada Jetson dan pastikan tegrastats dapat dipanggil."
fi

# ==============================================================================
# SUSUN ARGUMEN APLIKASI
# ==============================================================================
APP_ARGS=(--config "$CONFIG_FILE" --benchmark "$FPS_CSV" --input "$INPUT_MODE" --output "$OUTPUT_MODE")

APP_ARGS+=(--tracker "$TRACKER_PATH")
TRACKER_LABEL="$TRACKER_PATH"

if [ "$INPUT_MODE" == "file" ]; then
    APP_ARGS+=(--input-file "$INPUT_FILE")
else
    APP_ARGS+=(--camera-fps "$CAMERA_FPS")
fi

if [ "$OUTPUT_MODE" == "file" ]; then
    APP_ARGS+=(--output-file "$OUTPUT_VIDEO_FILE")
fi

if [ "${TESTING_MODE:-0}" -eq 1 ]; then
    APP_ARGS+=(--quiet)
fi

# ==============================================================================
# METADATA RUN (penting untuk reproducibility di skripsi - lihat docs/04)
# ==============================================================================
{
    echo "model                  : $MODEL_NAME"
    echo "config_file            : $CONFIG_FILE"
    echo "tracker                : $TRACKER_LABEL"
    echo "tracker_path           : $TRACKER_PATH"
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
    echo "tegrastats_path        : ${TEGRASTATS_EXEC:-not found}"
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

	if [ "${TESTING_MODE:-0}" -eq 0 ]; then
    	echo -e "\n[INFO] Menghentikan perekam hardware..."
	fi

    if [ -n "${TEGRA_PID:-}" ]; then
        kill "$TEGRA_PID" 2>/dev/null
        wait "$TEGRA_PID" 2>/dev/null
    fi
    if [ -n "${HW_FORMATTER_PID:-}" ]; then
        wait "$HW_FORMATTER_PID" 2>/dev/null
    fi
    rm -f "$HW_FIFO"

    if parse_hardware_data; then
        echo "hardware_log_status     : success" >> "$RUN_INFO"
    else
        echo "hardware_log_status     : failed (lihat output terminal/hardware_recorder_error.log)" >> "$RUN_INFO"
    fi

    echo "finished_at            : $(date '+%Y-%m-%d %H:%M:%S')" >> "$RUN_INFO"

	if [ "${TESTING_MODE:-0}" -eq 1 ]; then
        echo "[OK] Run Selesai. Hasil tersimpan di: $RUN_DIR"
    else
	    echo ""
	    echo "=== BENCHMARK SELESAI: $MODEL_NAME ($RUN_ID) ==="
	    echo "- Folder hasil     : $RUN_DIR"
	    echo "- Log FPS+Latency  : $FPS_CSV"
	    if [ -s "$HW_CSV" ]; then
		echo "- Log Hardware     : $HW_CSV"
	    else
		echo "- Log Hardware     : TIDAK TERSEDIA (lihat $RUN_INFO)"
	    fi
	    echo "- Info run         : $RUN_INFO"
	fi
}
trap cleanup SIGINT SIGTERM EXIT

parse_hardware_data() {
    if [ -z "$TEGRASTATS_EXEC" ]; then
        echo "[ERROR] Hardware CSV tidak dibuat karena tegrastats tidak tersedia."
        return 1
    fi
    if [ ! -s "$RAW_LOG" ]; then
        echo "[ERROR] tegrastats tidak menghasilkan data. Log mentah dipertahankan: $RAW_LOG"
        if [ -s "$HW_ERROR_LOG" ]; then
            echo "[ERROR] Detail perekam hardware:"
            cat "$HW_ERROR_LOG"
        else
            echo "[ERROR] Coba jalankan 'tegrastats --interval $TEGRA_INTERVAL_MS' secara manual."
        fi
        return 1
    fi
    if [ -z "$PARSER_EXEC" ]; then
        echo "[ERROR] parser tidak tersedia. Log mentah dipertahankan: $RAW_LOG"
        return 1
    fi

	if [ "${TESTING_MODE:-0}" -eq 0 ]; then
    	echo "[INFO] Memproses data hardware mentah menjadi CSV..."
	fi

    if "$PARSER_EXEC" "$RAW_LOG" "$HW_CSV" && [ -s "$HW_CSV" ]; then
        rm -f "$RAW_LOG"
        if [ ! -s "$HW_ERROR_LOG" ]; then
            rm -f "$HW_ERROR_LOG"
        fi
        return 0
    fi

    echo "[ERROR] Gagal memproses data log. Log mentah tetap ada di: $RAW_LOG"
    return 1
}

# ==============================================================================
# EKSEKUSI UTAMA
# ==============================================================================
if [ "${TESTING_MODE:-0}" -eq 0 ]; then
    echo "=== MEMULAI BENCHMARK: $MODEL_NAME ==="
    echo "[INFO] Hasil akan disimpan di folder: $RUN_DIR"
fi

TEGRA_PID=""
HW_FORMATTER_PID=""
if [ -n "$TEGRASTATS_EXEC" ]; then
    rm -f "$HW_FIFO"
    if mkfifo "$HW_FIFO"; then
        HW_START_EPOCH_MS="$(date +%s%3N)"
        echo "tegrastats_started_ms  : $HW_START_EPOCH_MS" >> "$RUN_INFO"
        awk -v start_ms="$HW_START_EPOCH_MS" -v interval_ms="$TEGRA_INTERVAL_MS" '
            {
                sample_index = NR - 1
                sample_ms = start_ms + (sample_index * interval_ms)
                sample_seconds = int(sample_ms / 1000)
                milliseconds = sample_ms - (sample_seconds * 1000)
                printf "%s.%03d Sample=%d Hardware_Elapsed_ms=%d %s\n",
                       strftime("%Y-%m-%d %H:%M:%S", sample_seconds),
                       milliseconds, sample_index, sample_index * interval_ms, $0
                fflush()
            }
        ' < "$HW_FIFO" > "$RAW_LOG" &
        HW_FORMATTER_PID=$!
        "$TEGRASTATS_EXEC" --interval "$TEGRA_INTERVAL_MS" \
            > "$HW_FIFO" 2> "$HW_ERROR_LOG" &
        TEGRA_PID=$!
		if [ "${TESTING_MODE:-0}" -eq 0 ]; then
        	echo "[INFO] tegrastats berjalan di background (PID: $TEGRA_PID)"
		fi
    else
        echo "[ERROR] Gagal membuat FIFO untuk perekam hardware: $HW_FIFO"
    fi
else
    echo "[WARN] Benchmark dilanjutkan tanpa data hardware."
fi

export NVDS_ENABLE_LATENCY_MEASUREMENT=1

if [ "${TESTING_MODE:-0}" -eq 1 ]; then
    # --- MODE TESTING (HENING & BERSIH) ---
    export GST_DEBUG=1
    export NVDSINFER_LOG_LEVEL=1
    APP_STDOUT="$RUN_DIR/app_stdout.log"

    if [ -n "$DURATION" ]; then
        timeout -s SIGINT "$DURATION" "$EXEC_PATH" "${APP_ARGS[@]}" > "$APP_STDOUT" 2>&1
    else
        "$EXEC_PATH" "${APP_ARGS[@]}" > "$APP_STDOUT" 2>&1
    fi
else
    # --- MODE DEBUG (DEFAULT, MUNCULKAN SEMUA INFO) ---
    APP_STDERR="/dev/stderr"

    echo "[INFO] Menjalankan: $EXEC_PATH ${APP_ARGS[*]}"
    if [ -n "$DURATION" ]; then
        echo "[INFO] Durasi dibatasi otomatis: ${DURATION}s"
        timeout -s SIGINT "$DURATION" "$EXEC_PATH" "${APP_ARGS[@]}" 2> "$APP_STDERR"
    else
        echo "[INFO] Tekan Ctrl+C satu kali untuk menghentikan..."
        "$EXEC_PATH" "${APP_ARGS[@]}" 2> "$APP_STDERR"
    fi
fi

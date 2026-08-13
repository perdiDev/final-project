#!/bin/bash
# ==============================================================================
# Automasi 18 Skenario Benchmark (6 Model x 3 Tracker)
# ==============================================================================
set -e

# Waktu jeda (cooldown) antar skenario (dalam detik)
COOLDOWN_TIME=60

# Path video yang akan diuji (ubah sesuai kebutuhan)
VIDEO_INPUT="data/input/video_testing.mp4"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

# ==============================================================================
# INISIALISASI HARDWARE JETSON
# ==============================================================================
echo "[INFO] Mengatur Jetson Orin Nano ke mode performa maksimal (MAXN)..."
# Memastikan mode MAXN (mode 0) aktif
sudo nvpmodel -m 0
# Memaksa kipas dan clock CPU/GPU berjalan di frekuensi maksimal
sudo jetson_clocks
echo "[INFO] Status jetson_clocks:"
sudo jetson_clocks --show
echo ""

# 1. Definisikan 6 Config Model
MODELS=(
    "yolov8n_kitti"
    "yolov9t_kitti"
    "yolov10n_kitti"
    "yolov26n_kitti"
    "yolov8n_kitti_efficientnms"
    "yolov9t_kitti_efficientnms"
)

# 2. Definisikan 3 Tracker
TRACKERS=(
    "nvdcf"
    "nvsort"
    "nvdcf_perf"
)

echo "======================================================================"
echo "MEMULAI BATCH BENCHMARK TUGAS AKHIR"
echo "Total Skenario: $((${#MODELS[@]} * ${#TRACKERS[@]})) (6 Model x 3 Tracker)"
echo "Input Video: $VIDEO_INPUT"
echo "======================================================================"

for MODEL in "${MODELS[@]}"; do
    for TRACKER in "${TRACKERS[@]}"; do

        COMBINED_NAME="${MODEL}_${TRACKER}"
        CONFIG_FILE="config/pgie_${MODEL}.txt"

        echo "----------------------------------------------------------------------"
        echo "[>>>] MENJALANKAN SKENARIO: $COMBINED_NAME"
        echo "----------------------------------------------------------------------"

        if [ ! -f "$CONFIG_FILE" ]; then
            echo "[ERROR] File $CONFIG_FILE tidak ditemukan! Melewati skenario ini."
            continue
        fi

        # Eksekusi run_benchmark.sh tanpa --duration dan dengan --input-file eksplisit
        ./scripts/run_benchmark.sh \
            --config "$CONFIG_FILE" \
            --model "$COMBINED_NAME" \
            --tracker "$TRACKER" \
            --input file \
            --input-file "$VIDEO_INPUT"

        echo "[INFO] Skenario $COMBINED_NAME selesai."
        echo "[INFO] Memasuki fase cooldown $COOLDOWN_TIME detik..."

        # Membersihkan memori agar run selanjutnya bersih
        sudo sync && sudo sysctl -q -w vm.drop_caches=3
        sleep "$COOLDOWN_TIME"

    done
done

echo "======================================================================"
echo "SEMUA SKENARIO BENCHMARK TELAH SELESAI!"
echo "======================================================================"

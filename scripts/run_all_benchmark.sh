#!/bin/bash
# ==============================================================================
# Automasi 60 Run Benchmark (6 Model x 2 Tracker x 5 Repetisi)
# ==============================================================================
set -e

# Waktu jeda (cooldown) antar skenario (dalam detik)
COOLDOWN_TIME=60

# Jumlah pengulangan setiap skenario
REPEAT_COUNT=5

# Path video yang akan diuji
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

# 2. Definisikan 2 Tracker
TRACKERS=(
    "nvdcf"
    "nvsort"
)

TOTAL_SCENARIOS=$((${#MODELS[@]} * ${#TRACKERS[@]}))
TOTAL_RUNS=$((TOTAL_SCENARIOS * REPEAT_COUNT))

echo "======================================================================"
echo "MEMULAI BATCH BENCHMARK TUGAS AKHIR"
echo "Model            : ${#MODELS[@]}"
echo "Tracker           : ${#TRACKERS[@]}"
echo "Pengulangan       : $REPEAT_COUNT kali"
echo "Total Skenario    : $TOTAL_SCENARIOS"
echo "Total Run         : $TOTAL_RUNS"
echo "Input Video       : $VIDEO_INPUT"
echo "======================================================================"

# ==============================================================================
# LOOP BENCHMARK
# ==============================================================================
SCENARIO_NO=0
RUN_NO=0

for MODEL in "${MODELS[@]}"; do
    for TRACKER in "${TRACKERS[@]}"; do

        SCENARIO_NO=$((SCENARIO_NO + 1))

        COMBINED_NAME="${MODEL}_${TRACKER}"
        CONFIG_FILE="config/pgie_${MODEL}.txt"

        echo ""
        echo "======================================================================"
        echo "[SCENARIO $SCENARIO_NO/$TOTAL_SCENARIOS] $COMBINED_NAME"
        echo "======================================================================"

        if [ ! -f "$CONFIG_FILE" ]; then
            echo "[ERROR] File $CONFIG_FILE tidak ditemukan!"
            echo "[WARN] Melewati seluruh $REPEAT_COUNT pengulangan untuk $COMBINED_NAME."
            continue
        fi

        # ----------------------------------------------------------------------
        # ULANGI SETIAP SKENARIO 5 KALI
        # ----------------------------------------------------------------------
        for ((REPEAT=1; REPEAT<=REPEAT_COUNT; REPEAT++)); do

            RUN_NO=$((RUN_NO + 1))

            echo ""
            echo "----------------------------------------------------------------------"
            echo "[RUN $RUN_NO/$TOTAL_RUNS]"
            echo "[SCENARIO] $COMBINED_NAME"
            echo "[REPETISI] $REPEAT/$REPEAT_COUNT"
            echo "----------------------------------------------------------------------"

            ./scripts/run_benchmark.sh \
                --config "$CONFIG_FILE" \
                --model "$COMBINED_NAME" \
                --tracker "$TRACKER" \
                --input file \
                --input-file "$VIDEO_INPUT"

            echo "[INFO] Run $REPEAT/$REPEAT_COUNT untuk $COMBINED_NAME selesai."
            echo "[INFO] Membersihkan cache..."

            sudo sync
            sudo sysctl -q -w vm.drop_caches=3

            # Tidak perlu cooldown setelah run terakhir suatu skenario
            if [ "$REPEAT" -lt "$REPEAT_COUNT" ] || [ "$COMBINED_NAME" != "${MODELS[-1]}_${TRACKERS[-1]}" ]; then
                echo "[INFO] Cooldown $COOLDOWN_TIME detik..."
                sleep "$COOLDOWN_TIME"
            fi

        done

        echo "[INFO] Semua $REPEAT_COUNT repetisi untuk $COMBINED_NAME selesai."

    done
done

echo ""
echo "======================================================================"
echo "SEMUA BENCHMARK TELAH SELESAI!"
echo "Total Skenario : $TOTAL_SCENARIOS"
echo "Total Run      : $TOTAL_RUNS"
echo "======================================================================"

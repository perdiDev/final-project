#!/bin/bash
# ==============================================================================
# Automasi 60 Run Benchmark (6 Model x 2 Tracker x 5 Repetisi)
# ==============================================================================
set -e

# --- DEKLARASI WARNA ---
CYAN='\033[1;36m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
PURPLE='\033[1;35m'
RED='\033[1;31m'
NC='\033[0m' # No Color

# --- LOGIKA FLAG DEBUG ---
DEBUG_MODE=0 # Default: Bersih (Testing Mode dinyalakan)
while [ $# -gt 0 ]; do
    case "$1" in
        --debug) DEBUG_MODE=1; shift ;;
        *) echo "Opsi tidak dikenal: $1"; exit 1 ;;
    esac
done

COOLDOWN_TIME=60
REPEAT_COUNT=5
VIDEO_INPUT="data/input/video_testing.mp4"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

# ==============================================================================
# INISIALISASI HARDWARE JETSON
# ==============================================================================
echo -e "${YELLOW}[INFO] Mengatur Jetson Orin Nano ke mode performa maksimal (MAXN)...${NC}"
sudo nvpmodel -m 0
sudo jetson_clocks

echo -e "${YELLOW}[INFO] Status jetson_clocks aktif. (Detail disembunyikan untuk kerapihan)${NC}\n"

MODELS=("yolov8n_kitti" "yolov9t_kitti" "yolov10n_kitti" "yolov26n_kitti" "yolov8n_kitti_efficientnms" "yolov9t_kitti_efficientnms")
TRACKERS=("nvdcf" "nvsort")

TOTAL_SCENARIOS=$((${#MODELS[@]} * ${#TRACKERS[@]}))
TOTAL_RUNS=$((TOTAL_SCENARIOS * REPEAT_COUNT))

echo -e "${CYAN}======================================================================${NC}"
echo -e "${CYAN}MEMULAI BATCH BENCHMARK TUGAS AKHIR${NC}"
echo -e "Model             : ${#MODELS[@]}"
echo -e "Tracker           : ${#TRACKERS[@]}"
echo -e "Pengulangan       : $REPEAT_COUNT kali"
echo -e "Total Run         : $TOTAL_RUNS"
echo -e "Mode Debug (Log)  : $(if [ "$DEBUG_MODE" -eq 1 ]; then echo -e "${RED}AKTIF${NC}"; else echo -e "${GREEN}NON-AKTIF (BERSIH)${NC}"; fi)"
echo -e "${CYAN}======================================================================${NC}\n"

SCENARIO_NO=0
RUN_NO=0

for MODEL in "${MODELS[@]}"; do
    for TRACKER in "${TRACKERS[@]}"; do
        SCENARIO_NO=$((SCENARIO_NO + 1))
        COMBINED_NAME="${MODEL}_${TRACKER}"
        CONFIG_FILE="config/pgie_${MODEL}.txt"

        echo -e "${PURPLE}======================================================================${NC}"
        echo -e "${PURPLE}[SCENARIO $SCENARIO_NO/$TOTAL_SCENARIOS] $COMBINED_NAME${NC}"
        echo -e "${PURPLE}======================================================================${NC}"

        if [ ! -f "$CONFIG_FILE" ]; then
            echo -e "${RED}[ERROR] File $CONFIG_FILE tidak ditemukan!${NC}"
            continue
        fi

        for ((REPEAT=1; REPEAT<=REPEAT_COUNT; REPEAT++)); do
            RUN_NO=$((RUN_NO + 1))
            
            echo ""
            echo -e "${CYAN}----------------------------------------------------------------------${NC}"
            echo -e "${CYAN}[RUN $RUN_NO/$TOTAL_RUNS]${NC} | ${GREEN}SKENARIO: $COMBINED_NAME${NC} | ${YELLOW}REPETISI: $REPEAT/$REPEAT_COUNT${NC}"
            echo -e "${CYAN}----------------------------------------------------------------------${NC}"

            # Menyusun argumen untuk run_benchmark.sh
            BENCHMARK_ARGS=(
                --config "$CONFIG_FILE"
                --model "$COMBINED_NAME"
                --tracker "$TRACKER"
                --input file
                --input-file "$VIDEO_INPUT"
            )
            
            # Jika run_all dijalankan TANPA --debug, kita set run_benchmark menjadi mode --testing
            if [ "$DEBUG_MODE" -eq 0 ]; then
                BENCHMARK_ARGS+=(--testing)
            fi

            # Eksekusi
            ./scripts/run_benchmark.sh "${BENCHMARK_ARGS[@]}"

            echo -e "${GREEN}[INFO] Run $REPEAT/$REPEAT_COUNT selesai.${NC}"
            
            sudo sync
            sudo sysctl -q -w vm.drop_caches=3

            if [ "$REPEAT" -lt "$REPEAT_COUNT" ] || [ "$COMBINED_NAME" != "${MODELS[-1]}_${TRACKERS[-1]}" ]; then
                echo -e "${YELLOW}[INFO] Cooldown $COOLDOWN_TIME detik...${NC}"
                sleep "$COOLDOWN_TIME"
            fi
        done
        echo -e "${GREEN}[INFO] Skenario $COMBINED_NAME (5 Repetisi) selesai.${NC}\n"
    done
done

echo -e "${GREEN}======================================================================${NC}"
echo -e "${GREEN}SEMUA BENCHMARK TELAH SELESAI!${NC}"
echo -e "${GREEN}======================================================================${NC}"

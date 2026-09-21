#!/bin/bash
# ==============================================================================
# Automasi Run Benchmark (Video x Model x Precision x Tracker x Repetisi)
# ------------------------------------------------------------------------------
# Precision FP32 bersifat OPT-IN lewat --precision, supaya perilaku default
# (tanpa flag apa pun) tetap sama seperti sebelumnya: hanya FP16.
#
# Hasil FP16 dan FP32 disimpan di subtree data/benchmark/ yang TERPISAH, supaya
# keduanya tidak pernah tertukar/tercampur:
#   data/benchmark/fp16/<model>_<tracker>_<video>/<run_id>/
#   data/benchmark/fp32/<model>_<tracker>_<video>/<run_id>/
#
# Contoh pemakaian:
#   ./scripts/run_all_benchmark.sh                     # default: precision fp16 saja
#   ./scripts/run_all_benchmark.sh --precision fp32    # hanya precision fp32
#   ./scripts/run_all_benchmark.sh --precision all      # fp16 DAN fp32 (2x jumlah run)
#   ./scripts/run_all_benchmark.sh --precision all --debug
# ==============================================================================
set -e

# --- DEKLARASI WARNA ---
CYAN='\033[1;36m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
PURPLE='\033[1;35m'
RED='\033[1;31m'
NC='\033[0m' # No Color

# --- LOGIKA FLAG ---
DEBUG_MODE=0 # Default: Bersih (Testing Mode dinyalakan)
PRECISION_MODE="fp16" # Default: hanya FP16 (baseline) -> perilaku sama seperti sebelumnya
while [ $# -gt 0 ]; do
    case "$1" in
        --debug) DEBUG_MODE=1; shift ;;
        --precision) PRECISION_MODE="${2:-}"; shift 2 ;;
        -h|--help)
            echo "Penggunaan: $0 [--debug] [--precision fp16|fp32|all]"
            echo ""
            echo "  --debug              Tampilkan seluruh log aplikasi (default: mode bersih/testing)"
            echo "  --precision <mode>   fp16 (default, baseline) | fp32 (pembanding) | all (keduanya)"
            exit 0
            ;;
        *) echo "Opsi tidak dikenal: $1"; exit 1 ;;
    esac
done

case "$PRECISION_MODE" in
    fp16) PRECISIONS=("fp16") ;;
    fp32) PRECISIONS=("fp32") ;;
    all)  PRECISIONS=("fp16" "fp32") ;;
    *)
        echo "[ERROR] --precision harus 'fp16', 'fp32', atau 'all' (dapat: '$PRECISION_MODE')"
        exit 1
        ;;
esac

COOLDOWN_TIME=60
REPEAT_COUNT=5

# Previous Skenario
# VIDEO_INPUTS=(
#     "data/input/video_testing.mp4"
#     "data/input/video_testing-2.mp4"
#     "data/input/video_testing-3.mp4"
#     "data/input/video_testing-4.mp4"
#     "data/input/video_testing-5.mp4"
# )
#

VIDEO_INPUTS=(
    "data/input/video-testing/realtime-1.mp4"
)

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

# --- HITUNG ULANG TOTAL SKENARIO ---
TOTAL_SCENARIOS=$((${#VIDEO_INPUTS[@]} * ${#MODELS[@]} * ${#PRECISIONS[@]} * ${#TRACKERS[@]}))
TOTAL_RUNS=$((TOTAL_SCENARIOS * REPEAT_COUNT))

echo -e "${CYAN}======================================================================${NC}"
echo -e "${CYAN}MEMULAI BATCH BENCHMARK TUGAS AKHIR${NC}"
echo -e "Video Input       : ${#VIDEO_INPUTS[@]} file"
echo -e "Model             : ${#MODELS[@]}"
echo -e "Precision         : ${PRECISIONS[*]}"
echo -e "Tracker           : ${#TRACKERS[@]}"
echo -e "Pengulangan       : $REPEAT_COUNT kali"
echo -e "Total Skenario    : $TOTAL_SCENARIOS"
echo -e "Total Run         : $TOTAL_RUNS"
echo -e "Mode Debug (Log)  : $(if [ "$DEBUG_MODE" -eq 1 ]; then echo -e "${RED}AKTIF${NC}"; else echo -e "${GREEN}NON-AKTIF (BERSIH)${NC}"; fi)"
echo -e "${CYAN}======================================================================${NC}\n"

SCENARIO_NO=0
RUN_NO=0

# === TAMBAHKAN LOOP UNTUK VIDEO ===
for VIDEO in "${VIDEO_INPUTS[@]}"; do
    VIDEO_NAME=$(basename "$VIDEO")
    VIDEO_BASE="${VIDEO_NAME%.*}" # Hapus ekstensi .mp4 untuk penamaan folder

    echo -e "${YELLOW}>>> MEMULAI PENGUJIAN UNTUK VIDEO: $VIDEO_NAME <<<${NC}\n"

    for MODEL in "${MODELS[@]}"; do
        for PRECISION in "${PRECISIONS[@]}"; do
            # fp16 = config baseline (tanpa suffix), fp32 = config pembanding "_fp32"
            # (lihat config/pgie_<model>_fp32.txt, dibuat oleh docs/03_deployment_pipeline.md §3.2)
            if [ "$PRECISION" == "fp32" ]; then
                MODEL_VARIANT="${MODEL}_fp32"
            else
                MODEL_VARIANT="$MODEL"
            fi
            CONFIG_FILE="config/pgie_${MODEL_VARIANT}.txt"

            for TRACKER in "${TRACKERS[@]}"; do
                SCENARIO_NO=$((SCENARIO_NO + 1))

                # Tambahkan nama video ke kombinasi agar folder hasilnya terpisah
                COMBINED_NAME="${MODEL}_${TRACKER}_${VIDEO_BASE}"
                # Precision jadi LEVEL FOLDER TERPISAH (bukan ditempel ke nama), supaya hasil
                # FP16 dan FP32 benar-benar berada di subtree data/benchmark/ yang berbeda:
                #   data/benchmark/fp16/<model>_<tracker>_<video>/<run_id>/
                #   data/benchmark/fp32/<model>_<tracker>_<video>/<run_id>/
                # run_benchmark.sh memakai nilai --model apa adanya sebagai fragmen path
                # (RUN_DIR="$BENCH_ROOT/$MODEL_NAME/$RUN_ID", dibuat lewat mkdir -p), jadi
                # menyisipkan "/" di sini sudah cukup tanpa perlu mengubah script tersebut.
                MODEL_LABEL="${PRECISION}/${COMBINED_NAME}"

                echo -e "${PURPLE}======================================================================${NC}"
                echo -e "${PURPLE}[SCENARIO $SCENARIO_NO/$TOTAL_SCENARIOS] $COMBINED_NAME (precision=$PRECISION)${NC}"
                echo -e "${PURPLE}======================================================================${NC}"

                if [ ! -f "$CONFIG_FILE" ]; then
                    echo -e "${RED}[ERROR] File $CONFIG_FILE tidak ditemukan!${NC}"
                    continue
                fi

                for ((REPEAT=1; REPEAT<=REPEAT_COUNT; REPEAT++)); do
                    RUN_NO=$((RUN_NO + 1))

                    echo ""
                    echo -e "${CYAN}----------------------------------------------------------------------${NC}"
                    echo -e "${CYAN}[RUN $RUN_NO/$TOTAL_RUNS]${NC} | ${GREEN}SKENARIO: $COMBINED_NAME${NC} | ${YELLOW}PRECISION: $PRECISION${NC} | ${YELLOW}REPETISI: $REPEAT/$REPEAT_COUNT${NC}"
                    echo -e "${CYAN}----------------------------------------------------------------------${NC}"

                    # Menyusun argumen untuk run_benchmark.sh
                    BENCHMARK_ARGS=(
                        --config "$CONFIG_FILE"
                        --model "$MODEL_LABEL"
                        --tracker "$TRACKER"
                        --input file
                        --input-file "$VIDEO"
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

                    # Logika cooldown disederhanakan: selama bukan run paling terakhir, jalankan cooldown
                    if [ "$RUN_NO" -lt "$TOTAL_RUNS" ]; then
                        for (( i=0; i<=COOLDOWN_TIME; i++ )); do
                            # Gunakan -ne agar echo tidak membuat baris baru, \r kembali ke awal, \033[K hapus sisa teks
                            echo -ne "\r${YELLOW}[INFO] Cooldown $COOLDOWN_TIME detik [$i/$COOLDOWN_TIME]${NC}\033[K"

                            # Jangan sleep di iterasi terakhir agar langsung lanjut
                            if [ "$i" -lt "$COOLDOWN_TIME" ]; then
                                sleep 1
                            fi
                        done
                        echo "" # Pindah ke baris baru setelah loop cooldown selesai
                    fi
                done
                echo -e "${GREEN}[INFO] Skenario $COMBINED_NAME (precision=$PRECISION, $REPEAT_COUNT Repetisi) selesai.${NC}\n"
            done
        done
    done
done

echo -e "${GREEN}======================================================================${NC}"
echo -e "${GREEN}SEMUA $TOTAL_RUNS BENCHMARK TELAH SELESAI!${NC}"
echo -e "${GREEN}======================================================================${NC}"

#!/bin/bash

# ==============================================================================
# KONFIGURASI FILE
# ==============================================================================
RAW_LOG="raw_hw.log"
HW_CSV="hardware_log.csv"
FPS_CSV="fps_log.csv"
CONFIG_FILE="config/pgie_coco.txt"

# ==============================================================================
# FUNGSI CLEANUP (Penanganan Ctrl+C)
# ==============================================================================
# Fungsi ini memastikan tegrastats otomatis mati jika Anda menekan Ctrl+C
cleanup() {
    echo -e "\n[INFO] Menghentikan perekam hardware..."
    if [ -n "$TEGRA_PID" ]; then
        kill $TEGRA_PID 2>/dev/null
    fi
    parse_hardware_data
    exit 0
}
trap cleanup SIGINT SIGTERM

# ==============================================================================
# FUNGSI PARSING DATA (Dijalankan SETELAH benchmark selesai)
# ==============================================================================
parse_hardware_data() {
    echo "[INFO] Memproses data hardware mentah menjadi CSV menggunakan C++ Parser..."

    # Cek lokasi aplikasi LogParser
    if [ -x "../LogParser" ]; then
        PARSER_EXEC="../LogParser"
    elif [ -x "./build/LogParser" ]; then
        PARSER_EXEC="./build/LogParser"
    else
        echo "[ERROR] LogParser tidak ditemukan. Hasil log mentah ada di: $RAW_LOG"
        exit 1
    fi

    # Eksekusi C++ Parser
    "$PARSER_EXEC" "$RAW_LOG" "$HW_CSV"

    # Hapus file mentah jika sukses memproses
    if [ $? -eq 0 ]; then
        rm -f "$RAW_LOG"
        echo "=== BENCHMARK SELESAI ==="
        echo "- Log FPS tersimpan di       : $FPS_CSV"
        echo "- Log Hardware tersimpan di  : $HW_CSV"
    else
        echo "[ERROR] Gagal memproses data log."
    fi
}

# ==============================================================================
# EKSEKUSI UTAMA
# ==============================================================================
echo "=== MEMULAI PROSES BENCHMARK ==="

# 1. Jalankan tegrastats di background.
# Gunakan awk untuk menambahkan timestamp di awal setiap baris secara sangat efisien.
tegrastats --interval 1000 | awk '{print strftime("%Y-%m-%d %H:%M:%S"), $0}' > "$RAW_LOG" &
TEGRA_PID=$!
echo "[INFO] Tegrastats berjalan di background (PID: $TEGRA_PID)"

# 2. Pengecekan Lokasi Executable Secara Dinamis
if [ -x "./DeepStreamZedyoloRTSP" ]; then
    EXEC_PATH="./DeepStreamZedyoloRTSP"
elif [ -x "./build/DeepStreamZedyoloRTSP" ]; then
    EXEC_PATH="./build/DeepStreamZedyoloRTSP"
else
    echo "[ERROR] File executable DeepStreamZedyoloRTSP tidak ditemukan di ../ maupun di ./build/"
    cleanup # Matikan tegrastats jika program C++ tidak ditemukan
    exit 1
fi

# 3. Jalankan aplikasi DeepStream C++ yang ditemukan
echo "[INFO] Menjalankan executable dari: $EXEC_PATH"
echo "[INFO] Tekan Ctrl+C untuk menghentikan..."

"$EXEC_PATH" --config "$CONFIG_FILE" --benchmark "$FPS_CSV"

# 4. Proses berhenti secara natural (tanpa Ctrl+C)
cleanup

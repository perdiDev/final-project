# 4. Protokol Benchmark

Metodologi pengujian dibagi menjadi **dua pilar** yang saling melengkapi. Jangan mencampur
angka dari kedua pilar ini — masing-masing diukur di hardware yang berbeda dan mengukur hal
yang berbeda.

| Pilar | Diukur di | Alat | Status |
|---|---|---|---|
| **Akurasi** (mAP, Precision, Recall) | Cloud GPU (Kaggle, Tesla T4) | Ultralytics `YOLO.val()` | ✅ Selesai — [05](05_accuracy_results.md) |
| **Runtime & Hardware** (FPS, latensi, GPU%, daya) | Jetson Orin Nano (target deployment) | `scripts/run_benchmark.sh` + `tegrastats` | 🔲 Dijalankan per model |

## 4.1 Variabel yang Dikontrol (Controlled Variables)

Supaya hasil antar model **benar-benar bisa dibandingkan** (bukan artefak perbedaan kondisi
pengujian), variabel berikut dijaga tetap sama di setiap run:

| Variabel | Nilai tetap | Alasan |
|---|---|---|
| Sumber input | File video yang sama (`data/input/video_testing.mp4`) | Live ZED feed punya konten adegan yang tidak identik antar run; file memastikan setiap model "melihat" input yang persis sama |
| Mode output | `file` (encode ke MP4) untuk seluruh model | Supaya overhead encoding/muxing konstan di semua run, tidak bias ke satu model |
| Precision | FP16 (`network-mode=2`) di semua config | Precision bukan variabel bebas pada eksperimen ini |
| Tracker config | Profil NvDCF perf bawaan DeepStream (default) | Kecuali sedang sengaja meneliti pengaruh tracker |
| Jetson power mode | Tetap (catat dengan `nvpmodel -q`, idealnya `jetson_clocks` aktif) | Perubahan power mode di tengah eksperimen membuat FPS/daya antar model tidak sebanding |
| Durasi/panjang klip | Sama untuk semua model (gunakan `--duration <detik>`) | Supaya jumlah frame yang diukur & window statistik konsisten |

**Kalau ingin mengganti satu variabel** (misal: ingin tahu pengaruh live ZED feed, atau
pengaruh output RTSP vs file), lakukan itu sebagai eksperimen terpisah yang dinyatakan
eksplisit — jangan campur dengan eksperimen utama.

## 4.2 Menjalankan Benchmark — `scripts/run_benchmark.sh`

Script ini menjalankan **satu model per eksekusi** (sengaja tidak batch-otomatis), supaya
setiap run dieksekusi secara sadar dan bisa dijeda/diulang kapan pun.

```bash
# 1. Build dulu (kalau belum)
./scripts/build.sh

# 2. Lihat model yang tersedia (otomatis terdeteksi dari config/pgie_*.txt)
./scripts/run_benchmark.sh --list

# 3a. Jalankan satu model secara interaktif (akan muncul menu pilihan)
./scripts/run_benchmark.sh

# 3b. Atau langsung tentukan model + durasi tetap (disarankan untuk skripsi:
#     durasi tetap membuat setiap run punya panjang klip yang identik)
./scripts/run_benchmark.sh --model yolov8n_kitti --duration 180
./scripts/run_benchmark.sh --model yolov9t_kitti --duration 180
./scripts/run_benchmark.sh --model yolov10n_kitti --duration 180
./scripts/run_benchmark.sh --model yolov26n_kitti --duration 180
```

Tekan `Ctrl+C` sekali untuk menghentikan lebih awal (jika tidak memakai `--duration`) — sama
seperti perilaku aplikasi utama, shutdown akan tetap mulus tanpa merusak file output.

### Output setiap run

```
data/benchmark/
└── <model>/
    └── <YYYYMMDD_HHMMSS>/
        ├── fps.csv                 # Satu baris per frame: PTS, elapsed, FPS, latency
        ├── hardware_analysis.csv   # RAM, GPU%, CPU/core, power rails (dari tegrastats)
        ├── run_info.txt            # metadata run: config, input/output, durasi, git commit, dst.
        └── pipeline_output.mp4     # hanya ada jika --output file (default)
```

Karena setiap run dibuatkan folder baru bertimestamp, **hasil run sebelumnya tidak akan
pernah tertimpa** — aman untuk mengulang beberapa kali per model (lihat §4.3).

### Format `fps.csv` per frame

Logger menghasilkan satu record untuk setiap frame dengan kolom:

```text
Timestamp,Frame_Number,Media_PTS_ms,Elapsed_ms,FPS,Latency_ms
```

- `Frame_Number`: nomor urut frame dari DeepStream.
- `Media_PTS_ms`: posisi frame pada timeline media; bernilai `-1` jika PTS tidak tersedia.
- `Elapsed_ms`: waktu wall-clock sejak pipeline mulai, untuk menghitung processing throughput.
- `FPS`: throughput pada window wall-clock sekitar satu detik; nilainya dapat berulang pada
  beberapa frame dan bernilai `0` selama window pertama belum lengkap.
- `Latency_ms`: latency frame yang dilaporkan DeepStream.

Input file dapat diproses lebih cepat daripada durasi pemutarannya. Karena itu, jumlah baris
mengikuti **jumlah frame**, bukan jumlah detik wall-clock maupun durasi media. Penulisan CSV
tetap dilakukan oleh thread terpisah dan di-flush secara berkala agar I/O tidak dilakukan di
jalur kritis pipeline.

### Kenapa `NVDS_ENABLE_LATENCY_MEASUREMENT=1` penting

DeepStream **tidak mengukur latensi per-frame secara default** — `nvds_measure_buffer_latency`
di `src/main.cpp` hanya mengembalikan nilai valid jika environment variable
`NVDS_ENABLE_LATENCY_MEASUREMENT=1` di-set sebelum pipeline berjalan. Tanpa ini, kolom
`Latency_ms` di `fps.csv` akan selalu `0`. `scripts/run_benchmark.sh` sudah otomatis
mengaktifkan ini (`export NVDS_ENABLE_LATENCY_MEASUREMENT=1`) — jika Anda menjalankan
executable secara manual tanpa script ini, jangan lupa set variabel ini sendiri.

### Jika `hardware_analysis.csv` tidak muncul

Runner memerlukan executable `tegrastats` dan `LogParser`. Verifikasi langsung di Jetson:

```bash
command -v tegrastats
tegrastats --interval 1000
ls -l ./LogParser ./build/LogParser
```

Tekan `Ctrl+C` setelah beberapa sampel `tegrastats` terlihat. Runner sekarang mencatat
`tegrastats_path` dan `hardware_log_status` di `run_info.txt`. Jika perekaman gagal,
`raw_hw.log` tidak dihapus dan stderr disimpan sebagai `hardware_recorder_error.log`, sehingga
penyebab seperti executable tidak ditemukan, izin ditolak, atau output kosong dapat diperiksa.

`tegrastats` tidak menyediakan timestamp sampel. Selain itu, outputnya dapat ter-buffer ketika
diarahkan ke pipe. Runner karena itu merekonstruksi waktu nominal dari waktu mulai recorder,
nomor sampel, dan `tegrastats_interval_ms`. Kolom hardware yang dihasilkan diawali dengan:

```text
Timestamp,Sample_Number,Hardware_Elapsed_ms,...
```

Cara ini membuat timestamp tetap maju sesuai interval meskipun beberapa baris baru diterima
bersamaan saat proses dihentikan. Timestamp tersebut adalah waktu sampling nominal; jitter
scheduler yang lebih kecil daripada interval tidak diukur.

## 4.3 Rekomendasi Jumlah Pengulangan (Repetisi)

Satu kali run per model **tidak cukup** untuk klaim ilmiah yang kuat — variasi kecil (thermal
throttling, jitter OS scheduler, dll) bisa membuat satu sampel menyesatkan. Disarankan:

1. Jalankan **minimal 3–5 repetisi** per model (ulangi `./scripts/run_benchmark.sh --model
   <nama> --duration 180` beberapa kali; masing-masing otomatis tersimpan di folder
   bertimestamp berbeda).
2. Beri jeda singkat (misal 30–60 detik) antar repetisi agar suhu SoC sempat stabil, supaya
   repetisi tidak saling mewarisi efek thermal throttling dari run sebelumnya.
3. Saat analisis (lihat [06](06_runtime_results.md)), buang **warm-up window** (misalnya 10–15
   detik pertama setiap run) sebelum menghitung rata-rata — di awal run TensorRT context
   & clock ramp-up GPU belum stabil, sehingga FPS/latensi awal tidak representatif.

## 4.4 Menjaga Kondisi Hardware Konsisten

Sebelum sesi benchmark (di Jetson, bukan di lingkungan pengembangan/sandbox):

```bash
# Cek/atur power mode (harus sama di semua sesi benchmark)
sudo nvpmodel -q
# contoh mengunci ke mode performa maksimum:
sudo nvpmodel -m 0

# Kunci clock GPU/CPU ke frekuensi maksimum supaya tidak ada variasi akibat DVFS
sudo jetson_clocks

# Bersihkan proses/cache yang mungkin mengganggu sebelum sesi benchmark
./scripts/clean_cache.sh
```

Catat `nvpmodel -q` dan status `jetson_clocks` di setiap sesi — `run_info.txt` sudah mencoba
merekam ini otomatis (lihat §4.5), tapi validasi manual tetap disarankan sebelum sesi panjang.

## 4.5 Metadata Run (`run_info.txt`)

Setiap folder run memiliki `run_info.txt` yang mencatat konfigurasi persis dari run tersebut,
supaya hasil dapat direproduksi dan dilacak balik ke commit kode yang menghasilkannya:

```
model                  : yolov8n_kitti
config_file            : config/pgie_yolov8n_kitti.txt
tracker_config         : <default DeepStream NvDCF perf profile>
input_mode             : file
input_file             : data/input/video_testing.mp4
output_mode            : file
duration_limit_s       : 180
run_id                 : 20260718_143000
started_at             : 2026-07-18 14:30:00
git_commit             : e5541d0
nvpmodel_mode          : ...
jetson_clocks_status   : ...
tegrastats_interval_ms : 1000
latency_measurement    : NVDS_ENABLE_LATENCY_MEASUREMENT=1 (diaktifkan otomatis)
finished_at            : 2026-07-18 14:33:05
```

Simpan file ini sebagai lampiran/apendiks skripsi untuk setiap sesi benchmark yang dilaporkan
di Bab 4 — ini bukti reproducibility yang biasanya ditanyakan penguji.

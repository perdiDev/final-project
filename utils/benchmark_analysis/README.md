# Benchmark Analysis

Script untuk mengagregasi hasil `scripts/run_all_benchmark.sh` (6 model x 3 tracker
= 18 skenario, folder `data/benchmark/<model>_<tracker>/<run_id>/`) menjadi tabel dan
grafik yang dipakai BAB IV skripsi (`skripsi/draft/BAB-4-Hasil-dan-Pembahasan.md`) dan
`docs/06_runtime_results.md` / `docs/07_tradeoff_analysis.md`.

Tidak mengarang angka — kalau `data/benchmark/` belum berisi run yang valid, kedua
script berhenti dengan pesan jelas dan tidak menulis file output.

## Prasyarat

```bash
pip install pandas matplotlib scipy
```

`scipy` hanya dibutuhkan untuk flag opsional `--significance` di `tradeoff_analysis.py`.

## Urutan pemakaian

### 1. Jalankan benchmark di Jetson

```bash
./scripts/run_all_benchmark.sh
```

Ulangi tiap skenario minimal 3-5 kali sesuai `docs/04_benchmark_protocol.md` §4.3 —
`run_all_benchmark.sh` sendiri hanya menjalankan tiap skenario sekali per eksekusi,
jalankan ulang scriptnya untuk menambah repetisi (folder run dibedakan lewat timestamp,
tidak saling menimpa).

### 2. Agregasi runtime & hardware

```bash
python utils/benchmark_analysis/aggregate_runtime.py
```

Menghasilkan (default ke `skripsi/eksperimen/`):

- `runtime_summary.csv` — satu baris per (model, tracker): avg/median/std FPS, p95
  latency, breakdown latency per komponen pipeline, avg GPU%, avg RAM, avg tiap rail daya.
- `runtime_per_run.csv` — data mentah per run (dipakai `--significance` di langkah 3).
- `plots/fps_boxplot_by_model.png`, `plots/fps_boxplot_by_tracker.png`.

Flag: `--bench-root`, `--warmup-s` (default 10 detik pertama dibuang), `--out-dir`.

### 3. Gabungkan dengan akurasi (trade-off)

```bash
python utils/benchmark_analysis/tradeoff_analysis.py
```

Menggabungkan `runtime_summary.csv` dengan `accuracy_reference.csv` (angka akurasi
ditranskrip dari `docs/05_accuracy_results.md`, lihat komentar header file tersebut untuk
sumber & asumsi `*_efficientnms`). Menghasilkan `tradeoff_summary.csv` dan grafik Pareto
front `plots/tradeoff_map_vs_fps.png` / `plots/tradeoff_map_vs_power.png`.

Uji signifikansi opsional (Welch's t-test antar dua skenario, format `model` atau
`model:tracker`):

```bash
python utils/benchmark_analysis/tradeoff_analysis.py \
    --significance yolov8n_kitti yolov10n_kitti
```

## File

- `common.py` — parsing `run_info.txt`/`fps.csv`/`hardware_analysis.csv`, deteksi kolom
  dinamis (rail daya, per-core CPU).
- `aggregate_runtime.py` — langkah 2.
- `accuracy_reference.csv` — transkrip manual dari `docs/05_accuracy_results.md` §5.1.
- `tradeoff_analysis.py` — langkah 3.

Setelah `tradeoff_summary.csv` terisi data asli, isi BAB IV dengan merujuk langsung ke
file-file di `skripsi/eksperimen/` — jangan menyalin angka dari sini secara manual tanpa
mengecek ulang, dan jangan mengisi BAB IV sebelum data ini benar-benar ada (lihat
`skripsi/eksperimen/README.md`).

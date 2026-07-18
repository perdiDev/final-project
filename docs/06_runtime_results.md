# 6. Hasil Runtime & Hardware (Jetson Orin Nano)

**Status: 🔲 Belum diisi.** Jalankan `scripts/run_benchmark.sh` untuk setiap model (lihat
[04_benchmark_protocol.md](04_benchmark_protocol.md)) lalu isi tabel di bawah ini dari
`fps.csv` dan `hardware_analysis.csv` pada setiap folder run.

## 6.1 Cara Mengisi Bagian Ini

1. Pastikan sudah menjalankan **minimal 3–5 repetisi** per model dengan durasi klip yang
   sama (`--duration`), sesuai §4.3.
2. Untuk setiap run, buang frame pada **warm-up window** (misal 10–15 detik pertama)
   berdasarkan `Elapsed_ms`, bukan berdasarkan jumlah baris — lihat §6.3 untuk contoh kode.
3. Hitung throughput FPS setiap run dari jumlah frame dibagi elapsed wall-clock. Gunakan
   seluruh nilai `Latency_ms` per-frame untuk mean, median, std, dan percentile.
4. Lakukan hal serupa untuk `hardware_analysis.csv` (`GPU_Persen`, `RAM_MB`, kolom power `*_mW`).
5. Isi tabel §6.2 dengan hasil agregat tersebut.

## 6.2 Tabel Ringkasan (isi setelah benchmark dijalankan)

| Model | Avg FPS | Median FPS | Std FPS | p95 Latency (ms) | Avg GPU % | Avg RAM (MB) | Avg Power (mW)* |
|---|---|---|---|---|---|---|---|
| YOLOv8n | | | | | | | |
| YOLOv9t | | | | | | | |
| YOLOv10n | | | | | | | |
| YOLO26n | | | | | | | |

*\*Sesuaikan nama kolom power dengan rail yang benar-benar muncul di `hardware_analysis.csv`
Anda (nama rail bergantung platform Jetson — misal `VDD_IN`, `VDD_CPU_GPU_CV`, dll; lihat
header CSV hasil `LogParser`).*

## 6.3 Contoh Skrip Agregasi (Python + pandas)

Simpan sebagai `data/utils/aggregate_runtime.py` (atau jalankan langsung di notebook) setelah
Anda memiliki beberapa folder run per model:

```python
import glob
import pandas as pd

WARMUP_SECONDS = 2  # gunakan 10–15 untuk run panjang; jangan melebihi durasi proses aktual

def load_runs(model_name: str) -> pd.DataFrame:
    frames = []
    for run_dir in glob.glob(f"data/benchmark/{model_name}/*/"):
        fps_path = f"{run_dir}fps.csv"
        try:
            df = pd.read_csv(fps_path)
        except FileNotFoundError:
            continue
        df = df[df["Elapsed_ms"] >= WARMUP_SECONDS * 1000].copy()
        if df.empty:
            continue
        df["run_dir"] = run_dir
        frames.append(df)
    return pd.concat(frames, ignore_index=True) if frames else pd.DataFrame()

def throughput_per_run(df: pd.DataFrame) -> pd.Series:
    results = {}
    for run_dir, run in df.groupby("run_dir"):
        elapsed_s = (run["Elapsed_ms"].max() - run["Elapsed_ms"].min()) / 1000
        results[run_dir] = (
            (len(run) - 1) / elapsed_s
            if elapsed_s > 0 and len(run) > 1
            else float("nan")
        )
    return pd.Series(results, name="FPS")

for model in ["yolov8n_kitti", "yolov9t_kitti", "yolov10n_kitti", "yolov26n_kitti"]:
    df = load_runs(model)
    if df.empty:
        print(f"{model}: belum ada data run")
        continue
    run_fps = throughput_per_run(df)
    print(f"--- {model} ---")
    print(f"Avg FPS antar-run   : {run_fps.mean():.2f}")
    print(f"Median FPS antar-run: {run_fps.median():.2f}")
    print(f"Std FPS antar-run   : {run_fps.std():.2f}")
    print(f"p95 Latency (ms)    : {df['Latency_ms'].quantile(0.95):.2f}")
```

Kolom `FPS` pada CSV adalah nilai window satu detik yang diulang pada setiap frame. Jangan
langsung merata-ratakan kolom tersebut karena window ber-FPS tinggi akan memiliki lebih banyak
baris dan mendapat bobot lebih besar. Rumus di atas menghitung throughput setiap run langsung
dari jumlah frame dan `Elapsed_ms`.

Lakukan hal yang sama untuk `hardware_analysis.csv` (gabungkan dengan `pd.merge_asof` pada
kolom `Timestamp` jika ingin mengaitkan FPS dengan GPU%/power pada waktu yang sama).

## 6.4 Visualisasi yang Disarankan

- **Box plot / violin plot** distribusi FPS per model (bukan cuma rata-rata) — menunjukkan
  stabilitas, bukan hanya kecepatan rata-rata.
- **Time-series overlay**: FPS-vs-waktu dan GPU%-vs-waktu untuk semua model dalam satu grafik
  (sumbu waktu sama) — untuk mendeteksi thermal throttling pada run yang panjang.
- **Bar chart** perbandingan Avg Power dan Avg GPU% antar model.

```python
import matplotlib.pyplot as plt

fig, ax = plt.subplots()
ax.boxplot([df_v8n["FPS"], df_v9t["FPS"], df_v10n["FPS"], df_v26n["FPS"]],
           tick_labels=["YOLOv8n", "YOLOv9t", "YOLOv10n", "YOLO26n"])
ax.set_ylabel("FPS")
ax.set_title("Distribusi FPS per Model (Jetson Orin Nano)")
plt.savefig("docs/assets/fps_boxplot.png", dpi=150, bbox_inches="tight")
```

(Buat folder `docs/assets/` untuk menyimpan grafik yang dihasilkan, supaya bisa disisipkan
langsung ke naskah skripsi.)

## 6.5 Setelah Tabel Ini Terisi

Lanjutkan ke [07_tradeoff_analysis.md](07_tradeoff_analysis.md) untuk menggabungkan hasil ini
dengan tabel akurasi di [05_accuracy_results.md](05_accuracy_results.md).

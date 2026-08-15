# 7. Analisis Trade-off (Akurasi vs. Kecepatan vs. Daya)

**Status: 🔲 Menunggu data runtime lengkap** dari
[06_runtime_results.md](06_runtime_results.md). Bagian ini menjelaskan **metodologi**
analisisnya; isi grafik/tabel setelah data runtime tersedia.

## 7.1 Kenapa Trade-off, Bukan "Model Terbaik" Tunggal?

Tabel akurasi di [05](05_accuracy_results.md) dan tabel runtime di [06](06_runtime_results.md)
hampir pasti **tidak** menunjuk ke satu model yang menang di semua sumbu sekaligus — pola umum
di edge-AI adalah semakin ringan model, semakin cepat/hemat daya, tapi sedikit menurun
akurasinya. Karena itu, kesimpulan skripsi Bab 4/5 sebaiknya berbentuk **rekomendasi
berkondisi** ("model X untuk prioritas akurasi, model Y untuk prioritas efisiensi daya"),
bukan klaim satu pemenang absolut — ini pendekatan standar pada paper-paper edge-AI/ADAS.

## 7.2 Menggabungkan Data

Setelah [06](06_runtime_results.md) terisi, satukan dengan tabel akurasi menjadi satu tabel
master:

| Model | mAP50-95 | Avg FPS (Jetson) | p95 Latency (ms) | Avg GPU % | Avg Power (mW) | GFLOPs |
|---|---|---|---|---|---|---|
| YOLOv8n | 0.8397 | ? | ? | ? | ? | 8.1 |
| YOLOv9t | 0.8120 | ? | ? | ? | ? | 7.6 |
| YOLOv10n | 0.8370 | ? | ? | ? | ? | 6.5 |
| YOLO26n | 0.8233 | ? | ? | ? | ? | 5.2 |

## 7.3 Scatter Plot Akurasi vs. Kecepatan (Pareto Front)

```python
import pandas as pd
import matplotlib.pyplot as plt

data = pd.DataFrame({
    "model": ["YOLOv8n", "YOLOv9t", "YOLOv10n", "YOLO26n"],
    "map50_95": [0.8397, 0.8120, 0.8370, 0.8233],
    "avg_fps": [None, None, None, None],  # isi dari tabel 7.2
})

fig, ax = plt.subplots()
ax.scatter(data["avg_fps"], data["map50_95"])
for _, row in data.iterrows():
    ax.annotate(row["model"], (row["avg_fps"], row["map50_95"]))
ax.set_xlabel("Avg FPS (Jetson Orin Nano)")
ax.set_ylabel("mAP50-95 (KITTI val)")
ax.set_title("Trade-off Akurasi vs. Kecepatan")
plt.savefig("docs/assets/tradeoff_map_vs_fps.png", dpi=150, bbox_inches="tight")
```

Buat grafik serupa untuk **mAP50-95 vs. Avg Power** dan (opsional) **mAP50-95 vs. GFLOPs**
(sumbu GFLOPs sudah bisa diplot sekarang karena tidak butuh data Jetson — bisa jadi grafik
"pemanasan" sebelum data runtime lengkap).

### Cara membaca grafik Pareto front

Titik yang berada di "sudut kanan-atas" grafik (FPS tinggi **dan** mAP tinggi) mendominasi
titik-titik di "kiri-bawahnya" — titik dominan itulah kandidat rekomendasi utama. Titik yang
tidak didominasi oleh titik manapun (tidak ada model lain yang lebih baik di FPS **dan** mAP
sekaligus) berada di **Pareto front** — biasanya lebih dari satu model berakhir di front ini,
dan itulah dasar untuk rekomendasi berkondisi di §7.1.

## 7.4 Uji Signifikansi (opsional, untuk memperkuat klaim)

Karena Anda menjalankan beberapa repetisi per model (§4.3 di
[04_benchmark_protocol.md](04_benchmark_protocol.md)), Anda punya distribusi FPS/latensi per
model, bukan cuma satu angka. Untuk klaim "model A signifikan lebih cepat dari model B" (bukan
cuma "angkanya lebih besar"), lakukan uji sederhana:

```python
from scipy import stats

# fps_a, fps_b = pd.Series hasil FPS dari seluruh repetisi masing-masing model
t_stat, p_value = stats.ttest_ind(fps_a, fps_b, equal_var=False)
print(f"p-value: {p_value:.4f}")  # p < 0.05 → perbedaan dianggap signifikan
```

Ini opsional untuk skripsi S1 (melaporkan mean ± std dari ≥5 repetisi biasanya sudah cukup),
tapi jika ada waktu, ini mempercantik Bab 4 dan menunjukkan kehati-hatian metodologis.

## 7.5 Struktur Kesimpulan yang Disarankan

Setelah grafik trade-off selesai, susun kesimpulan Bab 5 dalam bentuk seperti:

1. **Jika prioritas akurasi maksimum** (misal untuk kasus deteksi jarak jauh/kritikal
   keselamatan) → rekomendasikan model dengan mAP tertinggi meski FPS sedikit lebih rendah,
   *dengan syarat FPS tersebut masih memenuhi kebutuhan real-time* (≥ 30 FPS, mengikuti
   konfigurasi default kamera ZED pada protokol benchmark dan standar ADAS *safety-critical*).
2. **Jika prioritas efisiensi daya/computolah (untuk deployment jangka panjang, battery-
   constrained, atau berbagi compute dengan modul ADAS lain)** → rekomendasikan model dengan
   GFLOPs/power terendah yang akurasinya masih dalam toleransi yang dapat diterima.
3. **Model yang berada di Pareto front tapi tidak ekstrem di kedua sisi** → biasanya inilah
   rekomendasi "default" yang paling aman untuk dilaporkan sebagai kesimpulan utama skripsi.

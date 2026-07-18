# 5. Hasil Akurasi (KITTI Validation Set)

**Status: Selesai.** Diukur dengan `best_model.val(data=yaml_path)` (Ultralytics) di Kaggle
(GPU Tesla T4), pada **val set yang identik** untuk keempat model: 1010 gambar, 4722 instance
(car 4167, van 405, truck 150). Karena split-nya identik, angka di bawah bisa dibandingkan
langsung antar model.

> ⚠️ Ingat caveat FP32 vs FP16 dari [03_deployment_pipeline.md](03_deployment_pipeline.md) §3.4
> — ini adalah akurasi bobot `.pt` (FP32), dipakai sebagai proxy akurasi deployment FP16.

## 5.1 Tabel Utama

| Model | Params | GFLOPs | mAP50 | mAP50-95 | Precision | Recall |
|---|---|---|---|---|---|---|
| YOLOv8n | 3,006,233 | 8.1 | **0.9767** | **0.8397** | **0.9696** | 0.9344 |
| YOLOv9t | 1,971,369 | 7.6 | 0.9670 | 0.8120 | 0.9643 | 0.9259 |
| YOLOv10n | 2,265,753 | 6.5 | 0.9704 | 0.8370 | 0.9689 | 0.9189 |
| YOLO26n | 2,375,421 | **5.2** | 0.9706 | 0.8233 | 0.9508 | **0.9297** |

(**Bold** = nilai terbaik pada kolom tersebut.)

## 5.2 Per Kelas — mAP50-95

| Model | Car | Van | Truck |
|---|---|---|---|
| YOLOv8n | 0.847 | 0.812 | 0.860 |
| YOLOv9t | 0.823 | 0.783 | 0.830 |
| YOLOv10n | 0.845 | 0.817 | 0.849 |
| YOLO26n | 0.838 | 0.806 | 0.825 |

## 5.3 Per Kelas — Precision / Recall

| Model | Car (P/R) | Van (P/R) | Truck (P/R) |
|---|---|---|---|
| YOLOv8n | 0.967 / 0.939 | 0.963 / 0.911 | 0.978 / 0.953 |
| YOLOv9t | 0.960 / 0.929 | 0.956 / 0.902 | 0.978 / 0.947 |
| YOLOv10n | 0.959 / 0.931 | 0.963 / 0.899 | 0.985 / 0.927 |
| YOLO26n | 0.944 / 0.943 | 0.945 / 0.906 | 0.964 / 0.940 |

## 5.4 Temuan & Diskusi

1. **Akurasi tidak berbanding lurus dengan ukuran model.** YOLOv8n memiliki parameter dan
   FLOPs terbesar di antara keempatnya, dan juga akurasi terbaik (mAP50-95 0.8397) — namun
   YOLOv10n mencapai 0.8370 (selisih hanya 0.3 poin) dengan **~20% lebih sedikit FLOPs**
   (6.5 vs 8.1 GFLOPs). Ini temuan utama untuk analisis trade-off efisiensi (lihat
   [07_tradeoff_analysis.md](07_tradeoff_analysis.md)): YOLOv10n kemungkinan adalah
   *"best accuracy-per-FLOP"* di antara keempat model, sebelum melihat data runtime Jetson.

2. **YOLOv9t adalah anomali menarik.** Parameternya paling kecil (1.97M) tetapi *inference
   time* di T4 justru paling lambat (2.6ms vs ~1.9-2.0ms model lain) dan akurasinya paling
   rendah (mAP50-95 0.8120). Ini mengindikasikan jumlah parameter **tidak selalu berkorelasi
   dengan latensi** — arsitektur PGI/GELAN pada YOLOv9 kemungkinan lebih sequential/memory-bound
   dibanding backbone v8/v10, sehingga lebih sedikit bobot tidak otomatis berarti lebih cepat.
   *(Catatan: angka 2.6ms ini dari T4, bukan Jetson — perlu dikonfirmasi apakah pola yang sama
   terulang di Jetson lewat [06_runtime_results.md](06_runtime_results.md)).*

3. **YOLO26n punya compute paling rendah (5.2 GFLOPs)** namun mAP50 tetap kompetitif (0.9706,
   hampir identik dengan YOLOv10n 0.9704). Precision-nya paling rendah di antara semua model
   (0.9508) tapi Recall-nya justru tertinggi kedua (0.9297) — menunjukkan YOLO26n cenderung
   sedikit lebih "agresif" mendeteksi (lebih banyak deteksi valid, tapi juga sedikit lebih
   banyak false positive) dibanding model lain. Ini konsisten dengan filosofi desain YOLO26
   yang menghilangkan NMS/DFL demi kecepatan (lihat
   [02_dataset_and_training.md](02_dataset_and_training.md)).

4. **Ketimpangan kelas (class imbalance) membuat metrik `van`/`truck` kurang stabil secara
   statistik** dibanding `car` — sampel truck hanya 150 instance. Selisih kecil pada mAP kelas
   `truck` antar model (0.825–0.860) bisa jadi sebagian dipengaruhi oleh varians sampel kecil,
   bukan murni perbedaan kemampuan model. Sebutkan ini sebagai limitasi saat membahas hasil
   per-kelas (lihat [08_limitations_future_work.md](08_limitations_future_work.md)).

5. **Kecepatan inferensi T4 (kolom "Speed" di output Ultralytics) tidak representatif untuk
   Jetson Orin Nano** — jangan dikutip sebagai bagian dari bab hasil runtime. Nilai itu hanya
   berguna sebagai sanity-check relatif antar model pada eksperimen akurasi ini, bukan angka
   performa deployment yang sebenarnya. Angka performa Jetson yang valid ada di
   [06_runtime_results.md](06_runtime_results.md).

## 5.5 Rangking Sementara (akurasi saja, belum termasuk runtime)

| Rank | Model | Alasan |
|---|---|---|
| 1 | YOLOv8n | mAP50-95 tertinggi, precision tertinggi |
| 2 | YOLOv10n | Akurasi hampir menyamai YOLOv8n dengan FLOPs jauh lebih rendah |
| 3 | YOLO26n | FLOPs terendah, akurasi kompetitif, tapi precision terendah |
| 4 | YOLOv9t | Akurasi terendah dan (di T4) bukan yang tercepat |

**Rangking ini bisa berubah total setelah data runtime Jetson tersedia** — model dengan
akurasi sedikit lebih rendah tapi FPS/efisiensi daya jauh lebih baik bisa jadi rekomendasi
akhir yang lebih tepat untuk deployment ADAS. Jangan simpulkan model "terbaik" hanya dari
tabel ini — lanjutkan ke [06](06_runtime_results.md) dan [07](07_tradeoff_analysis.md).

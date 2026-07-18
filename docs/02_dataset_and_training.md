# 2. Dataset & Training

## 2.1 Dataset: KITTI (subset 3 kelas)

Proyek ini memakai dataset **KITTI Object Detection** yang telah dipetakan ulang ke label
custom 3 kelas (lihat `labels/labels_kitti_custom.txt`):

```
0: car
1: van
2: truck
```

KITTI dipilih karena (lihat juga [01_scope_and_architecture.md](01_scope_and_architecture.md)
untuk alasan lengkap) merepresentasikan domain jalan raya/otomotif secara langsung, berbeda
dari dataset generik seperti COCO.

### Statistik split validasi (val set) — terverifikasi dari hasil evaluasi

Keempat model dievaluasi pada **val set yang identik** (1010 gambar) — ini penting supaya
perbandingan antar model adil (semua model "diuji dengan soal yang sama").

| Kelas | Jumlah Gambar Mengandung Kelas Ini | Jumlah Instance (bounding box) |
|---|---|---|
| car | 992 | 4167 |
| van | 310 | 405 |
| truck | 147 | 150 |
| **Total** | **1010 gambar** | **4722 instance** |

> ⚠️ **Catatan ketidakseimbangan kelas (class imbalance):** instance `car` (4167) jauh lebih
> banyak daripada `van` (405) dan `truck` (150). Ini membuat metrik per-kelas untuk `van` dan
> `truck` secara statistik kurang stabil dibanding `car` — sampel truck hanya 150 instance.
> Ini **harus** disebutkan sebagai limitasi saat membahas hasil per-kelas di skripsi (lihat
> [08_limitations_future_work.md](08_limitations_future_work.md)).

### 🔲 TODO — lengkapi dari notebook training Anda

Bagian berikut **wajib diisi dari notebook Kaggle/training Anda** — jangan menebak angka ini,
karena akan menjadi bagian dari Bab 3 (Metodologi) skripsi Anda dan harus akurat:

| Item | Nilai (isi di sini) |
|---|---|
| Rasio train/val/test split | ? (misal 80/20, atau train/val/test 70/20/10) |
| Random seed split | ? |
| Jumlah gambar training | ? |
| Ukuran citra input (`imgsz`) | ? |
| Jumlah epoch | ? |
| Batch size training | ? |
| Optimizer | ? (SGD/AdamW/MuSGD — YOLO26 memakai optimizer baru **MuSGD**, lihat catatan di
  bawah) |
| Augmentasi data | ? (mosaic, flip, HSV jitter, dll — cek `args.yaml` hasil training) |
| Pretrained weights awal | ? (COCO pretrained / from scratch) |
| Sumber/tanggal training | ? (link notebook Kaggle masing-masing model) |

Tips: Ultralytics menyimpan seluruh hyperparameter training di file `args.yaml` pada folder
`runs/detect/train*/`. Salin isinya ke sini atau lampirkan sebagai apendiks skripsi.

## 2.2 Model yang Dibandingkan

| Model | Params | GFLOPs | Layers (fused) | Catatan Arsitektur |
|---|---|---|---|---|
| YOLOv8n | 3,006,233 | 8.1 | 73 | Baseline anchor-free + DFL, arsitektur paling matang/stabil |
| YOLOv9t | 1,971,369 | 7.6 | 197 | PGI (Programmable Gradient Information) + GELAN backbone |
| YOLOv10n | 2,265,753 | 6.5 | 102 | NMS-free training (dual assignment), head lebih ringkas |
| YOLO26n | 2,375,421 | 5.2 | 122 | Rilis Ultralytics terbaru (Jan 2026) — lihat catatan khusus di bawah |

Selain 4 model KITTI di atas, terdapat satu model tambahan:

| Model | Kelas | Peran dalam Penelitian |
|---|---|---|
| YOLOv8n (COCO, 80 kelas) | 80 kelas umum | **Baseline sanity-check** pipeline DeepStream saja, bukan pembanding utama akurasi domain otomotif |

### Catatan khusus: YOLO26n bukan sekadar "generasi lebih baru"

YOLO26 (rilis resmi Ultralytics, Januari 2026) punya perbedaan arsitektur yang cukup
mendasar dibanding v8/v9/v10, bukan cuma tuning kecil:

- **NMS-free end-to-end inference** secara default (prediksi akhir tidak melalui tahap
  Non-Maximum Suppression terpisah).
- **Tanpa Distribution Focal Loss (DFL)** pada head regresi box — head deteksinya lebih
  ringan/sederhana dibanding v8/v9/v10 yang masih memakai DFL.
- Optimizer training baru: **MuSGD** (hybrid SGD + Muon).

**Implikasi penting untuk deployment**: pipeline `nvinfer` di proyek ini memakai custom
parser (`libnvdsinfer_custom_impl_Yolo.so`, fungsi `NvDsInferParseYolo`) dan konfigurasi
`nms-iou-threshold` di `[class-attrs-all]` yang diasumsikan sama untuk keempat model. Karena
YOLO26 secara desain **tidak membutuhkan NMS terpisah**, perlu diverifikasi apakah:
1. Format output tensor ONNX YOLO26n benar-benar cocok diparse oleh parser custom yang sama
   dengan v8/v9/v10, atau
2. `nms-iou-threshold=0.45` di `config/pgie_yolov26n_kitti.txt` benar-benar diperlukan/berefek
   untuk model ini, atau redundan.

Karena hasil mAP YOLO26n dari validasi Ultralytics terlihat wajar (mAP50 0.9706, sebanding
dengan model lain), kemungkinan besar ekspor ONNX-nya sudah benar. Namun ini tetap perlu
dicatat sebagai **item verifikasi**, bukan diasumsikan otomatis benar — lihat
[03_deployment_pipeline.md](03_deployment_pipeline.md) §3.4 dan
[08_limitations_future_work.md](08_limitations_future_work.md).

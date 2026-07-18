# Dokumentasi Proyek — ADAS Perception Layer Benchmark

Dokumentasi ini mendukung Tugas Akhir (S1) yang membahas **layer persepsi (perception layer)**
pada sistem ADAS (Advanced Driver Assistance System), menggunakan deteksi objek real-time
berbasis YOLO yang dijalankan di atas **NVIDIA DeepStream 7.1** pada perangkat edge
**Jetson Orin Nano**, dengan kamera stereo **Stereolabs ZED** sebagai sumber video.

Empat varian model (YOLOv8n, YOLOv9t, YOLOv10n, YOLO26n) yang di-*fine-tune* pada dataset
**KITTI** (kelas `car`, `van`, `truck`) dibandingkan pada dua sumbu utama: **akurasi deteksi**
dan **performa runtime/daya** di atas hardware target.

## Daftar Isi

1. [Scope & Arsitektur Sistem](01_scope_and_architecture.md) — batasan penelitian, arsitektur
   pipeline, dan alasan pemilihan setiap komponen teknologi ("kenapa pilih ini/itu").
2. [Dataset & Training](02_dataset_and_training.md) — KITTI dataset, kelas objek, dan ringkasan
   model yang dibandingkan.
3. [Deployment Pipeline](03_deployment_pipeline.md) — proses ekspor model (`.pt` → `.onnx` →
   TensorRT engine) dan integrasi ke DeepStream.
4. [Protokol Benchmark](04_benchmark_protocol.md) — metodologi pengujian akurasi & performa
   runtime, cara mereproduksi hasil dengan `scripts/run_benchmark.sh`.
5. [Hasil Akurasi](05_accuracy_results.md) — hasil validasi mAP/Precision/Recall keempat model
   pada KITTI val set (**sudah lengkap**).
6. [Hasil Runtime & Hardware](06_runtime_results.md) — template hasil FPS/latensi/daya di Jetson
   Orin Nano (**diisi setelah benchmark dijalankan**).
7. [Analisis Trade-off](07_tradeoff_analysis.md) — analisis akurasi vs. kecepatan vs. daya
   (Pareto front).
8. [Limitasi & Pekerjaan Selanjutnya](08_limitations_future_work.md).

## Status Pengerjaan

| Tahap | Status |
|---|---|
| Pipeline DeepStream + benchmarking harness (FPS/latency/hardware) | ✅ Selesai |
| Training 4 model (YOLOv8n, v9t, v10n, YOLO26n) pada KITTI | ✅ Selesai |
| Evaluasi akurasi (mAP/P/R) via Ultralytics `val()` | ✅ Selesai — lihat [05](05_accuracy_results.md) |
| Benchmark runtime per-model di Jetson (`scripts/run_benchmark.sh`) | 🔲 Jalankan lalu isi [06](06_runtime_results.md) |
| Analisis trade-off akhir (akurasi vs FPS vs daya) | 🔲 Setelah data runtime lengkap |

## Cara Pakai Cepat

```bash
# Lihat model apa saja yang bisa di-benchmark
./scripts/run_benchmark.sh --list

# Jalankan benchmark untuk satu model tertentu, non-interaktif
./scripts/run_benchmark.sh --model yolov8n_kitti --duration 180

# Atau jalankan tanpa argumen untuk memilih model lewat menu interaktif
./scripts/run_benchmark.sh
```

Detail lengkap ada di [04_benchmark_protocol.md](04_benchmark_protocol.md).

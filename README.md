### 📄 Mulai Salin Dari Sini

# DeepStream ZED YOLOv8 RTSP Pipeline

Proyek ini mengintegrasikan kamera **Stereolabs ZED** dengan **NVIDIA DeepStream SDK** dan **YOLOv8** untuk menjalankan *pipeline* deteksi objek secara *real-time*. Hasil deteksi (*bounding box* dan label) akan digambar ke dalam *frame* video dan dipancarkan (*streaming*) melalui server RTSP internal.

Proyek ini dirancang khusus dan dioptimalkan untuk perangkat NVIDIA Jetson (seperti Orin Nano) yang menggunakan *software encoding* (`x264enc`) karena ketiadaan *hardware encoder* (NVENC).

---

## 📂 Struktur Direktori

Struktur folder telah diatur ulang agar lebih rapi dan modular:

```text
.
├── CMakeLists.txt
├── config/
│   ├── deepstream_app.txt
│   ├── pgie_coco.txt
│   └── pgie_yolov8n.txt
├── DeepStreamZedyoloRTSP (File Executable)
├── labels/
│   ├── labels_coco.txt
│   └── labels_kitti_custom.txt
├── lib/
│   └── libnvdsinfer_custom_impl_Yolo.so
├── models/
│   ├── yolov8n_coco.onnx
│   ├── yolov8n_coco.onnx_b1_gpu0_fp16.engine
│   ├── yolov8n.engine
│   └── yolov8n.onnx
└── src/
    └── main.cpp

```

---

## 🛠️ Prerequisites (Persyaratan Sistem)

Sebelum melakukan kompilasi (*build*), pastikan perangkat keras dan perangkat lunak berikut sudah terinstal di NVIDIA Jetson Anda:

1. **NVIDIA JetPack & DeepStream SDK** (v6.0 atau lebih baru)
2. **GStreamer & Plugins Dasar**
```bash
sudo apt-get update
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-ugly gstreamer1.0-tools gstreamer1.0-rtsp

```


3. **ZED SDK**
* Unduh ZED SDK terbaru untuk platform JetPack Anda dari [Website Resmi Stereolabs](https://www.stereolabs.com/developers/release/).
* Jalankan file `.run` yang diunduh (misal: `chmod +x ZED_SDK_*.run && ./ZED_SDK_*.run`).


4. **ZED GStreamer Plugin (`gst-zed`)**
Plugin ini wajib agar DeepStream bisa membaca *feed* kamera ZED menggunakan elemen `zedsrc`.
```bash
git clone [https://github.com/stereolabs/gst-zed.git](https://github.com/stereolabs/gst-zed.git)
cd gst-zed
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install

```


*(Pastikan Anda bisa menjalankan `gst-inspect-1.0 zedsrc` tanpa error setelah instalasi ini).*

---

## ⚠️ Perhatian Penting: Pembaruan Path (Jalur File)

Karena struktur folder telah dirapikan, Anda **wajib** memastikan konfigurasi di dalam *source code* menunjuk ke direktori yang baru.

**1. Di dalam `src/main.cpp`:**
Pastikan properti `config-file-path` menunjuk ke dalam folder `config/`.

```cpp
g_object_set(G_OBJECT(pgie), "config-file-path", "config/pgie_yolov8n.txt", NULL);

```

**2. Di dalam `config/pgie_yolov8n.txt`:**
Perbarui jalur file agar DeepStream bisa menemukan model, *library custom parser*, dan file label.

```ini
[property]
onnx-file=../models/yolov8n_coco.onnx
model-engine-file=../models/yolov8n_coco.onnx_b1_gpu0_fp16.engine
labelfile-path=../labels/labels_coco.txt
custom-lib-path=../lib/libnvdsinfer_custom_impl_Yolo.so

# Konfigurasi Wajib YOLOv8
model-color-format=0
net-scale-factor=0.0039215697906911373
cluster-mode=2

```

---

## 🚀 Cara Kompilasi (Build)

Gunakan perintah standar `CMake` dan `make` untuk mengkompilasi *source code* C++ Anda.

```bash
# 1. Buat direktori build
mkdir build && cd build

# 2. Jalankan CMake untuk membuat Makefile
cmake ..

# 3. Kompilasi program menggunakan semua core CPU
make -j$(nproc)

# 4. (Opsional) Pindahkan executable ke root folder jika diperlukan
mv DeepStreamZedyoloRTSP ../
cd ..

```

---

## 🏃 Cara Menjalankan Aplikasi

Jalankan *executable* yang telah dikompilasi:

```bash
./DeepStreamZedyoloRTSP

```

Jika berhasil, Anda akan melihat log proses pembuatan *engine* TensorRT (jika belum ada) dan terminal akan mencetak:
`*** RTSP Stream is READY at rtsp://<Jetson-IP>:8554/ds-test ***`

Aplikasi ini sudah dilengkapi dengan **Signal Handler**. Untuk mematikan kamera dan menghentikan *pipeline* secara aman (agar kamera tidak *nyangkut*), cukup tekan `Ctrl+C` satu kali dan tunggu program membersihkan *resource*.

---

## 📺 Cara Menonton Stream (Client Side)

Untuk memutar *stream* video RTSP dari komputer lain (berada dalam satu jaringan WiFi/LAN) dengan **latensi serendah mungkin**, gunakan FFmpeg/FFplay.

Buka terminal di komputer *client* (bukan di Jetson) dan jalankan:

```bash
ffplay -fflags nobuffer -flags low_delay -framedrop -rtsp_transport tcp rtsp://<IP_ADDRESS_JETSON>:8554/ds-test

```

*Ganti `<IP_ADDRESS_JETSON>` dengan alamat IP dari Jetson Anda.*

---


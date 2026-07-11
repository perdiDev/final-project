# DeepStream ZED YOLOv8 RTSP Pipeline

Proyek ini mengintegrasikan kamera **Stereolabs ZED** dengan **NVIDIA DeepStream 7.1 SDK** dan **YOLOv8** untuk menjalankan *pipeline* deteksi objek secara *real-time* di perangkat **NVIDIA Jetson Orin Nano**. Hasil deteksi berupa *bounding box* dan label kelas digambar secara dinamis pada video dan disalurkan (*streaming*) melalui RTSP Server, ditampilkan di monitor lokal, atau disimpan langsung ke dalam file video.

Kode aplikasi ini telah direfaktor menggunakan standar C++17 modern untuk performa optimal, manajemen memori berbasis RAII, keamanan multithreading, dan penghentian program secara mulus tanpa mengorbankan performa *real-time* perangkat Jetson.

---

## ✨ Fitur & Keunggulan Baru

- **Arsitektur C++ Modern (C++17)**: Tidak ada lagi variabel global mutable dan pengelolaan resource secara manual. Manajemen resource dikemas di dalam class `DeepStreamApplication` dengan siklus hidup (RAII) yang bersih.
- **Dukungan Hardware & Software Encoder**: Aplikasi akan mencoba menggunakan hardware-accelerated encoder NVIDIA Jetson (`nvv4l2h264enc`) dengan zero-copy buffers. Jika tidak tersedia (seperti pada beberapa lingkungan setup development), sistem akan otomatis beralih (*fallback*) menggunakan software encoder CPU (`x264enc`).
- **Input & Output Sangat Fleksibel**: 
  - **Input**: Mendukung kamera live **ZED** (`zedsrc`) atau file video lokal (menggunakan `uridecodebin`).
  - **Output**: Mendukung pancaran jaringan **RTSP**, display monitor lokal **NV3D** (`nv3dsink`), atau penyimpanan aman file video lokal **MP4**.
- **Normalisasi Format Video Otomatis**: Semua input (termasuk file video non-NV12 atau software decoded) secara otomatis dikonversi ke format `NV12` di dalam memori NVMM sebelum dikirim ke `nvstreammux` untuk mencegah ketidakcocokan format.
- **Graceful Shutdown Tanpa Data Korup**: Penanganan sinyal `Ctrl+C` (`SIGINT`/`SIGTERM`) terintegrasi langsung dengan GMainLoop bawaan GLib (`g_unix_signal_add`). Jika Anda menyimpan output ke file MP4, sistem akan mengirimkan sinyal EOS (*End of Stream*) dan menunggu hingga 10 detik agar file tertutup dengan aman sebelum program mati.
- **Asynchronous Benchmark Logging**: Logger untuk FPS dan latensi berjalan pada thread terpisah berbasis queue thread-safe non-blocking GLib (`GAsyncQueue`), memastikan operasi I/O file tidak mengganggu frame rate utama.

---

## 📂 Struktur Direktori

```text
.
├── CMakeLists.txt              # Script build CMake (mendukung standard C++17)
├── config/
│   ├── deepstream_app.txt      # Konfigurasi referensi DeepStream
│   ├── pgie_coco.txt           # Konfigurasi model COCO
│   └── pgie_yolov8n.txt        # Konfigurasi nvinfer untuk YOLOv8n
├── labels/
│   ├── labels_coco.txt         # Nama label kelas dataset COCO
│   └── labels_kitti_custom.txt # Nama label kelas dataset KITTI
├── lib/
│   └── libnvdsinfer_custom_impl_Yolo.so # Custom bounding box parser YOLO
├── models/
│   ├── yolov8n.engine          # File engine TensorRT YOLOv8 (FP16/INT8)
│   └── yolov8n.onnx            # File model ONNX asli
├── src/
│   └── main.cpp                # Source code aplikasi utama (direfaktor)
└── data/
    ├── input/
    │   └── video_testing.mp4   # Sampel file video pengujian
    └── output/                 # Folder tujuan penyimpanan file MP4 output
```

---

## 🛠️ Persyaratan Sistem (Prerequisites)

Sebelum melakukan kompilasi, pastikan perangkat keras dan perangkat lunak berikut sudah terpasang di NVIDIA Jetson Anda:

### 1. Sistem Operasi & SDK Utama
- Perangkat **NVIDIA Jetson** dengan **JetPack 6.x** atau lebih baru.
- **NVIDIA DeepStream SDK 7.1** (atau yang kompatibel).

### 2. GStreamer & Dependensi Build
Pasang pustaka pengembangan GStreamer dan RTSP Server melalui APT:
```bash
sudo apt-get update
sudo apt-get install -y \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-tools \
    gstreamer1.0-rtsp \
    libgstrtspserver-1.0-dev \
    libglib2.0-dev \
    cmake \
    build-essential
```

### 3. ZED SDK & GStreamer Plugin
- Unduh dan pasang **ZED SDK** terbaru untuk versi JetPack Anda melalui [Situs Resmi Stereolabs](https://www.stereolabs.com/developers/release/).
- Pasang plugin GStreamer ZED (`gst-zed`) agar elemen `zedsrc` dapat dibaca oleh DeepStream:
```bash
git clone https://github.com/stereolabs/gst-zed.git
cd gst-zed
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```
*Verifikasi instalasi plugin dengan menjalankan perintah: `gst-inspect-1.0 zedsrc`.*

---

## ⚠️ Konfigurasi Penting YOLOv8

Agar modul *Primary GID Inference* (`nvinfer`) dapat berjalan dengan benar, pastikan file konfigurasi model Anda telah merujuk ke lokasi file yang tepat.

Buka file `config/pgie_yolov8n.txt` dan perbarui path jika diperlukan:
```ini
[property]
# Path model ONNX dan engine TensorRT Anda
onnx-file=../models/yolov8n.onnx
model-engine-file=../models/yolov8n.engine

# Path custom bounding box parser YOLOv8
custom-lib-path=../lib/libnvdsinfer_custom_impl_Yolo.so
parse-bbox-func-name=NvDsInferParseYolo

# Path file label kelas
labelfile-path=../labels/labels_coco.txt

# Konfigurasi format & jaringan
model-color-format=0
net-scale-factor=0.0039215697906911373
network-mode=2 # 2 = FP16 precision (sangat direkomendasikan untuk Jetson Orin Nano)
cluster-mode=4
```

---

## 🚀 Cara Kompilasi (Build)

Gunakan perintah `cmake` standar untuk mengkompilasi aplikasi Anda:

```bash
# 1. Buat dan masuk ke direktori build
mkdir -p build && cd build

# 2. Generate file Makefile menggunakan CMake
cmake ..

# 3. Kompilasi program menggunakan seluruh core CPU yang tersedia
make -j$(nproc)

# 4. Pindahkan executable hasil kompilasi ke direktori root proyek
mv DeepStreamZedyoloRTSP ../
cd ..
```

---

## 🏃 Cara Menjalankan Aplikasi

Aplikasi mendukung berbagai parameter dinamis yang dapat disesuaikan saat dijalankan.

### Argumen Command-Line (CLI Options)
| Parameter | Deskripsi | Nilai Default |
| :--- | :--- | :--- |
| `--config <path>` | Lokasi file konfigurasi model YOLO | `config/pgie_yolov8n.txt` |
| `--input <zed\|file>` | Mode sumber input video | `zed` |
| `--input-file <path>` | Lokasi file video (jika input adalah `file`) | *(Wajib diisi jika input=file)* |
| `--output <rtsp\|monitor\|file>` | Mode output stream hasil deteksi | `rtsp` |
| `--output-file <path>` | Nama file penyimpanan output (jika output adalah `file`) | `output.mp4` |
| `--benchmark [path]` | Mengaktifkan logging performa ke file CSV | *Nonaktif* (Default file: `benchmark_result.txt`) |
| `--help`, `-h` | Tampilkan menu bantuan instruksi penggunaan | — |

---

### Contoh Perintah Menjalankan Aplikasi

#### 1. Input ZED Kamera ➜ Output Streaming RTSP (Default)
```bash
./DeepStreamZedyoloRTSP --input zed --output rtsp
```

#### 2. Input File Video ➜ Output Simpan MP4 Lokal (Aman & Rapih)
```bash
./DeepStreamZedyoloRTSP --input file --input-file data/input/video_testing.mp4 --output file --output-file data/output/hasil_deteksi.mp4
```

#### 3. Input Kamera ZED ➜ Tampilkan Langsung ke Monitor (HDMI/DP lokal)
```bash
./DeepStreamZedyoloRTSP --input zed --output monitor
```

#### 4. Menjalankan Uji Performa (Benchmark) ke File CSV Kustom
```bash
./DeepStreamZedyoloRTSP --input zed --output rtsp --benchmark data/benchmark/log_performa.csv
```

---

## 📺 Cara Menonton Stream RTSP (Client Side)

Untuk menonton hasil deteksi objek dari komputer klien yang berada dalam jaringan lokal (WiFi atau LAN) yang sama dengan **latensi serendah mungkin**, gunakan utilitas **FFplay** atau **VLC**.

Buka terminal di komputer klien Anda dan jalankan perintah berikut:

```bash
ffplay -fflags nobuffer -flags low_delay -framedrop -rtsp_transport tcp rtsp://<ALAMAT_IP_JETSON>:8554/ds-test
```

*Ganti `<ALAMAT_IP_JETSON>` dengan alamat IP internal perangkat Jetson Orin Nano Anda (contoh: `192.168.1.15`).*

---

## 🛑 Penghentian Program Secara Aman

- Cukup tekan `Ctrl+C` **satu kali** pada terminal tempat aplikasi berjalan.
- **Pada mode output file (`--output file`)**: Program akan otomatis mematikan asupan input, mengirim event EOS ke encoder dan muxer untuk merapikan metadata kontainer MP4, lalu keluar dengan aman setelah file selesai ditulis.
- Jangan khawatir file video Anda rusak atau corrupt; mekanisme safety timeout 10 detik akan memastikan file tersimpan dengan sempurna di disk.

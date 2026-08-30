# Daftar Library yang Digunakan

Daftar ini disusun berdasarkan audit terhadap `#include`, `import`, konfigurasi
`CMakeLists.txt`, elemen pipeline yang dibuat oleh aplikasi, serta uji langsung
pada environment project. Library yang hanya terdapat pada contoh, dokumentasi,
atau terpasang di sistem tetapi tidak digunakan oleh source code tidak dianggap
sebagai library project.

### Library aplikasi C++ dan pipeline deployment

a. **NVIDIA DeepStream SDK v7.1.0**, digunakan sebagai framework pipeline deteksi
   objek real-time. Komponen yang digunakan mencakup `nvinfer` untuk inferensi,
   `nvstreammux` untuk penggabungan stream, `nvtracker` untuk pelacakan objek,
   `nvdsosd` untuk menggambar hasil deteksi, serta API metadata NVIDIA melalui
   `nvdsmeta`, `gstnvdsmeta`, dan `nvdsinfer_custom_impl`.

b. **GStreamer v1.20.3**, digunakan untuk membangun dan menjalankan pipeline
   multimedia, mengelola input video, decoding, konversi format, encoding, dan
   keluaran video. Aplikasi menggunakan elemen seperti `uridecodebin`,
   `videoconvert`, `nvvideoconvert`, `capsfilter`, `h264parse`, `qtmux`, dan
   `filesink`.

c. **GStreamer RTSP Server v1.20.1**, digunakan untuk menyediakan layanan
   streaming hasil deteksi melalui protokol RTSP.

d. **GLib v2.72.4**, digunakan untuk event loop, penanganan sinyal sistem,
   fungsi utilitas pipeline, serta antrean `GAsyncQueue` pada logging benchmark
   dan dump deteksi.

e. **NVIDIA TensorRT v10.3.0**, digunakan sebagai runtime inferensi untuk
   menjalankan engine YOLO pada GPU Jetson. TensorRT juga digunakan oleh utilitas
   Python untuk membaca, membangun, memeriksa, dan melakukan deserialisasi engine,
   termasuk engine yang menggunakan plugin `EfficientNMS_TRT`. Paket sistem yang
   terdeteksi adalah `libnvinfer10` v10.3.0.30-1+cuda12.5.

f. **CUDA Toolkit v12.6.68**, digunakan sebagai dependensi komputasi GPU dan
   dependensi build parser custom EfficientNMS. Pada konfigurasi CMake, CUDA
   Runtime API dicari melalui berkas `cuda_runtime_api.h`. Paket toolkit yang
   terdeteksi adalah v12.6.11-1.

g. **Stereolabs ZED SDK v5.4.0**, digunakan sebagai dependensi sumber video
   kamera ZED yang diakses melalui elemen GStreamer `zedsrc`. Versi ini
   teridentifikasi dari berkas `/usr/local/zed/zed-config-version.cmake`, dan
   library `libsl_zed.so` serta `libsl_ai.so` terdeteksi melalui `ldconfig`.

h. **GStreamer ZED plugin (`gst-zed`)**, digunakan untuk menghubungkan kamera ZED
   ke pipeline GStreamer melalui elemen `zedsrc`. Pemeriksaan `gst-inspect-1.0
   zedsrc` pada environment ini mengalami crash, sehingga versi plugin belum
   dapat diverifikasi secara andal.

### Library pada utilitas Python

i. **NumPy v1.26.4**, digunakan pada `extra_plots.py` untuk operasi numerik pada
   visualisasi dan pada `build_efficientnms_engine.py` untuk membaca serta
   memproses tensor model.

j. **Pandas v1.3.5**, digunakan oleh utilitas analisis benchmark untuk membaca,
   menggabungkan, dan menganalisis data CSV yang berisi FPS, latensi, penggunaan
   hardware, dan konsumsi daya.

k. **Matplotlib v3.5.1**, digunakan untuk menghasilkan grafik distribusi FPS,
   perbandingan latensi, estimasi energi per frame, serta analisis trade-off.

l. **SciPy v1.8.0**, digunakan secara opsional oleh `tradeoff_analysis.py` untuk
   menjalankan Welch's t-test pada pengujian signifikansi perbedaan performa
   antar-skenario. Saat diuji, SciPy mengeluarkan peringatan kompatibilitas
   dengan NumPy 1.26.4 karena instalasi SciPy tersebut mengharapkan NumPy dengan
   versi di bawah 1.25.0.

m. **pycocotools**, merupakan dependensi opsional pada `eval_deepstream_map.py`
   untuk evaluasi mAP menggunakan `COCO` dan `COCOeval`. Library ini tidak
   terpasang pada environment yang diuji, sehingga fitur evaluasi COCO belum
   dapat dijalankan pada environment tersebut.

n. **Python TensorRT binding (`tensorrt`) v10.3.0**, digunakan oleh
   `build_efficientnms_engine.py` dan `cek_nms.py` untuk menginisialisasi TensorRT,
   membangun atau memeriksa engine, mengaktifkan plugin bawaan, dan melakukan
   deserialisasi engine. Binding ini merupakan bagian dari instalasi TensorRT.

### Dependensi standar

o. **C++ Standard Library dengan standar C++17**, digunakan pada aplikasi utama,
   parser log, dan parser custom melalui `std::vector`, `std::string`,
   `std::filesystem`, `std::thread`, `std::mutex`, `std::regex`, serta fasilitas
   pemrosesan berkas dan waktu.

p. **Python Standard Library**, digunakan melalui modul seperti `argparse`,
   `pathlib`, `csv`, `json`, `re`, `dataclasses`, dan `typing` untuk pemrosesan
   argumen, berkas, data konfigurasi, dan anotasi tipe.

### Library yang tidak terverifikasi sebagai library project

Berdasarkan pencarian terhadap source code (`src/`, `utils/`, `scripts/`, dan
`CMakeLists.txt`) serta uji import Python:

- **Roboflow** tidak digunakan dan tidak terpasang. Kemunculan URL Roboflow pada
  berkas teks skripsi merupakan rujukan dataset, bukan penggunaan SDK Roboflow.
- **Ultralytics** tidak diimpor dan tidak terpasang pada environment ini. Beberapa
  dokumentasi project menyebut proses training atau validasi Ultralytics, tetapi
  notebook atau source training tersebut tidak terdapat pada repository ini.
  Dengan demikian, Ultralytics bukan library runtime pada source project yang
  diaudit.
- **PyTorch dan Torchvision** tidak diimpor dan tidak terpasang pada environment
  ini. File model atau penyebutan format `.pt` tidak cukup untuk menyatakan bahwa
  library tersebut digunakan oleh source project ini.
- **OpenCV (`cv2`)** terpasang pada environment dengan versi 4.8.0, tetapi tidak
  terdapat `import cv2` atau pemanggilan API OpenCV pada source project. OpenCV
  tidak digunakan oleh pipeline ini.
- **Pillow (`PIL`)** terpasang dengan versi 9.0.1, tetapi tidak terdapat `import
  PIL` atau `from PIL` pada source project. Pillow tidak digunakan oleh pipeline
  atau utilitas yang diaudit.

### Catatan verifikasi versi

Versi Python package diperoleh melalui `importlib.metadata` dan uji import aktual.
Versi GStreamer dan GLib diperoleh melalui `pkg-config` serta pemeriksaan paket
sistem. Versi DeepStream diperoleh melalui `deepstream-app --version`, versi CUDA
melalui `nvcc --version`, dan versi TensorRT melalui binding Python serta paket
`libnvinfer10`.

Uji build dengan `cmake --build build -j2` berhasil untuk target `app`, `parser`,
dan `efficientnms_parser`. Eksekusi `./build/app --help` juga berhasil. Namun,
versi library pada daftar ini merepresentasikan environment pemeriksaan saat ini;
metadata environment perangkat pengujian perlu dicatat terpisah apabila berbeda.

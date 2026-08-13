# TensorRT EfficientNMS

`build_efficientnms_engine.py` menambahkan plugin NVIDIA
**`EfficientNMS_TRT`** ke network TensorRT. Algoritme NMS dieksekusi oleh kernel
GPU; tidak ada loop NMS Python/CPU pada engine hasil. Plugin ini adalah tail yang
dependen pada output detector, sehingga tidak berjalan bersamaan dengan backbone
untuk frame yang sama.

Script ini **tidak mengubah ONNX atau engine baseline**. Jika `-o` tidak
diberikan, output dibuat sebagai:

```text
models/yolov8n_kitti.onnx
models/yolov8n_kitti_efficientnms.engine  # engine baru
```

## Prasyarat

Jalankan di NVIDIA Jetson/host dengan:

- TensorRT Python (`import tensorrt` berhasil),
- `libnvinfer_plugin` yang menyediakan `EfficientNMS_TRT`,
- NumPy.

TensorRT harus dapat mem-build engine untuk GPU target. Engine TensorRT
sebaiknya dibangun ulang pada perangkat target Jetson karena engine umumnya
tidak portable antar GPU/versi TensorRT.

## Cek model tanpa menulis file

```bash
python3 utils/trt_efficientnms/build_efficientnms_engine.py \
    models/yolov8n_kitti.onnx \
    --num-classes 3 \
    --inspect
```

Untuk model KITTI di repository ini, script menemukan tensor internal
`[1, 8400, 7]` sebelum output baseline `[1, 8400, 6]`. Tensor tersebut berisi
`[x1, y1, x2, y2, score_class_0, score_class_1, score_class_2]` dan dipasang
langsung ke EfficientNMS.

## Build engine baru

```bash
python3 utils/trt_efficientnms/build_efficientnms_engine.py \
    models/yolov8n_kitti.onnx \
    --num-classes 3 \
    --input-format full_scores \
    --score-threshold 0.05 \
    --iou-threshold 0.45 \
    --max-output-boxes 300 \
    --fp16
```

Hasil default:

```text
models/yolov8n_kitti_efficientnms.engine
```

Contoh model COCO memakai 80 kelas:

```bash
python3 utils/trt_efficientnms/build_efficientnms_engine.py \
    models/yolov8n_coco.onnx \
    --num-classes 80 \
    --output models/yolov8n_coco_efficientnms.engine
```

Script menolak mengganti output yang sudah ada kecuali `--force` diberikan.
Jangan arahkan `--output` ke engine baseline jika ingin menyimpannya untuk
perbandingan.

## Format output engine

Engine hasil hanya mengekspos empat output standar plugin:

| Nama | Bentuk umum | Keterangan |
|---|---|---|
| `num_detections` | `[B, 1]` | jumlah detection valid per image |
| `detection_boxes` | `[B, max_output_boxes, 4]` | koordinat `xyxy` |
| `detection_scores` | `[B, max_output_boxes]` | confidence |
| `detection_classes` | `[B, max_output_boxes]` | indeks kelas |

Input yang dipakai script adalah `NCHW`, resolusi dari ONNX, dan batch statis
(default `1`). Gunakan `--batch`, `--height`, atau `--width` bila diperlukan.

Selain head YOLO `[B,N,4+C]`, tersedia:

- `--input-format full_scores`: jalur graph paling ringkas dan default pilihan
  `auto`, tetapi setiap kandidat dapat dipertimbangkan pada lebih dari satu kelas.
- `--input-format decoded6`: memakai output `[B,N,6]` berupa
  `[x1,y1,x2,y2,score,class_id]`. TensorRT membentuk matriks score dengan satu
  compare + select berbasis broadcasting. Mode ini paling dekat dengan baseline
  parser yang memilih `max-score/argmax`, tetapi menambah operasi graph dan tidak
  otomatis lebih cepat.
- `--input-format boxes_scores --boxes-name ... --scores-name ...` untuk dua
  tensor `[B,N,4]` dan `[B,N,C]`.

Gunakan mode yang sama ketika membandingkan akurasi. `full_scores` vs `decoded6`
bukan hanya pilihan performa; keduanya dapat menghasilkan jumlah/kelas deteksi
yang berbeda.

## Primary GIE DeepStream

Parser Primary GIE khusus tersedia di `src/efficientnms_parser.cpp`. Build
library dan aplikasi dengan:

```bash
cmake -S . -B build
cmake --build build -j2
```

Target library dibuat sebagai:

```text
lib/libnvdsinfer_custom_impl_EfficientNMS.so
```

Setelah engine dibuat, konfigurasi Primary GIE yang siap dipakai adalah:

```text
config/pgie_yolov8n_kitti_efficientnms.txt
```

Jalankan aplikasi C++ dengan konfigurasi tersebut:

```bash
./app --config config/pgie_yolov8n_kitti_efficientnms.txt
```

Untuk `deepstream-app` standar, tersedia konfigurasi pipeline alternatif:

```bash
deepstream-app -c config/deepstream_app_efficientnms.txt
```

Parser membaca empat output EfficientNMS dan tidak melakukan NMS kedua di CPU.
`cluster-mode=4` dipakai agar DeepStream tidak menjalankan clustering/NMS lagi.
Koordinat box dari builder ini adalah `xyxy` absolut pada koordinat input model.
Config EfficientNMS sengaja hanya menunjuk file engine: ONNX sumber tidak memiliki
empat output plugin dan tidak dapat membangun ulang engine yang ekuivalen.

## Baseline DeepStream

Konfigurasi dan engine baseline yang sudah ada tidak dihapus atau ditimpa.
Engine EfficientNMS mengubah kontrak output menjadi empat tensor plugin, jadi
parser YOLO baseline yang mengharapkan output `[B,N,6]` tidak boleh dipakai
untuk engine baru. Gunakan engine/config baseline untuk jalur lama, dan engine
serta config bersufiks `_efficientnms` sebagai jalur eksperimen/perbandingan
yang terpisah.

## Batas optimasi dan alternatif

Pada TensorRT 10.3, `EfficientNMS_TRT` sudah sangat murah untuk YOLOv8n batch 1
(dalam profiling proyek ini sekitar `0.05 ms` untuk plugin; slice/reformat menambah
biaya tail). `max_output_boxes` mengecilkan output, tetapi implementasi plugin
masih dapat menyeleksi hingga 4096 score secara internal. Karena itu, menurunkan
`max_output_boxes` saja tidak menghapus biaya utama selection/sort.

Urutan optimasi yang disarankan:

1. Naikkan `score-threshold` atau turunkan kandidat hanya jika evaluasi mAP/recall
   mengizinkan; terapkan threshold identik pada baseline.
2. Uji `max-output-boxes` 100/200 bila jumlah objek aktual tidak pernah mendekati
   batas tersebut. Ini terutama mengurangi D2H/parser/tracker, bukan selection
   internal plugin.
3. Profilkan dengan `trtexec --dumpProfile` dan Nsight Systems. Jangan menyimpulkan
   speedup dari satu run pipeline.
4. Bila model boleh diganti, head end-to-end/NMS-free (misalnya model YOLO yang
   memang dilatih NMS-free) menghilangkan tail NMS dan biasanya lebih menjanjikan
   daripada menulis ulang kernel.
5. Custom plugin baru baru layak bila mem-fuse **decode + sigmoid/argmax + threshold
   + pre-TopK kecil + bitmask NMS + compact output** dalam satu `IPluginV3`, khusus
   untuk batch/resolusi/jumlah kelas target. Menulis kernel NMS generik saja kecil
   kemungkinannya mengalahkan plugin NVIDIA secara end-to-end.

Untuk benchmark yang adil, build kedua engine pada device, TensorRT, precision,
workspace, dan source graph yang sama; gunakan `cluster-mode=2` pada baseline dan
`cluster-mode=4` hanya pada engine yang sudah mengandung NMS. Validasi pula mAP
atau setidaknya tuple `(class, confidence, box)` per frame sebelum membandingkan
FPS, karena jumlah deteksi mengubah beban tracker/OSD.

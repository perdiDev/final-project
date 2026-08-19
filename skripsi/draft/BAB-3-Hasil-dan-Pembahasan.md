# BAB III — HASIL DAN PEMBAHASAN

> Status: draf hasil **restrukturisasi** (2026-08-19) dari `BAB-4-Hasil-dan-Pembahasan.md` versi
> lama (sudah dihapus) mengikuti format skripsi Departemen Teknik Informatika Unhas — BAB III
> kini berisi Hasil dan Pembahasan (bergeser dari BAB IV lama), sejalan dengan pergeseran BAB II
> lama (Metodologi) menjadi BAB II baru (Metode Penelitian, lihat `BAB-2-Metode-Penelitian.md`).
> Isinya berbasis data eksekusi penuh **60 *run*** (6 model × 2 *tracker* × 5 repetisi) pada
> Jetson Orin Nano, dijalankan `scripts/run_all_benchmark.sh` tanggal 2026-08-19. Seluruh angka
> di bab ini bersumber dari `../eksperimen/runtime_summary.csv`, `../eksperimen/runtime_per_run.csv`,
> `../eksperimen/tradeoff_summary.csv`, grafik di `../eksperimen/plots/`, serta
> `../../docs/05_accuracy_results.md` (akurasi FP32). Bagian yang datanya belum tersedia
> (verifikasi akurasi *as-deployed* FP16) ditandai `TODO` secara eksplisit — lihat status di
> `BAB-2-Metode-Penelitian.md` §2.6.1 poin 4. Riwayat perubahan lengkap ada di
> `../log/log-perubahan.md`.

## Kondisi Eksekusi dan Alur Agregasi Data

*Pipeline* DeepStream (`src/main.cpp`) berhasil dikompilasi (`scripts/build.sh`) dan dijalankan
penuh di Jetson Orin Nano untuk keenam konfigurasi model (`BAB-2-Metode-Penelitian.md` §2.2.4)
dikombinasikan dengan kedua konfigurasi *tracker* (§2.2.5). Orkestrasi otomatis
`scripts/run_all_benchmark.sh` menjalankan seluruh **12 skenario (6 model × 2 *tracker*) × 5
repetisi = 60 *run*** secara berurutan tanpa intervensi manual di antara *run*, sesuai tahap 6
metodologi (§2.3). Seluruh 60 *run* berhasil menghasilkan ketiga artefak yang diharapkan
(`fps.csv`, `hardware_analysis.csv`, `run_info.txt`) di
`data/benchmark/<model>_<tracker>/<timestamp>/` — tidak ada *run* yang gagal atau perlu diulang.

| Parameter | Nilai (dari `run_info.txt`) |
|---|---|
| Commit kode | `5d04bed` (konsisten di seluruh 60 *run*) |
| Video input | `data/input/video_testing.mp4` (identik di seluruh *run*, sesuai variabel terkontrol §4.1 `04_benchmark_protocol.md`) |
| Mode output | `file` (encode MP4) |
| Mode daya (`nvpmodel`) | `10W` (indeks mode 0) — satu-satunya mode performa tinggi yang tersedia pada varian Jetson Orin Nano 4GB (`BAB-2-Metode-Penelitian.md` §2.2.1); diaktifkan sekali di awal *batch* (`sudo nvpmodel -m 0`) sebelum ke-60 *run* |
| `jetson_clocks` | Dikunci di awal *batch* (`sudo jetson_clocks`, dengan `set -e` — seluruh *script* akan berhenti di *run* pertama bila langkah ini gagal, sehingga status kunci pada 60 *run* berikutnya konsisten) |
| Pengukuran latensi | `NVDS_ENABLE_LATENCY_MEASUREMENT=1` aktif otomatis di seluruh *run* |
| Interval `tegrastats` | 1000 ms |
| Rentang waktu eksekusi *batch* | 2026-08-19 11:33–12:46 WITA (± 72 menit, termasuk *cooldown* 60 detik antar-*run*) |

**Catatan metodologis**: kolom `jetson_clocks_status` di dalam masing-masing `run_info.txt`
mencatat galat *"Run this script as a root user"*. Ini **bukan** indikasi bahwa *clock* tidak
terkunci — galat ini berasal dari pemeriksaan status (`jetson_clocks --show`, dipanggil tanpa
`sudo` di dalam `scripts/run_benchmark.sh:314`) yang berjalan sebagai proses anak tanpa privilese
root, terpisah dari langkah pengunci *clock* yang sesungguhnya (`sudo jetson_clocks`, dijalankan
sekali di awal `run_all_benchmark.sh` dengan privilese root). Karena *script* memakai `set -e`,
kegagalan pada langkah pengunci itu sendiri akan menghentikan seluruh *batch* di *run* pertama —
sehingga 60 *run* yang berhasil selesai secara konsisten mengindikasikan langkah penguncian
berhasil. Kelemahan pencatatan status ini (bukan kelemahan pengujian) dicantumkan sebagai catatan
transparansi, bukan disembunyikan.

Durasi setiap *run* individual relatif singkat (± 13–20 detik *wall-clock*, mengikuti panjang
`video_testing.mp4`) karena berkas input diputar apa adanya tanpa batas durasi eksplisit
(`--duration` tidak digunakan oleh `run_all_benchmark.sh`). Agregasi membuang 10 detik pertama
setiap *run* sebagai *warm-up* (mengikuti rekomendasi `04_benchmark_protocol.md` §4.3), sehingga
jumlah *frame* yang tersisa untuk statistik per skenario berkisar 633–1.399 *frame* (gabungan 5
repetisi) — lebih sedikit dibanding rekomendasi klip 180 detik di protokol. Implikasi
keterbatasan ini dibahas lebih lanjut di §3.5.2.

Data mentah 60 *run* diagregasi dengan `utils/benchmark_analysis/aggregate_runtime.py` menjadi
`../eksperimen/runtime_summary.csv` (ringkasan per skenario) dan
`../eksperimen/runtime_per_run.csv` (data mentah per *run*, dipakai untuk uji signifikansi §3.2
dan §3.3), ditambah grafik distribusi FPS (`../eksperimen/plots/fps_boxplot_by_model.png`,
`fps_boxplot_by_tracker.png`). Data ini kemudian digabung dengan akurasi KITTI
(`../../docs/05_accuracy_results.md`) oleh `utils/benchmark_analysis/tradeoff_analysis.py`
menjadi `../eksperimen/tradeoff_summary.csv` dan grafik Pareto (`tradeoff_map_vs_fps.png`,
`tradeoff_map_vs_power.png`), dipakai di §3.5.

## 3.1 Hasil Pengujian Kinerja Baseline Pipeline (Menjawab RM1)

### 3.1.1 Analisis Throughput (FPS)

Skenario *baseline* memakai *tracker default* NvDCF, sesuai `BAB-2-Metode-Penelitian.md` §2.6.1
poin 1.

| Model | Avg FPS | Median FPS | Std FPS | Latensi median (ms) | Latensi p95 (ms) | Latensi p99 (ms) |
|---|---|---|---|---|---|---|
| YOLOv8n | 66,77 | 66,52 | 0,51 | 262,57 | 334,01 | 374,99 |
| YOLOv9t | **51,52** | **51,73** | 0,61 | **454,38** | **539,03** | **547,15** |
| YOLOv10n | 67,02 | 67,22 | 0,40 | 238,13 | 298,83 | 308,91 |
| YOLO26n | 65,66 | 65,45 | 1,02 | 350,83 | 412,25 | 424,54 |

**Seluruh model *baseline* jauh melampaui ambang *real-time* 30 FPS**
(`BAB-2-Metode-Penelitian.md` §2.6.2) — bahkan YOLOv9t yang paling lambat sekalipun mencapai
rata-rata 51,52 FPS (± 72% di atas ambang). Pemenuhan ambang ini juga diverifikasi tidak hanya
pada rata-rata, tetapi pada **setiap** dari 60 *run* individual (bukan hanya rata-rata skenario,
agar *run* tunggal yang kebetulan lambat tidak tersembunyi di balik rata-rata):

| Model | Tracker | FPS minimum (dari 5 repetisi) | FPS maksimum | Status vs. ambang 30 FPS |
|---|---|---|---|---|
| YOLOv8n | NvDCF | 66,43 | 67,66 | Lulus (+121%) |
| YOLOv8n | NvSORT | 66,49 | 67,41 | Lulus (+122%) |
| YOLOv9t | NvDCF | 50,88 | 52,29 | Lulus (+70%) |
| YOLOv9t | NvSORT | 66,90 | 67,23 | Lulus (+123%) |
| YOLOv10n | NvDCF | 66,55 | 67,40 | Lulus (+122%) |
| YOLOv10n | NvSORT | 67,31 | 67,37 | Lulus (+124%) |
| YOLO26n | NvDCF | 64,27 | 67,06 | Lulus (+114%) |
| YOLO26n | NvSORT | 67,08 | 67,34 | Lulus (+124%) |
| YOLOv8n+EfficientNMS | NvDCF | 66,42 | 66,82 | Lulus (+121%) |
| YOLOv8n+EfficientNMS | NvSORT | 66,94 | 67,43 | Lulus (+123%) |
| YOLOv9t+EfficientNMS | NvDCF | **49,89** | 51,18 | Lulus (+66%) |
| YOLOv9t+EfficientNMS | NvSORT | 66,75 | 67,73 | Lulus (+123%) |

**Seluruh 60 *run*, tanpa kecuali, memenuhi kriteria *real-time* ≥ 30 FPS** — bahkan skenario
paling lambat (YOLOv9t+EfficientNMS dengan NvDCF, FPS minimum 49,89) masih melampaui ambang
sebesar 66%. Ini menjawab bagian inti RM1: keempat model *baseline* memenuhi kriteria *real-time*
pada Jetson Orin Nano dengan margin yang besar, sehingga margin tersebut dapat "dikonversi" untuk
mengejar akurasi lebih tinggi (§3.5.1) tanpa mengorbankan kepatuhan *real-time*. Rincian FPS pada
varian EfficientNMS dan varian *tracker* NvSORT dibahas lebih lanjut masing-masing di §3.2 dan
§3.3.

**Catatan lingkup pengujian**: hasil di atas berasal dari protokol *file*-based terkontrol
(`04_benchmark_protocol.md` §4.1) menggunakan `video_testing.mp4` yang identik di seluruh *run* —
bukan dari kamera ZED *live* secara langsung. Keputusan ini disengaja (sumber video tetap agar
setiap model "melihat" input yang identik, sehingga hasil antar-model benar-benar sebanding).
Validasi tambahan pada aliran kamera ZED *live* belum dilakukan pada laporan ini dan dicatat
sebagai pekerjaan lanjutan (`../../docs/08_limitations_future_work.md`).

### 3.1.2 Analisis Latensi End-to-End

Distribusi latensi *end-to-end* pada Tabel §3.1.1 sengaja dilaporkan sebagai persentil **p95/p99**
selain median, mengikuti kriteria evaluasi RM1 (`BAB-2-Metode-Penelitian.md` §2.6.2), agar
*outlier*/*jitter* — relevan untuk aplikasi *safety-critical* seperti ADAS — tidak tersembunyi di
balik rata-rata. **YOLOv9t adalah *outlier* yang konsisten dengan temuan akurasi T4**
(`../../docs/05_accuracy_results.md` §5.4 poin 2, yang mencatat YOLOv9t sebagai model dengan
*inference time* T4 paling lambat walau parameternya paling kecil). Di Jetson, pola ini terulang
dan makin nyata pada sisi latensi: median 454,38 ms dan p99 547,15 ms — jauh di atas ketiga model
lain (median 238–351 ms, p99 309–425 ms). Selisih antara median dan p99 pada YOLOv9t (~93 ms)
juga lebih besar dibanding model lain (~46–74 ms), mengindikasikan sebaran *jitter* yang lebih
lebar pada model ini — sebuah pertimbangan penting jika YOLOv9t hendak dipakai pada sistem
*safety-critical* yang menuntut latensi terprediksi, bukan sekadar rata-rata rendah.

### 3.1.3 Analisis Latensi Per-Komponen

Rincian latensi per-komponen *pipeline* (rata-rata ms, dari `fps.csv`) mengurai kontribusi tiap
elemen terhadap total latensi pada Tabel §3.1.1–§3.1.2:

| Model | PreMux | Mux | Infer | Tracker | PreOSD | OSD | Output |
|---|---|---|---|---|---|---|---|
| YOLOv8n | 213,84 | 14,09 | 19,21 | 10,98 | ~0,01 | 9,34 | 5,87 |
| YOLOv9t | **360,92** | 27,48 | 28,65 | **18,93** | ~0,01 | 13,76 | 7,93 |
| YOLOv10n | 205,36 | 10,11 | 16,72 | 3,53 | ~0,01 | 7,08 | 4,67 |
| YOLO26n | 279,70 | 21,87 | 22,88 | 13,63 | ~0,01 | 10,95 | 6,52 |

Dua temuan utama menjelaskan *bottleneck* per-komponen:

1. **`Lat_PreMux_ms` YOLOv9t (360,92 ms) jauh di atas model lain (~205–280 ms).**
   `Lat_PreMux_ms` mengukur waktu tunggu *buffer* di *decoder*/*queue* sebelum masuk
   `nvstreammux` (`../../docs/04_benchmark_protocol.md`); nilainya yang membengkak untuk YOLOv9t
   kemungkinan besar adalah gejala *backpressure* dari tahap hilir yang lebih lambat (arsitektur
   PGI/GELAN YOLOv9 yang lebih *sequential*, ditambah biaya NvDCF di tahap *tracking*) yang
   merambat balik ke antrean di depan *muxer* — bukan *decoder* itu sendiri yang melambat. Dugaan
   ini konsisten dengan pola di §3.3.1 (nilai `Lat_PreMux_ms` YOLOv9t turun ke ~206 ms, sejajar
   model lain, begitu *tracker* diganti ke NvSORT).
2. **YOLO26n memiliki GFLOPs teoretis terendah (5,2) tetapi bukan model tercepat di Jetson**
   (65,66 FPS, di bawah YOLOv8n dan YOLOv10n) — mengonfirmasi catatan
   `../../docs/05_accuracy_results.md` bahwa GFLOPs teoretis tidak selalu berbanding lurus dengan
   performa aktual pada perangkat *edge*; kepadatan komputasi memori/latensi *tracker* NvDCF
   (13,63 ms untuk YOLO26n, kedua tertinggi setelah YOLOv9t) turut menyumbang. Diskusi lanjutan
   tentang karakter NMS-*free* YOLO26n ada di §3.2.3.

## 3.2 Hasil Pengujian Optimasi NMS Paralel (Menjawab RM2)

Perbandingan pasangan model dengan bobot identik, *tracker* NvDCF (kondisi lain sama dengan
§3.1), sesuai `BAB-2-Metode-Penelitian.md` §2.6.1 poin 2. Uji signifikansi memakai Welch's
*t*-test atas distribusi FPS 5 repetisi per skenario (`tradeoff_analysis.py --significance`).

### 3.2.1 Dampak EfficientNMS terhadap Latensi Inferensi

| Pasangan | `Lat_Infer_ms` *baseline* | `Lat_Infer_ms` EfficientNMS | Δ |
|---|---|---|---|
| YOLOv8n | 19,21 | 18,97 | −0,24 ms |
| YOLOv9t | 28,65 | 28,09 | −0,56 ms |

Pada kedua model, `Lat_Infer_ms` (tahap yang mencakup *post-processing* NMS) sedikit membaik
setelah EfficientNMS dipasang, namun selisihnya sangat kecil secara absolut. Ini konsisten dengan
catatan `utils/trt_efficientnms/README.md` §"Batas optimasi dan alternatif" bahwa biaya
`EfficientNMS_TRT` itu sendiri sudah sangat murah (~0,05 ms) pada TensorRT 10.3 untuk YOLOv8n
*batch* 1 — implementasi NMS standar `nvinfer` yang dipakai *baseline* bukanlah *bottleneck*
besar untuk mulai dengan, sehingga ada sedikit "ruang" yang bisa dioptimasi pada tahap inferensi
itu sendiri.

### 3.2.2 Dampak EfficientNMS terhadap Throughput Keseluruhan

| Pasangan | FPS *baseline* | FPS EfficientNMS | Δ FPS | *p*-value | Kesimpulan |
|---|---|---|---|---|---|
| YOLOv8n vs. +EfficientNMS | 66,77 | 66,63 | −0,14 (−0,2%) | 0,592 | Tidak signifikan |
| YOLOv9t vs. +EfficientNMS | 51,52 | 50,44 | **−1,08 (−2,1%)** | **0,023** | **Signifikan — EfficientNMS lebih lambat** |

Latensi total rata-rata (ms): YOLOv8n 273,35 ms (*baseline*) vs. 262,34 ms (EfficientNMS);
YOLOv9t 457,67 ms (*baseline*) vs. 468,26 ms (EfficientNMS).

Pada YOLOv8n, tidak ada perbedaan FPS yang signifikan secara statistik. Pada YOLOv9t, sebaliknya,
perbedaan FPS justru **signifikan namun berlawanan arah dari yang diharapkan** — EfficientNMS
lebih lambat, bukan lebih cepat. Karena `Lat_Infer_ms` itu sendiri hampir tidak berubah (§3.2.1),
penurunan FPS pada YOLOv9t bukan berasal dari biaya *plugin* NMS itu sendiri, melainkan dari
interaksi dengan tahap lain. Hasil ini adalah **temuan negatif yang sah secara ilmiah**, bukan
kegagalan implementasi, dan dapat dijelaskan oleh tiga faktor — seluruhnya berdasar pada
dokumentasi teknis proyek ini (`utils/trt_efficientnms/README.md`):

1. **Biaya `EfficientNMS_TRT` itu sendiri sudah sangat kecil** (§3.2.1), sehingga ruang optimasi
   dari sisi *plugin* NMS memang terbatas sejak awal.
2. **`EfficientNMS_TRT` adalah *tail* yang dependen pada *output* detektor** — ia tidak berjalan
   bersamaan (*concurrent*) dengan komputasi *backbone* untuk *frame* yang sama. Karakteristik
   *sequential*/tidak-*overlapping* ini kemungkinan mengurangi kesempatan *pipelining* GPU antar
   *frame*, yang konsisten dengan turunnya rata-rata pemakaian GPU pada YOLOv9t+EfficientNMS
   (69,3%, turun dari 87,7% pada *baseline*, lihat §3.3.3) meski FPS-nya juga turun — indikasi
   *bubble*/*idle* GPU yang lebih besar, bukan GPU yang lebih sibuk.
3. **Perbaikan `Lat_Infer_ms` semata tidak otomatis menaikkan FPS** bila tahap lain
   (`Lat_Tracker_ms`, `Lat_PreMux_ms`) menjadi *bottleneck* yang lebih dominan — persis kondisi
   YOLOv9t (§3.1.3), di mana `Lat_Infer_ms` nyaris tidak berubah tetapi FPS turun karena
   interaksi dengan *tail latency* di tahap lain.

Sesuai rekomendasi `utils/trt_efficientnms/README.md` §"Batas optimasi dan alternatif", peluang
optimasi lanjutan yang lebih menjanjikan untuk kasus ini bukan pada *plugin* NMS itu sendiri,
melainkan pada `score-threshold`/`max-output-boxes` yang lebih agresif (bila mAP masih dalam
toleransi) atau model dengan *head* NMS-*free* — dibahas pada §3.2.3.

### 3.2.3 Pembahasan Model NMS-free

YOLOv10n dan YOLO26n (`BAB-2-Metode-Penelitian.md` §2.2.4) tidak disertakan pada perbandingan RM2
di atas karena keduanya sudah *NMS-free* secara arsitektural — tidak ada pasangan *baseline*
vs. EfficientNMS yang setara untuk dibandingkan. Namun demikian, hasil *baseline* di §3.1.1 dan
§3.1.3 tetap relevan untuk menilai apakah pendekatan arsitektural NMS-*free* memberi keuntungan
runtime dibanding model dengan NMS terpisah: YOLOv10n mencatat FPS tertinggi di antara keempat
model *baseline* (67,02) dengan `Lat_Infer_ms` terendah (16,72 ms), sedangkan YOLO26n — meski
memiliki GFLOPs teoretis terendah (5,2) — hanya mencapai 65,66 FPS, di bawah YOLOv8n dan YOLOv10n
(§3.1.3 poin 2). Dengan kata lain, **karakter NMS-*free* tidak secara otomatis menjamin
*throughput* tertinggi** pada level *pipeline* lengkap — faktor lain seperti kepadatan komputasi
*backbone* dan biaya *tracker* NvDCF tetap berkontribusi signifikan terhadap FPS akhir.

Temuan ini melengkapi hasil RM2: bagi model kelas *nano/tiny* pada Jetson Orin Nano, pendekatan
menghilangkan NMS secara arsitektural (seperti pada YOLOv10n/YOLO26n) dan pendekatan
mempercepat NMS lewat *plugin* paralel (EfficientNMS pada YOLOv8n/YOLOv9t) sama-sama **tidak**
memberi jaminan otomatis atas peningkatan *throughput* yang besar — pada kelas model ini,
`Lat_Infer_ms` (termasuk NMS di dalamnya) bukan komponen *bottleneck* dominan dibanding
`Lat_PreMux_ms` dan `Lat_Tracker_ms` (§3.1.3, §3.3.1).

## 3.3 Hasil Pengujian Efisiensi Komputasi Tracking (Menjawab RM3)

Skenario ini adalah inti rumusan masalah #3 (`BAB-2-Metode-Penelitian.md` §2.6.1 poin 3):
membandingkan **efisiensi komputasi** (bukan kualitas *tracking*) NvDCF vs. NvSORT di keenam
model, masing-masing 5 repetisi (total 60 *run*, lihat bagian pembuka bab ini). Statistik
median/p95 `Lat_Tracker_ms` dihitung langsung dari gabungan seluruh *frame* (setelah buang 10
detik *warm-up*) per skenario, bukan rata-rata dari rata-rata, agar persentil merepresentasikan
distribusi *frame* yang sebenarnya.

### 3.3.1 Perbandingan Latensi Tracker (NvDCF vs. NvSORT)

**Tabel 3.3.1** `Lat_Tracker_ms` (dihitung dari seluruh *frame* gabungan 5 repetisi per skenario,
setelah buang *warm-up*)

| Model | Median NvDCF | p95 NvDCF | Median NvSORT | p95 NvSORT | Rasio median (NvDCF/NvSORT) |
|---|---|---|---|---|---|
| YOLOv8n | 9,77 | 22,75 | 0,29 | 0,44 | ~34× |
| YOLOv9t | 21,33 | 31,06 | 0,35 | 3,66 | ~61× |
| YOLOv10n | 2,02 | 10,53 | 0,20 | 0,34 | ~10× |
| YOLO26n | 12,47 | 24,73 | 0,31 | 0,53 | ~40× |
| YOLOv8n+EfficientNMS | 10,89 | 22,15 | 0,29 | 0,48 | ~38× |
| YOLOv9t+EfficientNMS | 22,19 | 31,82 | 0,36 | 2,80 | ~62× |

**Biaya komputasi `Lat_Tracker_ms` NvDCF secara konsisten 10×–62× lebih tinggi daripada NvSORT,
di seluruh enam model** — sejalan dengan karakteristik arsitektural keduanya
(`BAB-2-Metode-Penelitian.md` §2.2.5, §2.5.3): NvDCF melakukan ekstraksi fitur berbasis piksel
penuh per objek, sedangkan NvSORT murni berbasis gerak (*Kalman filter* + algoritma Hungarian)
tanpa pemrosesan piksel. Ini adalah hasil yang **tidak bergantung pada model deteksi** — pola
yang sama muncul baik pada model *nano*-class ringan (YOLOv10n) maupun yang lebih berat
(YOLOv9t).

### 3.3.2 Dampak Algoritma Tracking terhadap FPS

**Tabel 3.3.2** FPS dan signifikansi statistik (Welch's *t*-test, NvDCF vs. NvSORT per model)

| Model | FPS NvDCF | FPS NvSORT | Δ FPS | *p*-value | Signifikan? |
|---|---|---|---|---|---|
| YOLOv8n | 66,77 | 66,86 | +0,09 | 0,753 | Tidak |
| YOLOv9t | **51,52** | **67,08** | **+15,56 (+30%)** | **<0,0001** | **Ya (sangat kuat)** |
| YOLOv10n | 67,02 | 67,34 | +0,32 | 0,146 | Tidak |
| YOLO26n | 65,66 | 67,29 | +1,63 | 0,023 | Ya |
| YOLOv8n+EfficientNMS | 66,63 | 67,23 | +0,60 | 0,003 | Ya |
| YOLOv9t+EfficientNMS | **50,44** | **67,19** | **+16,75 (+33%)** | **<0,0001** | **Ya (sangat kuat)** |

Lihat juga `../eksperimen/plots/fps_boxplot_by_tracker.png` untuk distribusi FPS per *tracker* di
seluruh skenario.

**Dampaknya terhadap FPS keseluruhan bergantung pada model** — inilah temuan paling penting dari
RM3. Untuk YOLOv8n, YOLOv10n, dan (secara praktis) YOLO26n/YOLOv8n+EfficientNMS, selisih FPS
akibat pergantian *tracker* kecil (0,09–1,63 FPS) meski sebagian secara statistik signifikan
(karena variansi antar-*run* yang sangat kecil, bukan karena selisihnya besar secara praktis).
Sebaliknya, untuk **YOLOv9t (baik varian *baseline* maupun EfficientNMS)**, penggantian
NvDCF→NvSORT meningkatkan FPS **+30% dan +33%** — selisih yang sangat besar dan sangat signifikan
(*p* < 0,0001).

Temuan inti RM3 dapat diringkas sebagai **"penghematan komputasi NvSORT bersifat universal pada
level komponen (§3.3.1), tetapi manfaatnya pada *throughput* akhir bersifat kondisional pada
model."** `Lat_Tracker_ms` NvSORT konsisten mendekati nol (median < 0,4 ms) untuk keenam model —
properti arsitektural NvSORT itu sendiri yang tidak bergantung pada model deteksi di hulu. Namun
demikian, penghematan komputasi ini hanya "terlihat" pada *throughput* akhir ketika model deteksi
di hulu **sudah menghabiskan sebagian besar *headroom* waktu-per-*frame* yang tersedia** —
kondisi yang secara empiris hanya terpenuhi oleh YOLOv9t pada eksperimen ini (Infer + PreMux +
Tracker gabungan mendekati/melebihi waktu per-*frame* yang dibutuhkan untuk mencapai 60 FPS,
lihat §3.1.3). Pada YOLOv8n, YOLOv10n, dan YOLO26n, yang memiliki *headroom* lebih besar, biaya
tambahan NvDCF "tersembunyi" di dalam *slack* tersebut dan tidak sampai menjadi *bottleneck*
akhir *pipeline*.

Implikasi praktis untuk *deployment* ADAS: **pemilihan *tracker* tidak dapat dievaluasi secara
independen dari model deteksi yang dipasangkan dengannya.** Rekomendasi generik "gunakan NvSORT
karena lebih efisien" benar secara komponen, tetapi dampaknya terhadap *throughput* keseluruhan
sistem baru signifikan pada kombinasi model yang *pipeline*-nya sudah mendekati batas kapasitas —
sesuatu yang hanya dapat diketahui melalui pengukuran *end-to-end* seperti yang dilakukan pada
bab ini, bukan dari spesifikasi *tracker* semata.

### 3.3.3 Analisis Penggunaan Sumber Daya Perangkat

**Tabel 3.3.3** Efisiensi hardware (rata-rata GPU%, RAM, daya `VDD_IN`, dan estimasi energi per
*frame* = `VDD_IN` ÷ FPS)

| Model | Tracker | GPU % | RAM (MB) | `VDD_IN` (mW) | Energi/*frame* (mJ) |
|---|---|---|---|---|---|
| YOLOv8n | NvDCF | 84,6 | 1368,0 | 9185,8 | 137,58 |
| YOLOv8n | NvSORT | 64,3 | 1296,5 | 8447,3 | **126,34** |
| YOLOv9t | NvDCF | 87,7 | 1365,2 | 8598,3 | **166,89** |
| YOLOv9t | NvSORT | 90,6 | 1300,5 | 8930,2 | 133,12 |
| YOLOv10n | NvDCF | 66,9 | 1449,1 | 8614,0 | 128,53 |
| YOLOv10n | NvSORT | 65,6 | 1388,4 | 8569,2 | **127,26** |
| YOLO26n | NvDCF | 75,6 | 1418,0 | 8827,6 | 134,45 |
| YOLO26n | NvSORT | 68,1 | 1389,5 | 8455,5 | **125,66** |
| YOLOv8n+EfficientNMS | NvDCF | 87,2 | 1371,3 | 9202,0 | 138,11 |
| YOLOv8n+EfficientNMS | NvSORT | 57,4 | 1310,7 | 8443,2 | **125,58** |
| YOLOv9t+EfficientNMS | NvDCF | 69,3 | 1361,7 | 8389,2 | **166,32** |
| YOLOv9t+EfficientNMS | NvSORT | 91,2 | 1318,2 | 8942,3 | 133,10 |

Dua temuan tambahan dari sisi *resource*:

1. **Estimasi energi per *frame* (daya ÷ FPS) menunjukkan NvSORT lebih hemat energi di *seluruh*
   enam model**, walau daya sesaat (`VDD_IN`) NvSORT justru sedikit lebih tinggi daripada NvDCF
   pada kedua varian YOLOv9t (8930,2 vs. 8598,3 mW dan 8942,3 vs. 8389,2 mW). Ini karena NvSORT
   menyelesaikan jauh lebih banyak *frame* per detik pada YOLOv9t (67 vs. 51 FPS), sehingga
   energi yang dikeluarkan *per unit pekerjaan* (per *frame*) tetap lebih rendah (133 mJ vs. 167
   mJ) meski daya sesaatnya lebih tinggi. Metrik daya sesaat sendirian dapat menyesatkan bila
   tidak dinormalisasi terhadap *throughput* — inilah alasan bagian ini melaporkan kedua metrik.
2. **Pola GPU% pada YOLOv9t terbalik dibanding empat model lain** — pada YOLOv8n/YOLOv10n/
   YOLO26n, NvDCF menghasilkan GPU% lebih tinggi daripada NvSORT (selisih 1,3–29,8 poin),
   sedangkan pada YOLOv9t, NvSORT justru menghasilkan GPU% *lebih tinggi* (90,6% vs. 87,7% dan
   91,2% vs. 69,3%). Kemungkinan penjelasannya: pada YOLOv9t+NvSORT, hilangnya *bottleneck*
   *tracker* membuat GPU bekerja mendekati kapasitas penuhnya untuk inferensi (GPU-*bound*),
   sedangkan pada YOLOv9t+NvDCF, sebagian waktu justru dihabiskan menunggu komputasi NvDCF (yang
   sebagian berjalan di CPU) tanpa GPU *idle* sepenuhnya tercatat sebagai penurunan besar —
   sampel `tegrastats` 1 Hz terlalu kasar untuk memastikan mekanisme persisnya. Pola ini
   dilaporkan sebagai temuan yang memerlukan profil lebih dalam (mis. `trtexec --dumpProfile`
   atau Nsight Systems, sesuai rekomendasi `utils/trt_efficientnms/README.md`), bukan disimpulkan
   secara pasti pada bagian ini.

Penggunaan RAM berkisar 1.265–1.449 MB di seluruh 12 konfigurasi — sekitar 31–35% dari kapasitas
total 4GB modul Jetson Orin Nano yang dipakai (`BAB-2-Metode-Penelitian.md` §2.2.1) untuk satu
*stream* video dan satu model aktif. Implikasi keterbatasan memori ini untuk skenario *deployment*
yang lebih kompleks (mis. multi-kamera/multi-model) dibahas di §3.5.2.

**Kualitas/akurasi *tracking* (ID *switch*, MOTA/IDF1) sengaja tidak diukur** pada bagian ini,
sesuai `BAB-1-Pendahuluan.md` §1.5 poin 5 dan justifikasi `BAB-2-Metode-Penelitian.md` §2.5.3 —
perbandingan di atas murni efisiensi komputasi.

## 3.4 Verifikasi Akurasi As-Deployed FP16 (Uji Sanity Check)

### 3.4.1 Evaluasi Nilai mAP50 dan mAP50-95

Sebagai rujukan akurasi, tabel berikut diukur dengan `model.val()` (Ultralytics) di GPU cloud
(Tesla T4, Kaggle) pada *val set* KITTI yang identik untuk keempat arsitektur dasar (1.010
gambar, 4.722 *instance*) — lihat `../../docs/05_accuracy_results.md`. Nilai ini berlaku untuk
pasangan *baseline*/EfficientNMS yang memakai bobot sama (EfficientNMS hanya mengubah eksekusi
NMS, bukan bobot deteksi).

| Model | Params | GFLOPs | mAP50 | mAP50-95 | Precision | Recall |
|---|---|---|---|---|---|---|
| YOLOv8n | 3.006.233 | 8,1 | **0,9767** | **0,8397** | **0,9696** | 0,9344 |
| YOLOv9t | 1.971.369 | 7,6 | 0,9670 | 0,8120 | 0,9643 | 0,9259 |
| YOLOv10n | 2.265.753 | 6,5 | 0,9704 | 0,8370 | 0,9689 | 0,9189 |
| YOLO26n | 2.375.421 | **5,2** | 0,9706 | 0,8233 | 0,9508 | **0,9297** |

(**Tebal** = nilai terbaik pada kolom tersebut.) YOLOv8n memimpin di mAP50-95 dan *precision*,
tetapi selisihnya terhadap YOLOv10n hanya 0,3 poin mAP50-95 meski YOLOv10n memakai ~20% lebih
sedikit GFLOPs — temuan ini menjadi salah satu dasar analisis *trade-off* di §3.5.1. Diskusi
per-kelas dan temuan detail lain ada di `../../docs/05_accuracy_results.md` §5.2–§5.4 dan tidak
diulang di sini untuk menghindari duplikasi.

> *Disclaimer* (mengikuti `../../docs/03_deployment_pipeline.md` §3.4): akurasi di atas diukur
> pada bobot FP32 (`.pt`); akurasi *deployment* FP16 yang benar-benar berjalan di Jetson via
> DeepStream diasumsikan setara dalam toleransi kuantisasi yang umum diamati pada model YOLO,
> namun **tidak diverifikasi secara independen** pada bagian ini — lihat §3.4.2.

**Nilai mAP50/mAP50-95 *as-deployed* FP16: `TODO — belum dieksekusi`.** Tabel yang seharusnya
menyandingkan mAP50/mAP50-95 hasil pipeline DeepStream FP16 sesungguhnya (dari `NvDsObjectMeta`,
dihitung dengan `pycocotools.cocoeval.COCOeval`) akan mengisi bagian ini setelah eksekusi
lapangan selesai (lihat status implementasi lengkap di §3.4.2). Angka tidak diisi dengan
perkiraan agar tidak melanggar aturan anti-karangan data (`../PANDUAN-AI.md`).

### 3.4.2 Analisis Deviasi Akurasi FP16 vs. Proxy FP32

**TODO — belum dieksekusi.** Infrastrukturnya sudah selesai diimplementasikan di kode
(`--dump-detections` di `src/main.cpp`, `scripts/prepare_eval_video.sh`,
`utils/eval_map/eval_deepstream_map.py`; lihat `BAB-2-Metode-Penelitian.md` §2.6.1 poin 4),
tetapi langkah eksekusi nyata di Jetson (ekspor 1.010 gambar val ke perangkat, jalankan dump
deteksi FP16, hitung mAP, bandingkan dengan §3.4.1) **belum dilakukan** pada 60 *run* yang
dilaporkan di bab ini — *run* tersebut memakai `video_testing.mp4`, bukan video hasil ekspor
gambar val KITTI. Tabel deviasi (Δ mAP50, Δ mAP50-95, FP32 vs. FP16) akan diisi setelah langkah
(a)–(d) di `BAB-2-Metode-Penelitian.md` §2.6.1 poin 4 selesai.

Sebagai kriteria keberhasilan yang sudah ditetapkan lebih dahulu (`BAB-2-Metode-Penelitian.md`
§2.6.2), deviasi ini akan dinilai sebagai **pass/fail** *sanity check* — bukan variabel yang
dibandingkan antar model — dengan tujuan membuktikan bahwa optimasi performa komputasi (§3.1–3.3)
tidak mengorbankan akurasi deteksi di luar batas toleransi yang wajar untuk kuantisasi FP16 pada
model YOLO.

## 3.5 Pembahasan Akhir dan Analisis Trade-off

### 3.5.1 Kompromi Kecepatan, Akurasi, dan Efisiensi Energi

Menggabungkan §3.4.1 (akurasi) dan §3.1–§3.3 (*runtime*/hardware) — lihat
`../eksperimen/plots/tradeoff_map_vs_fps.png` dan `tradeoff_map_vs_power.png` — seluruh model
*baseline* melampaui ambang *real-time* dengan margin besar (§3.1.1), sehingga tidak ada model
yang "gugur" murni karena kecepatan. Karena itu, rekomendasi disusun berkondisi (mengikuti
struktur `../../docs/07_tradeoff_analysis.md` §7.5), bukan klaim satu model "terbaik" tunggal:

- **Prioritas akurasi maksimum**: YOLOv8n (mAP50-95 0,8397 tertinggi) dengan *tracker* NvSORT
  (FPS 66,86, hampir tidak berbeda dari NvDCF secara statistik — §3.3.2 — namun `Lat_Tracker_ms`
  jauh lebih rendah dan energi/*frame* lebih hemat, §3.3.3).
- **Prioritas efisiensi komputasi/GFLOPs terendah dengan akurasi kompetitif**: YOLO26n (GFLOPs
  5,2, mAP50 0,9706 hampir menyamai YOLOv8n) dipasangkan NvSORT — FPS meningkat signifikan
  (65,66 → 67,29, §3.3.2) dan GPU% turun dari 75,6% ke 68,1% (§3.3.3).
- **YOLOv10n sebagai kandidat "*default*" Pareto-*front*** — akurasi hampir menyamai YOLOv8n (Δ
  mAP50-95 hanya 0,3 poin) dengan GFLOPs jauh lebih rendah (6,5 vs. 8,1) dan FPS tertinggi di
  antara keempat model *baseline* (67,02–67,34 FPS); efek *tracker* pada model ini juga paling
  kecil dan tidak signifikan (§3.3.2), membuatnya paling "toleran" terhadap pilihan *tracker*
  apa pun.
- **YOLOv9t tidak direkomendasikan pada konfigurasi *default* (NvDCF)** — akurasinya paling
  rendah di antara keempat model (§3.4.1) *dan* menunjukkan interaksi *bottleneck* paling parah
  dengan *tracker* berat (§3.1.3, §3.3.1–3.3.2). Jika arsitektur ini tetap ingin dipakai,
  **NvSORT bukan lagi opsional melainkan hampir wajib** — kombinasi YOLOv9t+NvDCF adalah
  satu-satunya yang mendekati (meski masih melampaui) ambang *real-time* dengan margin tersempit
  (§3.1.1).
- **EfficientNMS tidak direkomendasikan** untuk kedua model yang diuji pada Jetson Orin Nano —
  tidak memberi keuntungan *throughput* yang signifikan (YOLOv8n) atau secara signifikan lebih
  lambat (YOLOv9t) — lihat §3.2.2.

**mAP50-95 pada tabel/grafik *trade-off* ini masih memakai *proxy* FP32** (§3.4.1); interpretasi
di atas akan lebih kuat setelah §3.4.2 (verifikasi *as-deployed* FP16) tersedia, meski deviasi
yang diharapkan kecil berdasarkan literatur umum kuantisasi FP16 pada model YOLO.

**Perbandingan dengan penelitian terkait.** Temuan hasil negatif EfficientNMS (§3.2.2) melengkapi
— bukan bertentangan dengan — literatur akselerasi NMS yang dirujuk pada `BAB-1-Pendahuluan.md`
§1.1 (Chen dkk., 2022; Oro dkk., 2022; Yang dkk., 2025): ketiga studi tersebut menunjukkan
percepatan besar dengan membangun akselerator/kernel *kustom dari nol*, sedangkan penelitian ini
menguji pendekatan yang lebih umum diadopsi pengembang aplikasi — *plugin* vendor siap pakai
(`EfficientNMS_TRT`). Hasil yang berbeda arah ini menegaskan bahwa klaim akselerasi NMS paralel
pada literatur **tidak otomatis berlaku umum** untuk semua strategi implementasi, terutama pada
model kelas *nano/tiny* yang biaya NMS *baseline*-nya sudah relatif kecil (§3.2.1). Sementara itu,
untuk RM3, tidak ditemukan studi pembanding langsung yang mengukur efisiensi komputasi NvDCF vs.
NvSORT pada pipeline DeepStream di perangkat Jetson-*class* (`BAB-1-Pendahuluan.md` §1.1) —
temuan §3.3 (penghematan NvSORT bersifat universal pada level komponen namun kondisional pada
model di level *throughput*) dengan demikian mengisi celah literatur tersebut, sejalan dengan
kerangka metodologis MLPerf Mobile Inference Benchmark (`BAB-2-Metode-Penelitian.md` §2.5.3) yang
menjadi rujukan pendekatan "akurasi sebagai ambang, komputasi sebagai variabel yang diukur".

### 3.5.2 Keterbatasan Sistem dan Rekomendasi

**Keterbatasan metodologis eksekusi 60 *run* ini:**

1. **Durasi klip video pengujian relatif singkat** (± 13–20 detik per *run*, lebih pendek dari
   rekomendasi 180 detik di `04_benchmark_protocol.md` §4.3) karena `run_all_benchmark.sh` tidak
   menetapkan `--duration` dan bergantung pada panjang alami `video_testing.mp4`. Setelah buang
   *warm-up* 10 detik, jumlah *frame* yang dianalisis per skenario berkisar 633–1.399 (gabungan 5
   repetisi) — cukup untuk membedakan pola besar seperti pada §3.3, tetapi estimasi persentil
   ekstrem (p99) pada skenario dengan *n* lebih kecil (mis. NvSORT, ~630–660 *frame*) memiliki
   margin ketidakpastian yang lebih besar dibanding jika direkam pada klip yang lebih panjang.
2. **Satu klip video dengan satu tingkat kepadatan objek** (`04_benchmark_protocol.md` §4.1) —
   hasil ini belum mengonfirmasi apakah pola *bottleneck* YOLOv9t+NvDCF (§3.1.3, §3.3) akan makin
   parah atau justru mengecil pada skenario lalu lintas yang lebih padat/lebih jarang.

**Keterbatasan modul memori 4GB.** Seluruh 12 konfigurasi menggunakan 1.265–1.449 MB RAM
(§3.3.3) dari total 4GB yang tersedia pada modul Jetson Orin Nano yang dipakai
(`BAB-2-Metode-Penelitian.md` §2.2.1) — sekitar 31–35% kapasitas untuk satu *stream* video dan
satu model aktif. Meski masih menyisakan *headroom* untuk skenario satu-kamera satu-model seperti
pada penelitian ini, angka ini mengindikasikan bahwa skenario *deployment* ADAS yang lebih
kompleks (mis. beberapa kamera sekaligus, atau beberapa model berjalan bersamaan untuk tugas
persepsi berbeda) berisiko mendekati batas kapasitas memori pada SKU 4GB — sebuah pertimbangan
praktis bagi pengembang yang mempertimbangkan platform ini untuk sistem produksi, di luar SKU 8GB
yang tidak diuji pada penelitian ini.

**Pengaruh *thermal throttling*.** Risiko *thermal throttling* dimitigasi secara prosedural
melalui jeda *cooldown* 60 detik antar skenario dan pembersihan *cache* (`sync` + `drop_caches`,
`BAB-2-Metode-Penelitian.md` §2.2.6), mengikuti rekomendasi `04_benchmark_protocol.md` §4.3.
Namun demikian, penelitian ini **tidak mengukur suhu SoC secara langsung** — kanal pengumpulan
data hardware (§2.4) hanya mencakup GPU%, CPU%, RAM, dan daya per-*rail* dari `tegrastats`, tanpa
membaca zona termal (`/sys/devices/virtual/thermal/`). Dengan demikian, efektivitas mitigasi
*cooldown* dalam mencegah *throttling* bersifat **prosedural** (mengikuti praktik yang
direkomendasikan), bukan **terverifikasi langsung** dengan data suhu — sebuah keterbatasan
instrumentasi yang perlu dicantumkan secara jujur, konsisten dengan `../PANDUAN-AI.md` dan
`../../docs/08_limitations_future_work.md`.

**Rekomendasi pengembangan lanjutan** (diurutkan dari usaha kecil/dampak sedang ke usaha
besar/dampak besar, mengikuti `../../docs/08_limitations_future_work.md` §8.2):

1. Menyelesaikan verifikasi akurasi *as-deployed* FP16 (§3.4.2) sebagai prioritas utama, karena
   seluruh rekomendasi *trade-off* di §3.5.1 masih bergantung pada *proxy* FP32.
2. Menambah kanal pengukuran suhu SoC (`tegrastats` sudah melaporkan suhu, namun belum
   diekstraksi oleh `LogParser` saat ini) untuk memverifikasi langsung asumsi mitigasi *thermal
   throttling* di atas, alih-alih bergantung pada prosedur *cooldown* semata.
3. Eksperimen presisi INT8 sebagai variabel tambahan (di luar SKU tanpa DLA seperti pada
   penelitian ini), untuk melengkapi perbandingan FP16 vs. INT8 secara terukur, bukan cuma
   teoretis.
4. Menguji skenario tambahan (kepadatan lalu lintas tinggi, cahaya rendah, cuaca buruk, klip
   video lebih panjang sesuai rekomendasi protokol) untuk menguji generalisasi temuan §3.1–§3.3
   di luar satu klip video terkontrol yang dipakai penelitian ini.
5. Mengukur kualitas/ketahanan *tracking* (ID *switch*, MOTA/IDF1) secara terpisah dari efisiensi
   komputasi (di luar *scope* penelitian ini, `BAB-1-Pendahuluan.md` §1.5 poin 5), untuk
   melengkapi gambaran *trade-off* NvDCF vs. NvSORT secara menyeluruh.
6. Menguji skenario *deployment* multi-kamera/multi-model pada SKU 4GB maupun 8GB untuk
   mengonfirmasi secara empiris batas praktis keterbatasan memori yang didiskusikan di atas.

# BAB II — TINJAUAN PUSTAKA

## 2.1 Penelitian Terkait (State of the Art)

Bagian ini merangkum penelitian pada tabel "Penelitian Terkait" proposal, dikelompokkan
menjadi tiga klaster tema yang relevan dengan tiga rumusan masalah di Bab I: (1) deteksi objek
real-time untuk ADAS pada perangkat edge, (2) akselerasi/optimasi Non-Maximum Suppression, dan
(3) konteks umum keselamatan/keandalan ADAS. Klaster ketiga sengaja dipisah karena penelitian
ini **tidak** menemukan studi yang secara langsung membandingkan efisiensi komputasi antar
algoritma *tracker* (NvDCF vs. NvSORT) pada pipeline DeepStream di perangkat edge kelas Jetson
Orin Nano — celah inilah yang mendasari rumusan masalah #3 (§2.3).

### 2.1.1 Deteksi Objek Real-Time untuk ADAS pada Edge Device

Ayachi dkk. (2025) melakukan evaluasi komprehensif terhadap model YOLO v1 hingga v9
menggunakan *transfer learning* pada dataset BDD100K, dan menyimpulkan YOLOv9 memberikan
keseimbangan optimal antara kecepatan (34 FPS) dan akurasi (85,54% mAP) untuk aplikasi ADAS
real-time. Temuan ini menjadi salah satu dasar pemilihan varian YOLO kelas *nano/tiny* sebagai
model uji pada penelitian ini — model kelas nano/tiny dipilih agar anggaran komputasi setara
dengan target perangkat edge.

Dhatrika dkk. (2025) mengimplementasikan YOLOv9 pada NVIDIA Jetson Nano menggunakan optimasi
TensorRT dan kerangka kerja DeepStream, mencapai mAP tinggi (91,9%) yang secara signifikan
mengungguli model lama seperti YOLOv5 dan YOLOv8. Penelitian ini paling relevan secara
metodologis dengan rumusan masalah #1 — sama-sama menggunakan kombinasi TensorRT + DeepStream
pada keluarga Jetson Nano/Orin Nano untuk deteksi real-time — namun tidak membahas perbandingan
antar-generasi model YOLO maupun optimasi *post-processing* (NMS) yang menjadi fokus tambahan
penelitian ini.

Chaman dkk. (2025) menerapkan YOLOv11 pada Jetson Nano dan Raspberry Pi 5 dengan dataset kustom
38.500 gambar, mencapai mAP@50 sebesar 98,1% dan membuktikan YOLOv11 cukup tangguh untuk
mendeteksi kendaraan modern seperti *e-scooter*. Guerrouj dkk. (2025) menguji kuantisasi
*post-training* INT8 pada YOLOv4 untuk perangkat edge, menaikkan kecepatan Jetson Nano dari
<2 FPS menjadi 5 FPS dengan kehilangan akurasi minimal — relevan sebagai pembanding trade-off
presisi (INT8 vs. FP16 yang dipakai penelitian ini).

Bouazizi dkk. (2024) melatih ulang model SSD-MobileNet pada subset MS COCO khusus objek jalan
raya, mencapai mAP 65,41% pada 71 FPS dengan memprioritaskan *recall* tinggi (0,873) demi
keselamatan deteksi pejalan kaki. Xie dkk. (2024) menggabungkan YOLOv5s dengan **DeepStream
SDK** untuk deteksi objek real-time dari video inspeksi UAV, mencapai mAP50 93,2% — menjadi
salah satu preseden literatur yang membuktikan efektivitas *framework* DeepStream (SDK yang
sama dipakai pada penelitian ini) di luar domain otomotif.

Tsai & Hsieh (2025) mengembangkan sistem peringatan tabrakan (*collision warning*) real-time
untuk kendaraan otonom berbasis YOLOv8n dikombinasikan dengan *stereo vision* SGBM, dan
menemukan YOLOv8n mencapai kecepatan inferensi 112 FPS dengan ukuran model hanya 4,5 MB —
menjadikannya pilihan paling praktis untuk deteksi kendaraan real-time pada platform
*embedded* dengan sumber daya terbatas (*MDPI Electronics*, 14(21), 4275).

Wu dkk. (2024) mengusulkan perbaikan YOLOv8 untuk skenario lalu lintas real-time ("Road object
detection based on improved YOLOv8 for real-time traffic scenarios", *Sensors*, 24(3), 1023),
dikutip sebagai bagian dari klaster literatur deteksi objek real-time berbasis YOLO untuk
konteks jalan raya; ringkasan metodologi/hasil rinci dari sitasi ini masih dalam proses
verifikasi lanjutan oleh penulis.

### 2.1.2 Optimasi dan Akselerasi Non-Maximum Suppression

Klaster ini menjadi dasar literatur untuk rumusan masalah #2. Chen dkk. (2022) memperkenalkan
ShapoolNMS, akselerator perangkat keras skalabel untuk algoritma PSRR-MaxpoolNMS, mencapai
percepatan 305x hingga 3626x dibandingkan perangkat lunak GreedyNMS dengan akurasi deteksi
yang tetap terjaga. Oro dkk. (2022) mengembangkan kernel CUDA skalabel berbasis matriks
adjacency boolean untuk menyelesaikan NMS secara paralel pada GPU tertanam (Tegra X1/X2),
mampu mengelompokkan 1024 objek dalam ±1 ms dan mencapai percepatan 14x–40x dibanding metode
NMS berbasis CNN yang ada (*The Computer Journal*, 65(4), 773–787). Yang dkk. (2025) mengusulkan
akselerator perangkat keras untuk *post-processing* dengan prioritas klasifikasi guna mengurangi
kalkulasi IoU redundan, mencapai percepatan 19,89x pada tahap inferensi dan 7,55x pada tugas NMS
dibanding sistem GPU tradisional (RTX 2080 Ti).

Ketiga penelitian ini secara konsisten menunjukkan bahwa memindahkan NMS dari implementasi
CPU/host standar ke eksekusi paralel di perangkat keras (GPU/akselerator khusus) memberikan
percepatan signifikan — inilah landasan konseptual proposal awal untuk mengimplementasikan NMS
paralel. Namun ketiganya membangun akselerator/kernel *khusus dari nol* (custom hardware atau
custom CUDA kernel), berbeda dengan pendekatan penelitian ini yang menggunakan plugin
`EfficientNMS_TRT` bawaan TensorRT (lihat §2.2.5) — sebuah integrasi *graph-level* siap pakai,
bukan kernel paralel yang ditulis manual. Perbedaan pendekatan ini penting untuk dijelaskan
secara eksplisit saat membandingkan hasil penelitian ini dengan ketiga rujukan di atas, karena
mekanisme percepatan yang diharapkan (kernel kustom vs. plugin vendor) tidak sepenuhnya identik.

### 2.1.3 Efisiensi Real-Time dan Penjadwalan pada Edge Device

Klaster ini merangkum penelitian yang mendasari Bab I §1.1 Latar Belakang, menjelaskan *mengapa*
efisiensi komputasi/real-time pada perangkat edge menjadi persoalan penting yang mendasari
seluruh tiga rumusan masalah.

Choi dkk. (2024) menunjukkan bahwa deteksi dan pelacakan objek berbasis perangkat *embedded
edge* merupakan aspek mendasar untuk pembentukan informasi lingkungan (*local dynamic map*)
secara real-time pada kendaraan cerdas — relevan langsung dengan penggabungan deteksi (rumusan
masalah #1–2) dan *tracking* (rumusan masalah #3) pada penelitian ini. Nigade dkk. (2024)
mengkaji *inference serving* dengan *end-to-end latency SLOs* pada jaringan edge yang dinamis,
menyimpulkan ketepatan waktu penyajian inferensi sama pentingnya dengan akurasi model. Zhang
dkk. (2024) meneliti penempatan *server* edge berbasis *graph clustering*, menekankan bahwa
pengaturan sumber daya yang tidak tepat dapat memperbesar latensi dan mengganggu pencapaian
kinerja real-time pada aplikasi kritis.

Ruiz-Barroso dkk. (2025) membahas optimasi *deployment* pada platform *embedded heterogen*
agar kebutuhan *frame rate*, akurasi, dan efisiensi energi tetap terpenuhi secara simultan.
Suder dkk. (2023) mengevaluasi kebutuhan daya perangkat *embedded* untuk deteksi garis video
real-time, menemukan bahwa pemrosesan real-time selalu menuntut kompromi antara kecepatan,
efisiensi energi, dan stabilitas kinerja pada beban kerja yang berubah-ubah — preseden langsung
untuk metrik utilisasi daya (`tegrastats`) yang dipakai penelitian ini (lihat §2.2.8). Seyfipoor
dkk. (2026) meneliti penjadwalan tugas berbasis urgensi adaptif pada sistem ADAS, menemukan
bahwa penjadwalan tugas ADAS sangat peka terhadap *deadline* dan kegagalan memenuhi batas waktu
dapat berimplikasi pada kegagalan sistem secara fungsional. Shah dkk. (2025) mencatat bahwa
penguatan ADAS melalui *computer vision* dan *machine learning* meningkatkan kapabilitas
deteksi namun juga menuntut proses komputasi yang lebih besar — argumen yang menjustifikasi
perlunya penelitian trade-off akurasi-vs-efisiensi seperti yang dilakukan pada penelitian ini.

### 2.1.4 Konteks Umum ADAS dan Keselamatan

Neumann (2024) menganalisis operasional berbagai sensor ADAS dan melakukan survei terhadap 80
pengemudi, menemukan lebih dari 50% pengemudi merasa jauh lebih aman dengan ADAS namun hasil
survei menekankan perlunya peningkatan presisi untuk mengurangi *alarm palsu*. Costa dkk.
(2025) mengkaji dampak keselamatan ADAS di Eropa hingga 2030, sejalan dengan klaim bahwa
penerapan ADAS berpotensi menurunkan angka kecelakaan berat (dikutip di Bab I §1.1 paragraf 1).
Kedua penelitian ini tidak membahas aspek teknis pipeline, tetapi memberi konteks motivasi
(mengapa performa real-time dan presisi deteksi ADAS penting) yang relevan untuk Bab I §1.1
Latar Belakang.

## 2.2 Landasan Teori

### 2.2.1 ADAS dan Perception Layer

*Advanced Driver-Assistance System* (ADAS) adalah kumpulan sistem elektronik yang membantu
pengemudi dalam berkendara dan memarkir kendaraan, umumnya melalui rangkaian sensor (kamera,
radar, LiDAR) dan unit pemrosesan yang mendeteksi kondisi lingkungan sekitar kendaraan secara
real-time (Shah dkk., 2025; Neumann, 2024). Penelitian ini secara eksplisit membatasi diri pada
*perception layer* — tahap deteksi objek dari citra kamera — dan tidak mencakup lapisan
prediksi, *planning*, maupun kontrol kendaraan (lihat Bab I §1.5 Batasan Masalah).

### 2.2.2 NVIDIA DeepStream SDK

NVIDIA DeepStream adalah SDK untuk membangun aplikasi *streaming analytics* berbasis GStreamer
yang dioptimalkan untuk perangkat keras NVIDIA, mencakup tahap decode video, inferensi deep
learning, *object tracking*, hingga penggambaran hasil (on-screen display) dan *output* akhir
dalam satu *pipeline* tunggal. Karakteristik kunci DeepStream yang relevan untuk penelitian ini
adalah penggunaan buffer **NVMM (NVIDIA Memory Manager)**, yang memungkinkan seluruh tahap
pipeline (decode → inferensi → tracking → encode) berjalan di memori GPU tanpa banyak *copy*
data bolak-balik ke CPU (*zero-copy*) — karakteristik ini menjadi salah satu alasan pemilihan
DeepStream dibanding pipeline manual berbasis OpenCV + TensorRT/ONNXRuntime.

Arsitektur pipeline DeepStream pada penelitian ini terdiri atas elemen `nvstreammux` (batching
frame ke buffer NVMM), `nvinfer` (Primary GIE — inferensi model YOLO), `nvtracker` (asosiasi ID
objek antar frame), `nvvideoconvert`, dan `nvdsosd` (penggambaran *bounding box*), sebelum
diteruskan ke *output sink* (RTSP, layar lokal, atau berkas MP4) — lihat diagram lengkap di
Bab III §3.4.

### 2.2.3 TensorRT dan Presisi Komputasi (FP32, FP16, INT8)

TensorRT adalah *runtime* inferensi deep learning dari NVIDIA yang mengoptimalkan model terlatih
(melalui *layer fusion*, kernel auto-tuning, dan kuantisasi presisi) menjadi *engine* yang
dieksekusi secara efisien pada GPU target. Tiga tingkat presisi umum yang didukung adalah FP32
(presisi penuh, baseline akurasi), FP16 (setengah presisi, memanfaatkan Tensor Core untuk
percepatan dengan penurunan akurasi yang umumnya dapat diabaikan), dan INT8 (kuantisasi bilangan
bulat 8-bit, tercepat namun membutuhkan dataset kalibrasi tambahan dan berisiko kehilangan
akurasi lebih besar).

Penelitian ini menggunakan presisi **FP16** secara konsisten di seluruh model uji, dengan
pertimbangan: Jetson Orin Nano tidak memiliki *Deep Learning Accelerator* (DLA) seperti seri
Orin NX/AGX Orin sehingga keuntungan INT8 khas DLA tidak berlaku, sementara FP32 tidak
memanfaatkan penuh Tensor Core GPU Ampere pada perangkat ini untuk keuntungan akurasi yang
hampir tidak terasa. Perbandingan akurasi FP32-proxy (hasil `YOLO.val()` di `.pt`) vs. FP16
as-deployed (hasil pipeline DeepStream sesungguhnya) menjadi bagian dari metodologi pengujian
akurasi — lihat Bab III §3.5.

### 2.2.4 Arsitektur YOLO (You Only Look Once)

YOLO adalah keluarga arsitektur deteksi objek *single-stage* yang memprediksi *bounding box*
dan kelas objek dalam satu kali *forward pass* jaringan, menjadikannya cocok untuk aplikasi
real-time dibanding pendekatan *two-stage* (mis. Faster R-CNN). Penelitian ini menguji empat
varian generasi YOLO kelas nano/tiny — YOLOv8n, YOLOv9t, YOLOv10n, dan YOLO26n — yang dipilih
karena kelas ukuran model yang setara (anggaran komputasi kira-kira sebanding), sehingga selisih
hasil antar model lebih mencerminkan perbedaan arsitektur generasi ketimbang perbedaan skala
model. Salah satu perbedaan arsitektural penting antar generasi ini adalah pendekatan terhadap
NMS: sebagian model generasi baru (mis. YOLO26n dalam repositori ini) bersifat *NMS-free*, yaitu
tidak memerlukan tahap NMS terpisah karena model sudah dilatih untuk menghasilkan prediksi tanpa
duplikasi — poin ini relevan untuk interpretasi hasil rumusan masalah #2, karena optimasi NMS
(EfficientNMS) secara inheren tidak berlaku sama untuk model NMS-free.

### 2.2.5 Non-Maximum Suppression (NMS) dan EfficientNMS_TRT

Non-Maximum Suppression adalah algoritma *post-processing* yang menyaring kandidat *bounding
box* hasil deteksi mentah suatu model, membuang box yang tumpang tindih (berdasarkan ambang
*Intersection-over-Union*/IoU) dengan box lain yang memiliki skor keyakinan lebih tinggi untuk
objek yang sama. Implementasi NMS standar (mis. *GreedyNMS*) bersifat sekuensial dan sering
dieksekusi di CPU, sehingga berpotensi menjadi *bottleneck* pada pipeline inferensi real-time
di perangkat edge — motivasi inilah yang mendasari berbagai penelitian akselerasi NMS pada
§2.1.2.

Penelitian ini mengimplementasikan optimasi NMS menggunakan plugin **`EfficientNMS_TRT`**
bawaan TensorRT, yang menyisipkan algoritma NMS sebagai bagian dari *graph* TensorRT itu sendiri
sehingga dieksekusi penuh oleh kernel GPU tanpa *loop* NMS Python/CPU pada *engine* hasil, dan
tanpa mengubah arsitektur ONNX/engine *baseline* (`EfficientNMS_TRT` dipasang sebagai *tail*
tambahan yang bergantung pada output detektor). Pendekatan ini berbeda dari yang digambarkan
pada diagram arsitektur proposal awal (custom CUDA kernel dengan tahap *ParallelDispatch →
Workers evaluasi pasangan IoU → Custom Map Kernel → ParallelReduce*) — proposal menggambarkan
kernel paralel yang ditulis dari nol, sedangkan implementasi final memakai plugin siap pakai
vendor. Perbedaan ini dijelaskan secara konsisten di Bab I, III, dan IV.

### 2.2.6 Multi-Object Tracking pada Edge Device: NvDCF dan NvSORT

*Object tracking* pada pipeline video mengasosiasikan deteksi objek antar frame berurutan
dengan sebuah ID unik, mengurangi efek *flicker* (objek terdeteksi lalu hilang sesaat akibat
kegagalan deteksi sesaat, mis. karena *partial occlusion*). NVIDIA DeepStream menyediakan elemen
`nvtracker` yang dapat dikonfigurasi dengan beberapa profil algoritma tracking melalui berkas
YAML, di antaranya:

- **NvDCF** (*Discriminative Correlation Filter*): tracker berbasis filter korelasi dengan fitur
  visual yang dipelajari (*learned feature*), umumnya lebih akurat dalam mempertahankan
  identitas objek pada kondisi oklusi parsial, namun membutuhkan komputasi lebih berat karena
  proses ekstraksi fitur dan pencarian korelasi per objek per frame.
- **NvDCF_perf**: varian NvDCF yang dikonfigurasi untuk profil performa (parameter dituning ke
  arah kecepatan, mengorbankan sebagian akurasi asosiasi ID).
- **NvSORT**: tracker klasik berbasis filter Kalman dan algoritma Hungarian untuk asosiasi data
  (pendekatan SORT — *Simple Online and Realtime Tracking*), tanpa ekstraksi fitur visual
  sehingga jauh lebih ringan secara komputasi dibanding pendekatan berbasis *deep/correlation
  feature* seperti NvDCF, dengan kompromi pada ketahanan terhadap oklusi.

Baseline implementasi proyek ini menggunakan profil NvDCF, dengan ketiga berkas konfigurasi
(`config/tracker_nvdcf.yml`, `config/tracker_nvdcf_perf.yml`, `config/tracker_nvsort.yml`)
sudah tersedia di repositori proyek. Trade-off akurasi-vs-komputasi antara pendekatan
*feature-based* (NvDCF/NvDCF_perf) dan *motion-only* (NvSORT) inilah yang mendasari rumusan
masalah #3 — namun sesuai Batasan Masalah §1.5 poin 5, penelitian ini **murni mengukur sisi
efisiensi komputasi** (FPS, `Lat_Tracker_ms`, utilisasi resource) dan tidak mengukur sisi
kualitas asosiasi ID (MOTA/IDF1/ID switch).

Pembatasan ini bukan sekadar konsekuensi keterbatasan dataset, melainkan konsisten dengan
definisi sumbu perbandingan yang sudah melekat pada desain ketiga profil tracker itu sendiri.
Dokumentasi resmi NVIDIA (*Gst-nvtracker* plugin manual, DeepStream SDK) menyatakan NvSORT
"tidak melibatkan pemrosesan data piksel sama sekali" sehingga "efisien secara komputasi",
sedangkan NvDCF memakai *visual tracker* berbasis *discriminative correlation filter* — yaitu
pemrosesan fitur piksel per objek per frame (NVIDIA, DeepStream SDK Plugin Manual, bagian
*Gst-nvtracker*). Postingan blog resmi NVIDIA Developer soal DeepStream SDK 6.2 juga secara
eksplisit menempatkan NvSORT sebagai *"lightweight, CPU-only implementation but still
competitively accurate"* dan NvDCF sebagai penghasil *"best accuracy and robustness"* lewat
kombinasi *conventional ML* (DCF) dan *deep learning* (ReID) (NVIDIA Developer Blog, 2023).
Preset `NvDCF_perf` sendiri adalah hasil tuning resmi NVIDIA pada sumbu akurasi-vs-kecepatan
yang sama (varian `max_perf`/`perf`/`accuracy` dari NvDCF) — mengonfirmasi bahwa sumbu
akurasi-vs-komputasi ini memang didesain vendor sebagai *trade-off* yang sudah diketahui
karakteristiknya secara kualitatif, bukan sesuatu yang perlu diukur ulang oleh penelitian ini
untuk membuat pertanyaan "tracker mana yang lebih efisien secara komputasi" valid dijawab.

Pendekatan mengevaluasi pilihan algoritma murni dari sisi biaya komputasi — dengan karakter
akurasi/kualitas sudah diketahui dan tidak diukur ulang — juga punya preseden metodologis yang
mapan di luar domain tracking: benchmark **MLPerf Mobile Inference Benchmark** (Janapa Reddi
dkk., 2022) menetapkan pada setiap tugas pengujian sebuah *ambang akurasi dan kualitas minimum*
sebagai syarat kelulusan tetap, sementara metrik yang secara eksplisit dibandingkan dan
dilaporkan meningkat antar generasi submisi adalah latensi dan *throughput* (tercatat membaik
hingga 12x dalam rentang enam bulan pada salah satu kasus) — struktur yang serupa dengan
penelitian ini, karena karakteristik akurasi setiap tracker sudah didokumentasikan kualitatif
oleh vendor, dan kontribusi penelitian ini adalah mengkuantifikasi biaya komputasinya secara
spesifik pada perangkat Jetson Orin Nano yang belum ada di literatur (§2.1.3). Preseden ini
relevan pula karena MLPerf Mobile secara khusus menyasar evaluasi performa pada perangkat
*on-device*/*edge* dengan sumber daya terbatas — konteks yang sejalan dengan platform Jetson
Orin Nano pada penelitian ini, alih-alih server kelas *datacenter*.

Adapun ketidaktersediaan dataset ber-anotasi *track ID* berurutan tetap menjadi alasan
pendukung (bukan alasan tunggal): metrik kualitas tracking (MOTA/IDF1/ID switch) secara
struktural membutuhkan *ground truth* dengan field identitas objek yang konsisten antar frame
(mis. benchmark KITTI Tracking, MOTChallenge MOT16/17/20), berbeda dari dataset deteksi
single-frame seperti KITTI 2D Object Detection yang dipakai penelitian ini (kotak per frame
tanpa field ID lintas-frame) — sehingga menambah metrik ini bukan sekadar menghitung ulang,
melainkan membutuhkan dataset dan proses anotasi baru yang di luar ruang lingkup penelitian.

### 2.2.7 Jetson Orin Nano sebagai Platform Edge Device

NVIDIA Jetson Orin Nano adalah *System-on-Chip* (SoC) embedded berbasis arsitektur GPU Ampere,
bagian dari keluarga Jetson Orin yang ditujukan untuk aplikasi *edge AI* dengan konsumsi daya
rendah. Berbeda dari varian Jetson Orin NX dan Jetson AGX Orin, Jetson Orin Nano **tidak
dilengkapi Deep Learning Accelerator (DLA)** — komponen akselerator khusus inferensi terpisah
dari GPU utama — sehingga strategi optimasi yang bergantung pada DLA (seperti pada proposal
awal penelitian ini yang menargetkan Jetson AGX Orin) tidak dapat diterapkan pada perangkat ini
(lihat Bab I §1.1 dan §1.5 Batasan Masalah). Spesifikasi teknis rinci (jumlah CUDA core,
kapasitas RAM, performa AI dalam TOPS sesuai SKU dan mode daya yang digunakan) akan dilengkapi
merujuk pada *datasheet* resmi NVIDIA setelah konfigurasi akhir perangkat pengujian ditetapkan,
karena spesifikasi ini berbeda antar SKU (4GB/8GB) dan mode daya (7W/15W/25W/MAXN).

### 2.2.8 Metrik Evaluasi

Penelitian ini menggunakan dua kelompok metrik evaluasi, mengikuti pembagian pada proposal
(§"Analisis dan Benchmarking") dan implementasi *tooling* yang sudah tersedia:

- **Kualitas deteksi**: *precision*, *recall*, dan *mean Average Precision* (mAP, pada IoU
  threshold 0.5 dan rentang 0.5:0.95) — dihitung menggunakan `pycocotools.cocoeval.COCOeval`
  untuk hasil *as-deployed* FP16, dan `Ultralytics YOLO.val()` untuk baseline FP32-proxy.
- **Performa runtime**: *throughput* (FPS) dan latensi *end-to-end* maupun latensi
  per-komponen pipeline (`Lat_PreMux_ms`, `Lat_Mux_ms`, `Lat_Infer_ms`, `Lat_Tracker_ms`,
  `Lat_PreOSD_ms`, `Lat_OSD_ms`, `Lat_Output_ms` — kolom pada `fps.csv`), diukur dengan *pad
  probe* GStreamer pada tiap elemen pipeline agar tidak mengganggu jalur kritis yang sedang
  diukur (logger berjalan di thread terpisah via `GAsyncQueue`).
- **Utilisasi sumber daya**: penggunaan GPU (%), CPU per-core (%), RAM, dan konsumsi daya
  per-*rail*, diambil dari `tegrastats` dan diproses menjadi `hardware_analysis.csv`.

Kriteria evaluasi lengkap (ambang throughput/latensi real-time, serta metrik spesifik untuk
masing-masing rumusan masalah) dirinci di Bab III §3.6.

## 2.3 Kerangka Berpikir

Tinjauan literatur pada §2.1 menunjukkan tiga hal: (1) kombinasi model YOLO kelas nano/tiny
dengan DeepStream + TensorRT pada perangkat Jetson-class terbukti layak untuk deteksi objek
ADAS real-time (§2.1.1), namun studi pembanding antar-generasi YOLO terbaru (termasuk model
NMS-free seperti YOLO26n) pada Jetson Orin Nano spesifik masih terbatas; (2) akselerasi NMS
melalui eksekusi paralel GPU terbukti secara konsisten mempercepat *post-processing* pada
studi-studi sebelumnya (§2.1.2), namun studi tersebut memakai akselerator/kernel kustom,
sementara penelitian ini menguji pendekatan yang lebih umum diterapkan di industri — plugin
`EfficientNMS_TRT` bawaan vendor — sehingga hasil (termasuk potensi hasil negatif) tetap
merupakan kontribusi ilmiah yang sah karena menguji klaim akselerasi pada kondisi implementasi
yang lebih realistis untuk pengembang aplikasi (dibanding menulis kernel CUDA dari nol);
(3) tidak ditemukan studi yang membandingkan efisiensi komputasi algoritma *tracker*
(feature-based vs. motion-only) secara spesifik pada pipeline DeepStream di perangkat Jetson
Orin Nano — mengisi celah inilah yang menjadi kontribusi rumusan masalah #3, sebagai pengganti
rumusan masalah DLA pada proposal awal yang tidak dapat dilaksanakan pada perangkat target akhir
(lihat Bab I §1.1 dan §1.2).

Ketiga rumusan masalah pada Bab I — (1) kinerja real-time pipeline dasar, (2) efek EfficientNMS
terhadap efisiensi pipeline, (3) efek pemilihan algoritma tracker terhadap efisiensi komputasi —
dengan demikian saling melengkapi sebagai evaluasi bertahap terhadap satu pipeline DeepStream
yang sama: dari performa dasar, ke satu titik optimasi spesifik (NMS), ke satu variabel desain
lain yang memengaruhi *real-time budget* pipeline secara keseluruhan (tracker) — menjawab
kebutuhan literatur akan studi *end-to-end* pada perangkat edge kelas *entry-level* (Jetson
Orin Nano), bukan hanya perangkat kelas atas (Jetson AGX Orin) yang lebih umum diuji pada
literatur ADAS existing.

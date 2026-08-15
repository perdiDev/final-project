# BAB II — TINJAUAN PUSTAKA

> Status: draf pertama. §2.1 disusun dari tabel "Penelitian Terkait" dan Daftar Pustaka pada
> `../Proposal/Proposal Final Perdi - AGX Orin ADAS-1.pdf` (20 sitasi, sudah disetujui di
> seminar proposal) — bukan sitasi baru yang dicari ulang, supaya konsisten dengan apa yang
> sudah diperiksa dosen pembimbing. §2.2 (landasan teori) sebagian digrounding pada
> `../../docs/01_scope_and_architecture.md` dan `../../utils/trt_efficientnms/README.md` agar
> istilah teknis konsisten dengan implementasi nyata, bukan generalisasi buku teks semata.
> Field bertanda `[VERIFIKASI]` yang masih tersisa perlu dicek ulang oleh penulis — lihat
> ringkasan status di §2.1 bawah tabel. Seluruh 20 sitasi jurnal individual dari proposal
> sudah disalin ke `../journal/daftar-referensi.md`.

## 2.1 Penelitian Terkait (State of the Art)

Bagian ini merangkum 10 penelitian pada tabel "Penelitian Terkait" proposal, dikelompokkan
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
model uji pada penelitian ini (lihat `../../docs/01_scope_and_architecture.md` §1.4 — model
kelas nano/tiny dipilih agar anggaran komputasi setara dengan target perangkat edge).

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
presisi (INT8 vs. FP16 yang dipakai penelitian ini, lihat `../../docs/01_scope_and_architecture.md`
§1.4 "Kenapa precision FP16, bukan FP32 atau INT8?").

Bouazizi dkk. (2024) melatih ulang model SSD-MobileNet pada subset MS COCO khusus objek jalan
raya, mencapai mAP 65,41% pada 71 FPS dengan memprioritaskan *recall* tinggi (0,873) demi
keselamatan deteksi pejalan kaki. Xie dkk. (2024) menggabungkan YOLOv5s dengan **DeepStream
SDK** untuk deteksi objek real-time dari video inspeksi UAV, mencapai mAP50 93,2% — menjadi
salah satu preseden literatur yang membuktikan efektivitas *framework* DeepStream (SDK yang
sama dipakai pada penelitian ini, lihat `../../docs/01_scope_and_architecture.md` §1.3) di luar
domain otomotif.

Tsai & Hsieh (2025) mengembangkan sistem peringatan tabrakan (*collision warning*) real-time
untuk kendaraan otonom berbasis YOLOv8n dikombinasikan dengan *stereo vision* SGBM, dan
menemukan YOLOv8n mencapai kecepatan inferensi 112 FPS dengan ukuran model hanya 4,5 MB —
menjadikannya pilihan paling praktis untuk deteksi kendaraan real-time pada platform
*embedded* dengan sumber daya terbatas ([MDPI Electronics 14(21),
4275](https://doi.org/10.3390/electronics14214275)). Nama penulis "Tsai, Hsu, & Lin (2025)"
pada Daftar Pustaka proposal asli keliru — dikonfirmasi via DOI di atas bahwa penulis
sebenarnya adalah **Tsai & Hsieh**; penulisan sitasi di draf ini dan di
`../journal/daftar-referensi.md` sudah dikoreksi ke nama yang benar (dikonfirmasi penulis
2026-08-14).

**Catatan penghapusan sitasi:** "Wu dkk. (2024)" yang sebelumnya dikutip Daftar Pustaka
proposal sebagai "Road object detection based on improved YOLOv8 for real-time traffic
scenarios" (*Sensors*, 24(3), 1023, DOI `10.3390/s24031023`) **sudah dihapus dari skripsi ini**
(2026-08-14, atas instruksi penulis), setelah verifikasi langsung ke Crossref API resmi
mengonfirmasi DOI tersebut sebenarnya milik artikel lain yang sama sekali tidak berhubungan —
"Railway Catenary Condition Monitoring: A Systematic Mapping of Recent Research" (Chen,
Frøseth, Derosa, Lau, & Rønnquist, 2024, *Sensors* 24(3), 1023), tentang pemantauan kondisi
jaringan kabel listrik kereta api. Pencarian tambahan dengan judul dan kata kunci yang sama
juga tidak menemukan artikel dengan judul tersebut di penerbit mana pun, sehingga disimpulkan
sitasi ini adalah kesalahan kutip/DOI yang salah tempel pada Daftar Pustaka proposal asli.
Sitasi ini tidak pernah dipakai sebagai rujukan substantif di bagian manapun pada skripsi ini
(sengaja tidak diberi ringkasan sejak draf pertama), sehingga penghapusannya tidak mengubah
klaim ilmiah apa pun di draf. **Catatan untuk sidang/bimbingan**: karena sitasi ini bagian dari
Daftar Pustaka yang sudah disetujui di seminar proposal, penulis tetap disarankan
menginformasikan penghapusan ini ke dosen pembimbing (bukti verifikasi lengkap ada di
`../log/log-perubahan.md`), meski secara substansi tidak ada isi Bab I–IV yang bergantung pada
sitasi ini.

### 2.1.2 Optimasi dan Akselerasi Non-Maximum Suppression

Klaster ini menjadi dasar literatur untuk rumusan masalah #2. Chen dkk. (2022) memperkenalkan
ShapoolNMS, akselerator perangkat keras skalabel untuk algoritma PSRR-MaxpoolNMS, mencapai
percepatan 305x hingga 3626x dibandingkan perangkat lunak GreedyNMS dengan akurasi deteksi
yang tetap terjaga. Oro dkk. (2022) mengembangkan kernel CUDA skalabel berbasis matriks
adjacency boolean untuk menyelesaikan NMS secara paralel pada GPU tertanam (Tegra X1/X2),
mampu mengelompokkan 1024 objek dalam ±1 ms dan mencapai percepatan 14x–40x dibanding metode
NMS berbasis CNN yang ada (*The Computer Journal*, 65(4), 773–787). Yang dkk. (2025) mengusulkan akselerator perangkat keras untuk
*post-processing* dengan prioritas klasifikasi guna mengurangi kalkulasi IoU redundan, mencapai
percepatan 19,89x pada tahap inferensi dan 7,55x pada tugas NMS dibanding sistem GPU tradisional
(RTX 2080 Ti).

Ketiga penelitian ini secara konsisten menunjukkan bahwa memindahkan NMS dari implementasi
CPU/host standar ke eksekusi paralel di perangkat keras (GPU/akselerator khusus) memberikan
percepatan signifikan — inilah landasan konseptual proposal awal untuk mengimplementasikan NMS
paralel. Namun ketiganya membangun akselerator/kernel *khusus dari nol* (custom hardware atau
custom CUDA kernel), berbeda dengan pendekatan penelitian ini yang menggunakan plugin
`EfficientNMS_TRT` bawaan TensorRT (lihat §2.2.5 dan **`[VERIFIKASI]`** di
`BAB-1-Pendahuluan.md` §1.2 poin 2 soal perbedaan ini) — sebuah integrasi *graph-level* siap
pakai, bukan kernel paralel yang ditulis manual. Perbedaan pendekatan ini penting untuk
dijelaskan secara eksplisit saat membandingkan hasil penelitian ini dengan ketiga rujukan di
atas, karena mekanisme percepatan yang diharapkan (kernel kustom vs. plugin vendor) tidak
sepenuhnya identik.

### 2.1.3 Efisiensi Real-Time dan Penjadwalan pada Edge Device

Klaster ini merangkum penelitian yang dikutip pada Bab I §1.1 Latar Belakang (bukan pada
tabel "Penelitian Terkait" proposal) untuk menjelaskan *mengapa* efisiensi komputasi/real-time
pada perangkat edge menjadi persoalan penting yang mendasari seluruh tiga rumusan masalah —
klaim di bawah ini diambil apa adanya dari teks proposal yang sudah disetujui, bukan hasil baca
ulang jurnal secara independen sesi ini.

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

`[VERIFIKASI]` Ringkasan enam penelitian di atas (Choi, Nigade, Zhang, Ruiz-Barroso, Suder,
Seyfipoor, Shah) masih setingkat klaim tematik seperti yang dipakai di Bab I §1.1, karena
proposal tidak memberi ringkasan metodologi/hasil terstruktur untuk jurnal-jurnal ini (tidak
ada di tabel "Penelitian Terkait"). Kalau Bab II versi final butuh detail metodologi/hasil yang
lebih dalam per jurnal, penulis perlu membaca PDF aslinya satu per satu — jangan mengarang
detail metodologi di luar apa yang sudah dikutip proposal.

### 2.1.4 Konteks Umum ADAS dan Keselamatan

Neumann (2024) menganalisis operasional berbagai sensor ADAS dan melakukan survei terhadap 80
pengemudi, menemukan lebih dari 50% pengemudi merasa jauh lebih aman dengan ADAS namun hasil
survei menekankan perlunya peningkatan presisi untuk mengurangi *alarm palsu*. Costa dkk.
(2025) mengkaji dampak keselamatan ADAS di Eropa hingga 2030, sejalan dengan klaim bahwa
penerapan ADAS berpotensi menurunkan angka kecelakaan berat (dikutip di Bab I §1.1 paragraf 1).
Kedua penelitian ini tidak membahas aspek teknis pipeline, tetapi memberi konteks motivasi
(mengapa performa real-time dan presisi deteksi ADAS penting) yang relevan untuk Bab I §1.1
Latar Belakang.

### Catatan referensi jurnal lain

Dari 20 sitasi asli Daftar Pustaka proposal, 19 sitasi kini sudah disebutkan dan diberi konteks
di salah satu subbagian §2.1 di atas (10 pada tabel "Penelitian Terkait" asli + 9 tambahan yang
sebelumnya hanya dikutip di Latar Belakang proposal tanpa ringkasan tabel); 1 sitasi (Wu dkk.,
2024) sudah dihapus permanen karena terbukti tidak valid (lihat penjelasan di bawah). Status
verifikasi per kelompok:

- **Sudah punya ringkasan metodologi/hasil terstruktur** (dari tabel proposal): 10 jurnal di
  §2.1.1–2.1.2.
- **Baru setingkat klaim tematik** (dikutip proposal di Latar Belakang, belum ada ringkasan
  metodologi/hasil independen): Choi, Nigade, Zhang, Ruiz-Barroso, Suder, Seyfipoor, Shah —
  §2.1.3. `[VERIFIKASI]` perlu baca PDF asli kalau butuh detail lebih dalam.
- **Diverifikasi ulang via pencarian DOI sesi ini** (bukan dari tabel proposal): Tsai & Hsieh
  (2025) — nama penulis di Daftar Pustaka proposal ("Tsai, Hsu, & Lin") keliru, sudah
  dikoreksi ke nama yang benar di §2.1.1 dan `../journal/daftar-referensi.md` (dikonfirmasi
  penulis 2026-08-14).
- **DIHAPUS — terkonfirmasi tidak valid** (verifikasi 2026-08-14 via Crossref API resmi): Wu
  dkk. (2024, Sensors 24(3), 1023, DOI `10.3390/s24031023`) — DOI tersebut ternyata milik
  artikel lain yang sama sekali tidak berhubungan (Chen dkk., 2024, "Railway Catenary Condition
  Monitoring"). Tidak ditemukan pula artikel berjudul "Road object detection based on improved
  YOLOv8 for real-time traffic scenarios" oleh penulis bernama Wu di pencarian tambahan.
  Kemungkinan besar kesalahan kutip pada Daftar Pustaka proposal asli — sitasi ini sudah
  **dihapus permanen** dari `../journal/daftar-referensi.md` atas instruksi penulis (2026-08-14).
  Detail temuan verifikasi di §2.1.1 dan `../log/log-perubahan.md`.

19 sitasi yang tersisa (dari 20 sitasi asli) sudah dipindahkan ke
`../journal/daftar-referensi.md` (kolom File masih "—" karena PDF individualnya belum
didownload ke folder `journal/`) — lihat entri log terbaru
untuk detail.

## 2.2 Landasan Teori

### 2.2.1 ADAS dan Perception Layer

*Advanced Driver-Assistance System* (ADAS) adalah kumpulan sistem elektronik yang membantu
pengemudi dalam berkendara dan memarkir kendaraan, umumnya melalui rangkaian sensor (kamera,
radar, LiDAR) dan unit pemrosesan yang mendeteksi kondisi lingkungan sekitar kendaraan secara
real-time (Shah dkk., 2025; Neumann, 2024). Penelitian ini secara eksplisit membatasi diri pada
*perception layer* — tahap deteksi objek dari citra kamera — dan tidak mencakup lapisan
prediksi, *planning*, maupun kontrol kendaraan (lihat `../../docs/01_scope_and_architecture.md`
§1.1 dan Bab I §1.5 Batasan Masalah).

### 2.2.2 NVIDIA DeepStream SDK

NVIDIA DeepStream adalah SDK untuk membangun aplikasi *streaming analytics* berbasis GStreamer
yang dioptimalkan untuk perangkat keras NVIDIA, mencakup tahap decode video, inferensi deep
learning, *object tracking*, hingga penggambaran hasil (on-screen display) dan *output* akhir
dalam satu *pipeline* tunggal. Karakteristik kunci DeepStream yang relevan untuk penelitian ini
adalah penggunaan buffer **NVMM (NVIDIA Memory Manager)**, yang memungkinkan seluruh tahap
pipeline (decode → inferensi → tracking → encode) berjalan di memori GPU tanpa banyak *copy*
data bolak-balik ke CPU (*zero-copy*) — karakteristik ini menjadi salah satu alasan pemilihan
DeepStream dibanding pipeline manual berbasis OpenCV + TensorRT/ONNXRuntime (lihat
`../../docs/01_scope_and_architecture.md` §1.4).

Arsitektur pipeline DeepStream pada penelitian ini terdiri atas elemen `nvstreammux` (batching
frame ke buffer NVMM), `nvinfer` (Primary GIE — inferensi model YOLO), `nvtracker` (asosiasi ID
objek antar frame), `nvvideoconvert`, dan `nvdsosd` (penggambaran *bounding box*), sebelum
diteruskan ke *output sink* (RTSP, layar lokal, atau berkas MP4) — lihat diagram lengkap di
`../../docs/01_scope_and_architecture.md` §1.2 dan implementasi konkret di `../../src/main.cpp`.

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
hampir tidak terasa (lihat `../../docs/01_scope_and_architecture.md` §1.4 dan Bab I §1.1
paragraf mengenai keterbatasan DLA). Perbandingan akurasi FP32-proxy (hasil `YOLO.val()` di
`.pt`) vs. FP16 as-deployed (hasil pipeline DeepStream sesungguhnya) menjadi bagian dari
metodologi pengujian akurasi — lihat `../../docs/05_accuracy_results.md` dan
`Bab-3-Metodologi-Penelitian.md` §3.5.

### 2.2.4 Arsitektur YOLO (You Only Look Once)

YOLO adalah keluarga arsitektur deteksi objek *single-stage* yang memprediksi *bounding box*
dan kelas objek dalam satu kali *forward pass* jaringan, menjadikannya cocok untuk aplikasi
real-time dibanding pendekatan *two-stage* (mis. Faster R-CNN). Penelitian ini menguji empat
varian generasi YOLO kelas nano/tiny — YOLOv8n, YOLOv9t, YOLOv10n, dan YOLO26n — yang dipilih
karena kelas ukuran model yang setara (anggaran komputasi kira-kira sebanding), sehingga selisih
hasil antar model lebih mencerminkan perbedaan arsitektur generasi ketimbang perbedaan skala
model (lihat `../../docs/01_scope_and_architecture.md` §1.4). Salah satu perbedaan arsitektural
penting antar generasi ini adalah pendekatan terhadap NMS: sebagian model generasi baru (mis.
YOLO26n dalam repositori ini) bersifat *NMS-free*, yaitu tidak memerlukan tahap NMS terpisah
karena model sudah dilatih untuk menghasilkan prediksi tanpa duplikasi (lihat catatan parser
NMS-free di `../../docs/08_limitations_future_work.md`) — poin ini relevan untuk interpretasi
hasil rumusan masalah #2, karena optimasi NMS (EfficientNMS) secara inheren tidak berlaku sama
untuk model NMS-free.

### 2.2.5 Non-Maximum Suppression (NMS) dan EfficientNMS_TRT

Non-Maximum Suppression adalah algoritma *post-processing* yang menyaring kandidat *bounding
box* hasil deteksi mentah suatu model, membuang box yang tumpang tindih (berdasarkan ambang
*Intersection-over-Union*/IoU) dengan box lain yang memiliki skor keyakinan lebih tinggi untuk
objek yang sama. Implementasi NMS standar (mis. *GreedyNMS*) bersifat sekuensial dan sering
dieksekusi di CPU, sehingga berpotensi menjadi *bottleneck* pada pipeline inferensi real-time
di perangkat edge — motivasi inilah yang mendasari berbagai penelitian akselerasi NMS pada
§2.1.2.

Penelitian ini mengimplementasikan optimasi NMS menggunakan plugin **`EfficientNMS_TRT`**
bawaan TensorRT (lihat `../../utils/trt_efficientnms/README.md`), yang menyisipkan algoritma
NMS sebagai bagian dari *graph* TensorRT itu sendiri sehingga dieksekusi penuh oleh kernel GPU
tanpa *loop* NMS Python/CPU pada *engine* hasil, dan tanpa mengubah arsitektur ONNX/engine
*baseline* (`EfficientNMS_TRT` dipasang sebagai *tail* tambahan yang bergantung pada output
detektor). `[VERIFIKASI]` Pendekatan ini **berbeda** dari yang digambarkan pada diagram
arsitektur proposal awal (custom CUDA kernel dengan tahap *ParallelDispatch → Workers evaluasi
pasangan IoU → Custom Map Kernel → ParallelReduce*) — proposal menggambarkan kernel paralel
yang ditulis dari nol, sedangkan implementasi final memakai plugin siap pakai vendor. Perbedaan
ini perlu dijelaskan secara konsisten di Bab I, III, dan IV (lihat anotasi terkait di Bab I
§1.2 poin 2).

### 2.2.6 Multi-Object Tracking pada Edge Device: NvDCF dan NvSORT

*Object tracking* pada pipeline video mengasosiasikan deteksi objek antar frame berurutan
dengan sebuah ID unik, mengurangi efek *flicker* (objek terdeteksi lalu hilang sesaat akibat
kegagalan deteksi sesaat, mis. karena *partial occlusion*) — lihat
`../../docs/01_scope_and_architecture.md` §1.4. NVIDIA DeepStream menyediakan elemen
`nvtracker` yang dapat dikonfigurasi dengan beberapa profil algoritma tracking melalui berkas
YAML. Penelitian ini membandingkan dua profil yang mewakili ujung-ujung spektrum efisiensi
komputasi:

- **NvDCF** (*Discriminative Correlation Filter*): tracker berbasis filter korelasi dengan fitur
  visual yang dipelajari (*learned feature*), umumnya lebih akurat dalam mempertahankan
  identitas objek pada kondisi oklusi parsial, namun membutuhkan komputasi lebih berat karena
  proses ekstraksi fitur dan pencarian korelasi per objek per frame.
- **NvSORT**: tracker klasik berbasis filter Kalman dan algoritma Hungarian untuk asosiasi data
  (pendekatan SORT — *Simple Online and Realtime Tracking*), tanpa ekstraksi fitur visual
  sehingga jauh lebih ringan secara komputasi dibanding pendekatan berbasis *deep/correlation
  feature* seperti NvDCF, dengan kompromi pada ketahanan terhadap oklusi.

Baseline implementasi proyek ini menggunakan profil NvDCF (lihat
`../../docs/01_scope_and_architecture.md` §1.3), dengan kedua berkas konfigurasi
(`config/tracker_nvdcf.yml`, `config/tracker_nvsort.yml`) sudah tersedia di repositori proyek.
Trade-off akurasi-vs-komputasi antara pendekatan *feature-based* (NvDCF) dan *motion-only*
(NvSORT) inilah yang mendasari rumusan masalah #3 — namun sesuai Batasan Masalah §1.5 poin 5,
penelitian ini **murni mengukur sisi efisiensi komputasi** (FPS, `Lat_Tracker_ms`, utilisasi
resource) dan tidak mengukur sisi kualitas asosiasi ID (MOTA/IDF1/ID switch).

Pembatasan ini bukan sekadar konsekuensi keterbatasan dataset, melainkan konsisten dengan
definisi sumbu perbandingan yang sudah melekat pada desain kedua profil tracker itu sendiri.
Dokumentasi resmi NVIDIA (*Gst-nvtracker* plugin manual, DeepStream SDK) menyatakan NvSORT
"tidak melibatkan pemrosesan data piksel sama sekali" sehingga "efisien secara komputasi",
sedangkan NvDCF memakai *visual tracker* berbasis *discriminative correlation filter* — yaitu
pemrosesan fitur piksel per objek per frame (NVIDIA, DeepStream SDK Plugin Manual, bagian
*Gst-nvtracker*). Postingan blog resmi NVIDIA Developer soal DeepStream SDK 6.2 juga secara
eksplisit menempatkan NvSORT sebagai *"lightweight, CPU-only implementation but still
competitively accurate"* dan NvDCF sebagai penghasil *"best accuracy and robustness"* lewat
kombinasi *conventional ML* (DCF) dan *deep learning* (ReID) (NVIDIA Developer Blog, 2023) —
mengonfirmasi bahwa sumbu akurasi-vs-komputasi ini memang didesain vendor sebagai *trade-off*
yang sudah diketahui karakteristiknya secara kualitatif, bukan sesuatu yang perlu diukur ulang
oleh penelitian ini untuk membuat pertanyaan "tracker mana yang lebih efisien secara
komputasi" valid dijawab.

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

> Verifikasi 2026-08-14: kedua sumber telah dibuka dan dibaca langsung, kutipan di atas
> dikonfirmasi akurat. Dokumentasi resmi *Gst-nvtracker* menyatakan NvSORT "is computationally
> efficient since it does not involve any pixel data processing", dan tabel perbandingan
> tracker pada dokumen yang sama mencantumkan NvDCF sebagai "Highly robust against partial
> occlusions... Easily adjust params for accuracy-perf tradeoff" versus NvSORT "Light weight...
> reasonable tracking accuracy". NVIDIA Developer Blog (diterbitkan 19 April 2023, penulis Paul
> Shin & Fangyu Li) memuat tabel yang secara eksplisit menyebut NvSORT sebagai "a lightweight,
> CPU-only implementation but still competitively accurate" dan NvDCF sebagai penghasil "the
> best accuracy and robustness by combining conventional machine learning (DCF) and deep
> learning (ReID)" — sesuai kutipan yang dipakai pada paragraf di atas. Format sitasi
> disesuaikan ke gaya APA yang dipakai kampus untuk sumber non-jurnal (organisasi/penulis
> individu + tahun + judul + URL), mengikuti contoh pada skripsi rujukan di
> `../referensi-skripsi/` (mis. pola "Badan Pangan Nasional. (2024). Judul. URL"):
>
> - NVIDIA. (t.t.). *Gst-nvtracker — DeepStream documentation*. Diakses 14 Agustus 2026, dari
>   https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_plugin_gst-nvtracker.html
> - Shin, P., & Li, F. (2023). *State-of-the-art real-time multi-object trackers with NVIDIA
>   DeepStream SDK 6.2*. NVIDIA Developer Blog.
>   https://developer.nvidia.com/blog/state-of-the-art-real-time-multi-object-trackers-with-nvidia-deepstream-sdk-6-2/
>
> Catatan: sumber kedua ternyata punya penulis individu tercantum (Paul Shin dan Fangyu Li),
> bukan sekadar "NVIDIA" generik — sitasi di `../journal/daftar-referensi.md` #23 diperbarui
> dari "NVIDIA (2023)" menjadi "Shin & Li (2023)" agar sesuai penulis asli.

### 2.2.7 Jetson Orin Nano sebagai Platform Edge Device

NVIDIA Jetson Orin Nano adalah *System-on-Chip* (SoC) embedded berbasis arsitektur GPU Ampere,
bagian dari keluarga Jetson Orin yang ditujukan untuk aplikasi *edge AI* dengan konsumsi daya
rendah. Berbeda dari varian Jetson Orin NX dan Jetson AGX Orin, Jetson Orin Nano **tidak
dilengkapi Deep Learning Accelerator (DLA)** — komponen akselerator khusus inferensi terpisah
dari GPU utama — sehingga strategi optimasi yang bergantung pada DLA (seperti pada proposal
awal penelitian ini yang menargetkan Jetson AGX Orin) tidak dapat diterapkan pada perangkat ini
(lihat Bab I §1.1 dan §1.4 Batasan Masalah, serta `../../docs/01_scope_and_architecture.md`
§1.4).

Unit yang dipakai pada penelitian ini adalah varian **Jetson Orin Nano 4GB** (bukan 8GB).
Identifikasi ini didasarkan pada mode daya maksimum yang terbaca lewat `nvpmodel -q` pada
perangkat, yaitu **10W** — nilai ini unik untuk SKU 4GB pada firmware standar (non-*Super*):
modul 8GB pada firmware yang sama hanya memiliki mode 7W/15W (tidak ada mode 10W), sesuai tabel
*Operating Requirements* pada *datasheet* resmi NVIDIA (NVIDIA, 2024). Karena mode maksimum
yang terbaca adalah 10W — bukan 25W — perangkat juga dipastikan berjalan pada konfigurasi daya
**default** (7W/10W), bukan mode `MAXN_SUPER` (JetPack 6.2+), sehingga performa GPU/CPU yang
menjadi acuan Bab IV mencerminkan batas atas mode default tersebut, bukan performa puncak SoC.
`[VERIFIKASI]` Identifikasi ini diturunkan dari pembacaan `nvpmodel`, bukan inspeksi label fisik
modul — penulis disarankan mengonfirmasi ulang dengan `cat /proc/device-tree/model` di
perangkat saat kembali tersedia, sebagai verifikasi independen.

**Tabel 2.X** Spesifikasi Jetson Orin Nano 4GB (NVIDIA, 2024)

| Komponen | Mode default (7W/10W) — dipakai penelitian ini | Mode `MAXN_SUPER` (tidak dipakai) |
|---|---|---|
| GPU | Ampere, 512 CUDA core, 16 Tensor core (generasi ke-3), hingga 625 MHz | hingga 1020 MHz |
| Performa AI (INT8) | 10 TOPS (*dense*) / 20 TOPS (*sparse*) | 17 TOPS (*dense*) / 34 TOPS (*sparse*) |
| CPU | 6-core Arm Cortex-A78AE (2 klaster: 4-core + 2-core), *system cache* 4MB, hingga 1,5 GHz/core | hingga 1,7 GHz/core |
| Memori | 4GB LPDDR5, bus 64-bit, *bandwidth* hingga 34 GB/s | hingga 51 GB/s |
| Mode daya tersedia | 7W / 10W / 25W (`MAXN_SUPER`) | — |
| DLA | Tidak tersedia | Tidak tersedia |

Sumber: NVIDIA (2024), tabel "AI Performance", "GPU Operation" (Tabel 2-1), "CPU Cluster",
"Memory Subsystem", dan "Operating Requirements".

### 2.2.8 Metrik Evaluasi

Penelitian ini menggunakan dua kelompok metrik evaluasi, mengikuti pembagian pada proposal
(§"Analisis dan Benchmarking") dan implementasi *tooling* yang sudah tersedia:

- **Kualitas deteksi**: *precision*, *recall*, dan *mean Average Precision* (mAP, pada IoU
  threshold 0.5 dan rentang 0.5:0.95) — dihitung menggunakan `pycocotools.cocoeval.COCOeval`
  (lihat `../../utils/eval_map/eval_deepstream_map.py`) untuk hasil *as-deployed* FP16, dan
  `Ultralytics YOLO.val()` untuk baseline FP32-proxy (`../../docs/05_accuracy_results.md`).
- **Performa runtime**: *throughput* (FPS) dan latensi *end-to-end* maupun latensi
  per-komponen pipeline (`Lat_PreMux_ms`, `Lat_Mux_ms`, `Lat_Infer_ms`, `Lat_Tracker_ms`,
  `Lat_PreOSD_ms`, `Lat_OSD_ms`, `Lat_Output_ms` — kolom pada `fps.csv`, lihat
  `../../docs/04_benchmark_protocol.md`), diukur dengan *pad probe* GStreamer pada tiap elemen
  pipeline agar tidak mengganggu jalur kritis yang sedang diukur (logger berjalan di thread
  terpisah via `GAsyncQueue`).
- **Utilisasi sumber daya**: penggunaan GPU (%), CPU per-core (%), RAM, dan konsumsi daya
  per-*rail*, diambil dari `tegrastats` dan diproses menjadi `hardware_analysis.csv` (lihat
  `../../docs/04_benchmark_protocol.md`).

Kriteria evaluasi lengkap (ambang throughput/latensi real-time, serta metrik spesifik untuk
masing-masing rumusan masalah) dirinci di `Bab-3-Metodologi-Penelitian.md` §3.6.

## 2.3 Kerangka Berpikir

Tinjauan literatur pada §2.1 menunjukkan tiga hal: (1) kombinasi model YOLO kelas nano/tiny
dengan DeepStream + TensorRT pada perangkat Jetson-class terbukti layak untuk deteksi objek
ADAS real-time (§2.1.1), namun studi pembanding antar-generasi YOLO terbaru (termasuk model
NMS-free seperti YOLO26n) pada Jetson Orin Nano spesifik masih terbatas; (2) akselerasi NMS
melalui eksekusi paralel GPU terbukti secara konsisten mempercepat *post-processing* pada
studi-studi sebelumnya (§2.1.2), namun studi tersebut memakai akselerator/kernel kustom,
sementara penelitian ini menguji pendekatan yang lebih umum diterapkan di industri — plugin
`EfficientNMS_TRT` bawaan vendor — sehingga hasil (termasuk potensi hasil negatif, lihat
`Bab-4-Hasil-dan-Pembahasan.md` §4.5) tetap merupakan kontribusi ilmiah yang sah karena menguji
klaim akselerasi pada kondisi implementasi yang lebih realistis untuk pengembang aplikasi
(dibanding menulis kernel CUDA dari nol); (3) tidak ditemukan studi yang membandingkan efisiensi
komputasi algoritma *tracker* (feature-based vs. motion-only) secara spesifik pada pipeline
DeepStream di perangkat Jetson Orin Nano — mengisi celah inilah yang menjadi kontribusi rumusan
masalah #3, sebagai pengganti rumusan masalah DLA pada proposal awal yang tidak dapat
dilaksanakan pada perangkat target akhir (lihat Bab I §1.1 dan §1.2).

Ketiga rumusan masalah pada Bab I — (1) kinerja real-time pipeline dasar, (2) efek EfficientNMS
terhadap efisiensi pipeline, (3) efek pemilihan algoritma tracker terhadap efisiensi komputasi —
dengan demikian saling melengkapi sebagai evaluasi bertahap terhadap satu pipeline DeepStream
yang sama: dari performa dasar, ke satu titik optimasi spesifik (NMS), ke satu variabel desain
lain yang memengaruhi *real-time budget* pipeline secara keseluruhan (tracker) — menjawab
kebutuhan literatur akan studi *end-to-end* pada perangkat edge kelas *entry-level* (Jetson
Orin Nano), bukan hanya perangkat kelas atas (Jetson AGX Orin) yang lebih umum diuji pada
literatur ADAS existing.

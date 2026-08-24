# BAB I PENDAHULUAN

## 1.1 Latar Belakang

Perkembangan sistem bantuan pengemudi atau *advanced driver assistance systems* (ADAS)
menunjukkan bahwa teknologi ini semakin diposisikan sebagai komponen penting dalam
peningkatan keselamatan berkendara. Temuan terbaru menunjukkan bahwa penerapan ADAS
berpotensi menurunkan angka kecelakaan berat serta mendukung agenda keselamatan jalan
raya secara lebih luas, khususnya ketika fitur-fitur bantu berkendara mulai diadopsi
secara lebih merata pada kendaraan modern (Costa et al., 2025). Sejalan dengan itu,
kajian terhadap ADAS juga menunjukkan bahwa pengguna cenderung menilai sistem ini mampu
meningkatkan rasa aman dan kenyamanan berkendara, meskipun masih dibutuhkan
penyempurnaan berkelanjutan agar manfaatnya benar-benar optimal dalam konteks keselamatan
(Neumann, 2024). Kondisi tersebut menegaskan bahwa kebutuhan akan sistem bantuan
pengemudi yang andal bukan lagi sekadar pilihan teknologi, melainkan kebutuhan nyata
dalam upaya menekan risiko kecelakaan di jalan.

Dalam konteks kendaraan cerdas, kemampuan sistem untuk memahami lingkungan secara cepat
dan akurat menjadi prasyarat utama. ADAS sangat bergantung pada pemrosesan video dan
citra secara terus-menerus agar kendaraan dapat mengenali objek penting, membaca situasi
lalu lintas, serta merespons potensi bahaya secara tepat waktu. Studi terkini menekankan
bahwa penguatan ADAS melalui *computer vision* dan pendekatan *machine learning* memang
meningkatkan kapabilitas deteksi, tetapi sekaligus menuntut proses komputasi yang lebih
besar dan berkelanjutan (Shah et al., 2025). Pada saat yang sama, implementasi berbasis
perangkat *embedded edge* untuk deteksi objek dan pembentukan informasi lingkungan
menunjukkan bahwa pengolahan data visual secara *real-time* merupakan aspek mendasar
dalam sistem kendaraan yang aman dan responsif (Choi et al., 2024). Oleh karena itu,
*real-time processing* bukan hanya karakteristik teknis, melainkan inti dari keandalan
sistem keselamatan pada kendaraan cerdas.

Persoalan utama muncul ketika kebutuhan pemrosesan *real-time* berhadapan dengan tuntutan
latensi yang sangat rendah. Pada sistem berbasis *edge*, keterlambatan sekecil apa pun
dapat mengurangi kualitas keputusan dan menurunkan efektivitas peringatan dini, terutama
ketika beban inferensi berjalan secara dinamis di lingkungan jaringan yang berubah-ubah.
Kajian mengenai *inference serving* dengan *end-to-end latency SLOs* menunjukkan bahwa
ketepatan waktu penyajian inferensi sama pentingnya dengan akurasi model, khususnya pada
jaringan *edge* yang bersifat dinamis (Nigade et al., 2024). Di sisi lain, penelitian
tentang penempatan server *edge* menegaskan bahwa pengaturan sumber daya yang tidak tepat
dapat memperbesar latensi transmisi dan memperberat beban kerja sistem, sehingga
mengganggu pencapaian kinerja waktu nyata yang dibutuhkan aplikasi kritis (Zhang et al.,
2024). Dengan demikian, tantangan utama pada ADAS bukan hanya menghasilkan keluaran yang
benar, tetapi juga memastikan keluaran tersebut hadir dalam batas waktu yang aman untuk
pengambilan keputusan berkendara.

Tantangan tersebut menjadi semakin kompleks ketika sistem dijalankan pada perangkat
*embedded* dengan sumber daya terbatas. Optimasi *deployment* pada platform *embedded
heterogen* diperlukan agar kebutuhan *frame rate*, akurasi, dan efisiensi energi tetap
terpenuhi (Ruiz-Barroso et al., 2025). Platform seperti **Jetson Orin Nano** menawarkan
kemampuan komputasi yang memadai untuk kelas perangkat *edge* berbiaya rendah, namun
tetap berada dalam batasan konsumsi daya, suhu operasi, dan kapasitas pemrosesan yang
jauh lebih ketat dibandingkan platform *edge* kelas atas (mis. Jetson AGX Orin) maupun
server kelas pusat. Kajian terhadap perangkat *embedded* menunjukkan bahwa pemrosesan
video *real-time* selalu menuntut kompromi antara kecepatan, efisiensi energi, dan
stabilitas kinerja, terutama ketika sistem harus mempertahankan luaran yang konsisten pada
beban kerja yang berubah-ubah (Suder et al., 2023). Dalam konteks aplikasi keselamatan,
kompromi semacam ini menjadi sangat penting karena keterbatasan sumber daya dapat
langsung berpengaruh pada keterlambatan deteksi, penurunan laju pemrosesan, dan
berkurangnya kemampuan sistem dalam mempertahankan performa secara berkelanjutan.

Selain beban komputasi, sistem ADAS juga menuntut karakteristik *deterministic* dan
stabil karena bersifat *safety-critical*. Setiap proses harus selesai sesuai tenggat
waktu agar fungsi peringatan, deteksi, dan respons dapat berjalan secara konsisten di
bawah kondisi operasional yang berubah cepat. Studi terbaru menunjukkan bahwa penjadwalan
tugas pada ADAS memiliki tingkat kepekaan yang tinggi terhadap *deadline*, dan kegagalan
memenuhi batas waktu dapat berimplikasi pada kegagalan sistem secara fungsional
(Seyfipoor et al., 2026). Hal ini menegaskan bahwa aplikasi keselamatan memerlukan
arsitektur yang tidak hanya cepat, tetapi juga stabil, terprediksi, dan efisien dalam
mengelola prioritas pemrosesan. Dengan kata lain, keandalan ADAS tidak dapat dilepaskan
dari kemampuan sistem untuk menjaga konsistensi waktu proses dalam kondisi beban tinggi,
baik beban tersebut berasal dari tahap deteksi objek (inferensi model dan *post-processing*
NMS-nya) maupun dari tahap asosiasi objek antar-frame (*tracking*) yang menjaga identitas
objek tetap konsisten sepanjang waktu.

Berdasarkan uraian tersebut, terlihat bahwa penelitian mengenai optimasi *real-time
pipeline* pada platform *embedded* menjadi relevan dan mendesak untuk dilakukan,
khususnya pada konteks ADAS berbasis Jetson Orin Nano. Permasalahan yang dihadapi bukan
semata-mata pada kemampuan mendeteksi objek, melainkan pada bagaimana sistem dapat
mempertahankan kecepatan, kestabilan, dan efisiensi pemrosesan secara simultan dalam
skenario operasi yang menuntut respons segera, pada setiap tahap pipeline, mulai dari
inferensi deteksi, *post-processing* (NMS), hingga *tracking*. Oleh karena itu, penelitian
ini diarahkan untuk memahami dan menganalisis faktor-faktor yang memengaruhi kinerja
*real-time pipeline* agar sistem ADAS dapat memenuhi kebutuhan keselamatan berkendara
secara lebih andal.

## 1.2 Landasan Teori

### 1.2.1 ADAS dan Perception Layer pada Kendaraan Cerdas

*Advanced Driver Assistance Systems* (ADAS) adalah kumpulan teknologi sistem elektronik
terintegrasi yang dirancang untuk membantu pengemudi dalam proses berkendara dan
memarkir kendaraan guna meningkatkan keselamatan dan kenyamanan. Sistem ADAS modern
sangat bergantung pada pemahaman lingkungan sekitar secara *real-time* untuk mendeteksi
potensi bahaya, seperti keberadaan kendaraan lain, pejalan kaki, atau infrastruktur jalan
(Neumann, 2024).

Secara arsitektural, sistem ADAS terdiri dari tiga lapisan utama: *perception layer*
(lapisan persepsi), *planning layer* (lapisan perencanaan), dan *control layer* (lapisan
kendali) (Shah et al., 2025). Penelitian ini secara khusus berfokus pada *perception
layer*, yaitu tahapan di mana data mentah dari sensor (dalam hal ini, citra video dari
kamera) diekstraksi menjadi informasi semantik berwujud deteksi dan pelacakan objek. Pada
lapisan ini, keandalan sistem tidak hanya diukur dari seberapa akurat objek dikenali,
melainkan juga seberapa cepat informasi tersebut dapat disajikan ke lapisan perencanaan,
karena keterlambatan sekecil apapun dapat mengurangi waktu respons pengereman darurat
atau peringatan dini.

### 1.2.2 Konsep Edge Computing dan Analisis Video Real-Time

Pemrosesan *perception layer* pada kendaraan otonom dan ADAS umumnya diimplementasikan
menggunakan paradigma *edge computing*, di mana komputasi data dilakukan secara lokal di
perangkat (*on-device*) yang berada sedekat mungkin dengan sumber data sensor (Choi et
al., 2024). Pendekatan ini meniadakan ketergantungan pada koneksi jaringan *cloud* yang
memiliki latensi transmisi dinamis dan rentan terputus (Zhang et al., 2024).

Namun, implementasi Edge AI memiliki batasan mendasar terkait anggaran sumber daya
(*resource constraints*). Perangkat *embedded edge* (seperti keluarga NVIDIA Jetson)
beroperasi dengan ketersediaan memori, kapabilitas komputasi (CPU/GPU), dan konsumsi
daya (*thermal design power*) yang sangat terbatas dibandingkan server pusat (Suder et
al., 2023). Oleh karena itu, *real-time video analytics* pada *edge device* selalu
menuntut kompromi yang optimal antara akurasi model, efisiensi energi, dan kecepatan
pemrosesan. Kinerja *real-time* dalam konteks ini didefinisikan sebagai kemampuan sistem
dalam mengeksekusi *pipeline* secara deterministik guna memenuhi tenggat waktu
(*deadline*) laju bingkai (misalnya 30 FPS) secara terus-menerus tanpa penumpukan
antrean pemrosesan (Seyfipoor et al., 2026).

### 1.2.3 NVIDIA DeepStream SDK dan Pipeline Inferensi

NVIDIA DeepStream adalah *Software Development Kit* (SDK) yang dibangun di atas kerangka
kerja multimedia GStreamer, dirancang khusus untuk membangun *pipeline* analitik video
bertenaga kecerdasan buatan (AI) secara *real-time* dengan memanfaatkan akselerasi
perangkat keras NVIDIA.

Karakteristik kunci DeepStream yang membedakannya dari *pipeline* konvensional (seperti
integrasi manual berbasis OpenCV) adalah penggunaan arsitektur memori *zero-copy*
berbasis NVIDIA Memory Manager (NVMM). Pada *pipeline* standar, *frame* gambar sering
disalin secara berulang antara memori CPU (*host*) dan GPU (*device*) pada setiap
tahapan (*decode*, *pre-processing*, inferensi, *post-processing*). NVMM mengeliminasi
hambatan (*bottleneck*) ini dengan menahan *buffer frame* video secara persisten di dalam
memori GPU dari awal hingga akhir *pipeline*.

### 1.2.4 Deteksi Objek Real-Time dan Arsitektur YOLO

*You Only Look Once* (YOLO) merupakan keluarga arsitektur jaringan saraf tiruan untuk
deteksi objek berbasis pendekatan *single-stage detector*. Berbeda dengan metode
*two-stage* (seperti Faster R-CNN) yang memerlukan tahapan proposal *region* sebelum
klasifikasi, YOLO memprediksi koordinat *bounding box* dan probabilitas kelas secara
bersamaan dalam satu kali eksekusi (*forward pass*) jaringan penuh. Karakteristik ini
membuat YOLO sangat ideal untuk pemrosesan *real-time* di lingkungan ADAS (Ayachi et
al., 2025; Dhatrika et al., 2025).

Penelitian ini mengevaluasi berbagai generasi kelas ringan (*nano/tiny*) dari YOLO,
seperti YOLOv8, YOLOv9, YOLOv10, dan YOLO26. Perbedaan struktural penting di antara
generasi ini terletak pada strategi *post-processing*. Generasi awal hingga v9
menghasilkan banyak prediksi *bounding box* redundan di sekitar objek tunggal, sehingga
mewajibkan penggunaan filter *Non-Maximum Suppression* (NMS). Sebaliknya, arsitektur
yang lebih modern (seperti YOLOv10n dan YOLO26n) telah mengintegrasikan metode optimasi
struktural (*consistent dual assignments*) selama pelatihan, menjadikannya model
*NMS-free* yang tidak lagi memerlukan *post-processing* heuristik eksternal, sehingga
berpotensi menurunkan latensi inferensi.

### 1.2.5 Post-Processing Deteksi: Bounding Box, IoU, dan NMS

Keluaran mentah dari arsitektur deteksi objek standar (non-*NMS-free*) terdiri dari
sekumpulan *bounding box* kandidat. Setiap kandidat direpresentasikan oleh parameter
koordinat (x, y, w, h) dan sebuah nilai keyakinan (*confidence score*) yang menyatakan
probabilitas keberadaan objek berserta kelasnya.

Untuk mengevaluasi tingkat tumpang-tindih antar kandidat *bounding box*, digunakan metrik
*Intersection over Union* (IoU), yang diformulasikan sebagai rasio luas irisan terhadap
luas gabungan dari dua *bounding box* (A dan B):

```
IoU = Area(A ∩ B) / Area(A ∪ B)
```

*Non-Maximum Suppression* (NMS) adalah algoritma *post-processing* yang menyaring kandidat
berlebih tersebut. Proses NMS standar mengurutkan seluruh *bounding box* berdasarkan
*confidence score* tertinggi, mempertahankan *bounding box* dengan skor tertinggi, lalu
menghapus *bounding box* lain yang memiliki nilai IoU dengan *bounding box* tertinggi
melampaui suatu nilai ambang batas (*threshold*) tertentu (misalnya IoU > 0,45).

Implementasi NMS tradisional (seperti *GreedyNMS*) bersifat sekuensial dan umumnya
dieksekusi oleh CPU (*host*), yang seringkali menjadi hambatan latensi pada pemrosesan
*edge* akibat proses perpindahan data dan interupsi siklus (Chen et al., 2022; Oro et
al., 2022). Untuk menanggulangi hal ini, *plugin* `EfficientNMS_TRT` mengintegrasikan
tahapan NMS secara langsung ke dalam graf komputasi TensorRT. Pendekatan ini
memungkinkan perbandingan IoU dan penyaringan objek dilakukan secara paralel penuh
(*fully parallelized*) di dalam *kernel* GPU, menghilangkan *bottleneck* transfer memori
ke CPU dan mempercepat penyajian inferensi (Yang et al., 2025).

### 1.2.6 Konsep Dasar Multi-Object Tracking (MOT)

Deteksi objek berbasis *frame* tunggal rentan terhadap ketidakstabilan visual, seperti
*flicker* akibat oklusi sementara, perubahan pencahayaan, atau kegagalan detektor secara
sesaat. *Multi-Object Tracking* (MOT) mengatasi masalah ini dengan mengasosiasikan
deteksi pada *frame* saat ini dengan lintasan historis objek dari *frame-frame*
sebelumnya, memberikan identitas unik (ID) yang konsisten.

Algoritma MOT paling efisien berakar pada paradigma *Tracking-by-Detection* yang
utamanya menggunakan dua komponen matematis:

1. **Kalman Filter**: sebuah algoritma estimasi rekursif yang memprediksi keadaan
   kinematik objek di masa depan (posisi dan kecepatan) berdasarkan pengukuran beruntun
   yang mengandung derau (*noise*).
2. **Hungarian Algorithm**: algoritma optimasi kombinatorial untuk memecahkan masalah
   penugasan (*assignment problem*). Dalam MOT, algoritma ini digunakan untuk
   mencocokkan (*bipartite matching*) prediksi lokasi Kalman Filter dengan lokasi
   *bounding box* deteksi baru, biasanya menggunakan matriks jarak berbasis IoU atau
   jarak Euclidean.

### 1.2.7 Algoritma Tracking pada DeepStream

DeepStream menyediakan elemen `nvtracker` yang mendukung berbagai profil pelacakan.
Penelitian ini mengevaluasi dua algoritma pelacakan dengan pendekatan desain yang
mewakili kedua ujung spektrum beban komputasi:

- **NvSORT** (*Simple Online and Realtime Tracking*): pelacak berbasis *motion-only*
  murni yang hanya mengandalkan Kalman Filter dan Hungarian Algorithm pada properti
  koordinat spasial tanpa mengekstraksi atau memproses data piksel gambar sama sekali.
  Hal ini membuatnya beroperasi sangat cepat dan efisien terhadap utilisasi CPU/GPU,
  dengan kompromi berkurangnya kemampuan membedakan objek saat terjadi oklusi silang
  yang kompleks (Shin & Li, 2023).
- **NvDCF** (*Discriminative Correlation Filter*): pelacak hibrida *feature-based* yang
  memelajari ciri visual khusus (*learned visual features*) dari setiap target secara
  *real-time*. NvDCF memproses area piksel dalam *bounding box* untuk membuat filter
  korelasi, yang memungkinkannya melacak objek saat detektor gagal atau tertutup
  sebagian secara tangguh. Konsekuensinya, NvDCF membutuhkan beban komputasi yang jauh
  lebih berat karena mengeksekusi ekstraksi fitur pada setiap objek di setiap *frame*.

### 1.2.8 Akselerasi Inferensi: TensorRT dan Kuantisasi Presisi

NVIDIA TensorRT adalah *runtime* optimasi *deep learning* yang mengkompilasi model
jaringan saraf (seperti arsitektur ONNX) menjadi *engine* yang dieksekusi secara
spesifik untuk mikroarsitektur perangkat keras target (GPU Ampere). TensorRT melakukan
fusi lapisan (*layer fusion*), kalibrasi presisi, dan pemilihan *kernel* (*kernel
auto-tuning*).

Kuantisasi adalah teknik mereduksi kedalaman *bit* yang digunakan untuk merepresentasikan
bobot dan aktivasi jaringan. Sementara pelatihan model konvensional dilakukan dalam
presisi *Floating-Point* 32-*bit* (FP32), inferensi pada perangkat *edge* dioptimalkan
menggunakan presisi lebih rendah. Presisi *Floating-Point* 16-*bit* (FP16) dapat
mempercepat kalkulasi berkat penggunaan unit *Tensor Core* pada arsitektur GPU dengan
penurunan akurasi yang marjinal.

### 1.2.9 Metrik Evaluasi Kinerja Real-Time Pipeline

Evaluasi kinerja *pipeline* pada komputasi lokal *edge* didasarkan pada efisiensi
komputasi, kelancaran eksekusi, serta mempertahankan akurasi dasar (Nigade et al.,
2024). Metrik yang digunakan meliputi:

**A. Throughput (FPS)**

Laju bingkai per detik (*Frames Per Second*) mengukur agregat volume pemrosesan.
Diformulasikan secara konseptual dari total latensi *end-to-end* rerata sistem per
*frame*:

```
FPS = 1 / (rerata t_e2e dalam detik)
```

di mana sistem dikategorikan *real-time* jika nilai laju pemrosesan stabil pada target
aplikasi kritis (misalnya minimum 30 FPS).

**B. Latensi dan Persentil (P95) Latensi**

Latensi adalah durasi tempuh pemrosesan. Pada penelitian ini, dievaluasi latensi
*end-to-end* yang diukur dari titik bingkai masuk pada tahap *multiplexing*
aliran video hingga keluar di blok antarmuka layar, dan dirincikan secara per-komponen
(mis. tahap inferensi model dan tahap *tracking* objek). Dalam sistem
*safety-critical* seperti ADAS, rerata (*mean*) rentan
menyembunyikan efek variabilitas ekstrem (*jitter*). Oleh karena itu, metrik menggunakan
persentil ke-95 (P95) yang mengindikasikan bahwa 95% dari seluruh siklus *frame*
diselesaikan dalam durasi kurang dari atau sama dengan nilai latensi tersebut.

**C. Akurasi Deteksi**

Untuk memastikan optimasi presisi komputasi (FP16) dan implementasi NMS tidak merusak
fungsionalitas pengenalan (*sanity-check*), akurasi diukur berdasar matriks kebingungan
(*confusion matrix*):

- **Precision (P)**: proporsi deteksi yang relevan (benar): `P = TP / (TP + FP)`
- **Recall (R)**: proporsi target asli yang berhasil ditemukan: `R = TP / (TP + FN)`
- **mean Average Precision (mAP)**: luas area di bawah kurva *Precision-Recall*
  rerata untuk seluruh kelas objek, dihitung melintasi batas ambang batas (IoU = 0,5
  untuk mAP50, dan rentang IoU = 0,5–0,95 untuk mAP50-95).

## 1.3 Rumusan Masalah

Berdasarkan latar belakang penelitian, rumusan masalah dalam penelitian ini adalah
sebagai berikut:

1. Bagaimana kinerja *real-time pipeline* Nvidia DeepStream pada proses deteksi
   kendaraan di Jetson Orin Nano?
2. Sejauh mana penerapan NMS paralel berbasis TensorRT plugin (EfficientNMS) dapat
   meningkatkan efisiensi *pipeline* deteksi kendaraan dibandingkan konfigurasi standar?
3. Sejauh mana pemilihan algoritma *tracking* (NvDCF vs. NvSORT) memengaruhi efisiensi
   komputasi *real-time pipeline* dibandingkan konfigurasi *baseline*?

## 1.4 Tujuan

Penelitian ini memiliki tujuan antara lain:

1. Menganalisis kinerja *real-time pipeline* Nvidia DeepStream pada deteksi kendaraan
   di Jetson Orin Nano.
2. Mengevaluasi pengaruh optimasi *pipeline*, khususnya penerapan NMS paralel berbasis
   TensorRT plugin (EfficientNMS), terhadap peningkatan performa pemrosesan *real-time*.
3. Mengevaluasi pengaruh pemilihan algoritma *tracking* terhadap efisiensi komputasi
   *real-time pipeline* dibandingkan konfigurasi *baseline*.

## 1.5 Manfaat

1. **Manfaat Teoritis**: Menjadi referensi ilmiah mengenai optimasi *pipeline*
   pemrosesan video *real-time* pada sistem *edge* untuk aplikasi ADAS, khususnya dalam
   penerapan Nvidia DeepStream, NMS paralel berbasis TensorRT plugin (EfficientNMS),
   serta evaluasi efisiensi komputasi pemilihan algoritma *tracking* pada perangkat
   Jetson Orin Nano.
2. **Manfaat Praktis**: Memberikan pedoman teknis bagi pengembang sistem *embedded* dan
   ADAS mengenai konfigurasi *pipeline* yang paling optimal untuk mencapai kinerja
   *real-time*, terutama dalam aspek latensi, *throughput*, dan stabilitas pemrosesan
   pada perangkat Jetson Orin Nano.
3. **Manfaat Institusional**: Berkontribusi dalam pengembangan riset terapan di bidang
   Visi Komputer, *Edge* AI, dan *Embedded Systems*, serta memperkuat kapasitas institusi
   dalam kajian pemanfaatan teknologi akselerasi perangkat keras NVIDIA pada sistem
   cerdas berbasis video.

## 1.6 Ruang Lingkup Penelitian

Agar penelitian lebih terarah dan fokus, maka batasan penelitian ini adalah sebagai
berikut:

1. Pengujian kinerja perangkat keras difokuskan hanya pada satu perangkat edge computing
   berupa NVIDIA Jetson Orin Nano varian memori 4 gigabyte, dan tidak mencakup perangkat
   kelas lebih tinggi berikut modul akselerator perangkat keras khusus yang menyertainya.
2. Peningkatan kinerja dianalisis hanya melalui perbandingan pipeline sebelum dan sesudah
   dua bentuk optimasi, berupa penerapan mekanisme Non-Maximum Suppression paralel
   berbasis plugin TensorRT dan penggantian algoritma pelacakan objek antara profil
   NvDCF dan profil NvSORT.
3. Penelitian menggunakan model deteksi yang telah dilatih sebelumnya sehingga tidak
   mencakup proses pelatihan model dari awal maupun peningkatan akurasi arsitektural
   model deteksi.
4. Penelitian hanya membahas pipeline perangkat lunak untuk pemrosesan video dan tidak
   mencakup integrasi dengan modul kendali kendaraan maupun subsistem bantuan pengemudi
   lain di luar komponen persepsi visual.
5. Evaluasi algoritma pelacakan objek dibatasi pada aspek efisiensi komputasi berupa laju
   bingkai per detik, latensi komponen pelacak, dan utilisasi sumber daya perangkat
   keras, dan tidak mencakup evaluasi kualitas maupun akurasi luaran pelacakan, karena
   penelitian ini tidak memiliki akses pada kumpulan data pelacakan objek berlabel dengan
   identitas objek yang konsisten antar-bingkai.
6. Seluruh pengujian dijalankan pada satu tingkat presisi numerik tunggal berupa setengah
   presisi (Half-Precision Floating-Point, FP16), dan tidak membandingkan konfigurasi
   presisi penuh maupun kuantisasi bilangan bulat 8-bit sebagai konfigurasi produksi
   alternatif.
7. Kamera stereo yang digunakan sebagai sumber video hanya dimanfaatkan sebagai sumber
   aliran video dua dimensi, sehingga kemampuan estimasi jarak berbasis stereo pada
   kamera tersebut tidak dimanfaatkan dan tidak menjadi bagian dari penelitian ini.

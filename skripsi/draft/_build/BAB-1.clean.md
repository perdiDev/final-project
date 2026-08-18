# BAB I — PENDAHULUAN

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
stabilitas kinerja, terutama ketika sistem harus mempertahankan hasil yang konsisten pada
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
dari kemampuan sistem untuk menjaga konsistensi waktu proses dalam kondisi beban tinggi —
baik beban tersebut berasal dari tahap deteksi objek (inferensi model dan *post-processing*
NMS-nya) maupun dari tahap asosiasi objek antar-frame (*tracking*) yang menjaga identitas
objek tetap konsisten sepanjang waktu.

Berdasarkan uraian tersebut, terlihat bahwa penelitian mengenai optimasi *real-time
pipeline* pada platform *embedded* menjadi relevan dan mendesak untuk dilakukan,
khususnya pada konteks ADAS berbasis Jetson Orin Nano. Permasalahan yang dihadapi bukan
semata-mata pada kemampuan mendeteksi objek, melainkan pada bagaimana sistem dapat
mempertahankan kecepatan, kestabilan, dan efisiensi pemrosesan secara simultan dalam
skenario operasi yang menuntut respons segera — pada setiap tahap pipeline, mulai dari
inferensi deteksi, *post-processing* (NMS), hingga *tracking*. Oleh karena itu, penelitian
ini diarahkan untuk memahami dan menganalisis faktor-faktor yang memengaruhi kinerja
*real-time pipeline* agar sistem ADAS dapat memenuhi kebutuhan keselamatan berkendara
secara lebih andal.

## 1.2 Rumusan Masalah

Berdasarkan latar belakang penelitian, rumusan masalah dalam penelitian ini adalah
sebagai berikut:

1. Bagaimana kinerja *real-time pipeline* Nvidia DeepStream pada proses deteksi
   kendaraan di Jetson Orin Nano?
2. Sejauh mana penerapan NMS paralel berbasis TensorRT plugin (EfficientNMS) dapat
   meningkatkan efisiensi *pipeline* deteksi kendaraan dibandingkan konfigurasi standar?
3. Sejauh mana pemilihan algoritma *tracking* (NvDCF vs. NvSORT) memengaruhi efisiensi
   komputasi *real-time pipeline* dibandingkan konfigurasi *baseline*?

## 1.3 Tujuan Penelitian

Adapun tujuan dari penelitian ini adalah sebagai berikut:

1. Menganalisis kinerja *real-time pipeline* Nvidia DeepStream pada deteksi kendaraan
   di Jetson Orin Nano.
2. Mengevaluasi pengaruh optimasi *pipeline*, khususnya penerapan NMS paralel berbasis
   TensorRT plugin (EfficientNMS), terhadap peningkatan performa pemrosesan *real-time*.
3. Mengevaluasi pengaruh pemilihan algoritma *tracking* terhadap efisiensi komputasi
   *real-time pipeline* dibandingkan konfigurasi *baseline*.

## 1.4 Manfaat Penelitian

1. **Manfaat Teoritis** — Menjadi referensi ilmiah mengenai optimasi *pipeline*
   pemrosesan video *real-time* pada sistem *edge* untuk aplikasi ADAS, khususnya dalam
   penerapan Nvidia DeepStream, NMS paralel berbasis TensorRT plugin (EfficientNMS),
   serta evaluasi efisiensi komputasi pemilihan algoritma *tracking* pada perangkat
   Jetson Orin Nano.
2. **Manfaat Praktis** — Memberikan pedoman teknis bagi pengembang sistem *embedded* dan
   ADAS mengenai konfigurasi *pipeline* yang paling optimal untuk mencapai kinerja
   *real-time*, terutama dalam aspek latensi, *throughput*, dan stabilitas pemrosesan
   pada perangkat Jetson Orin Nano.
3. **Manfaat Institusional** — Berkontribusi dalam pengembangan riset terapan di bidang
   Visi Komputer, *Edge* AI, dan *Embedded Systems*, serta memperkuat kapasitas institusi
   dalam kajian pemanfaatan teknologi akselerasi perangkat keras NVIDIA pada sistem
   cerdas berbasis video.

## 1.5 Batasan Masalah

Agar penelitian lebih terarah dan fokus, maka batasan penelitian ini adalah sebagai
berikut:

1. Pengujian penelitian dilakukan hanya pada perangkat **Jetson Orin Nano** sebagai
   platform utama eksperimen.
2. Peningkatan performa dianalisis hanya berdasarkan perbandingan *pipeline* sebelum dan
   sesudah optimasi menggunakan NMS paralel berbasis TensorRT plugin (EfficientNMS) dan
   pemilihan algoritma *tracking* (NvDCF vs. NvSORT).
3. Penelitian menggunakan model *pre-trained* sehingga tidak berfokus pada proses
   pelatihan maupun peningkatan akurasi model deteksi.
4. Penelitian hanya membahas *pipeline* pemrosesan video dan tidak mencakup integrasi
   dengan sistem kendaraan secara keseluruhan.
5. Evaluasi algoritma *tracking* dibatasi pada aspek **efisiensi komputasi** (FPS,
   latensi per-komponen, utilisasi *resource*), dan **tidak mencakup evaluasi kualitas
   atau akurasi *tracking*** (mis. jumlah *ID switch*, MOTA/IDF1). Pembatasan ini konsisten
   dengan rumusan masalah ketiga yang memang dirumuskan secara eksplisit sebagai
   pertanyaan efisiensi komputasi (§1.2), sejalan dengan sumbu akurasi-vs-komputasi yang
   sudah melekat pada desain profil NvDCF dan NvSORT itu sendiri menurut dokumentasi resmi
   NVIDIA (lihat Bab II §2.2.6) — bukan sesuatu yang perlu diukur ulang penelitian ini agar
   pertanyaannya valid dijawab. Ketidaktersediaan dataset *tracking* berlabel (video
   berurutan dengan ID objek konsisten, mis. KITTI Tracking) pada ruang lingkup penelitian
   ini menjadi alasan pendukung tambahan.

## 1.6 Sistematika Penulisan

Bagian ini akan disusun setelah seluruh isi Bab I–V penelitian ini selesai disusun, agar
ringkasan yang diberikan mencerminkan struktur akhir laporan secara akurat.

# BAB I — PENDAHULUAN

> Status: draf pertama. Diadaptasi dari `../Proposal/Proposal Final Perdi - AGX Orin
> ADAS-1.pdf` (versi yang sudah disetujui di seminar proposal), dengan penyesuaian akibat
> perubahan perangkat **Jetson AGX Orin → Jetson Orin Nano** dan penggantian rumusan
> masalah #3 (DLA → perbandingan algoritma tracking), sesuai keputusan yang diambil
> 2026-08-14 (lihat `../log/log-perubahan.md`). Field bertanda `[VERIFIKASI]` perlu dicek
> ulang oleh penulis sebelum dianggap final — lihat catatan di bawah masing-masing bagian.
>
> **Update 2026-08-19 (restrukturisasi format Unhas):** §1.1 Latar Belakang kini memuat
> ringkasan *state of the art* (klaster deteksi objek YOLO pada *edge* dan klaster akselerasi
> NMS) yang sebelumnya berada di `BAB-2-Tinjauan-Pustaka.md` §2.1 (draf lama, sudah dihapus) —
> dipindahkan ke sini karena BAB II sekarang wajib berupa Metode Penelitian (bukan Tinjauan
> Pustaka), lihat `BAB-2-Metode-Penelitian.md`. Rujukan silang "Bab II §2.2.x" pada beberapa
> bagian di bawah sudah diperbarui mengikuti struktur BAB II baru.

## Judul (revisi)

**Analisis Optimasi Real-Time Pipeline Nvidia DeepStream untuk Aplikasi ADAS Berbasis
Edge Device**

> Judul asli proposal: "...Berbasis Jetson AGX Orin". Judul ini **sudah disetujui dosen
> pembimbing** untuk diganti menjadi bentuk generik "...Berbasis Edge Device" (bukan
> "Jetson Orin Nano" — sempat ditulis begitu di draf sebelumnya, sudah dikoreksi
> 2026-08-14). Perubahan judul ke istilah generik ini sejalan dengan catatan revisi dosen
> pembimbing di seminar proposal ("Ditambahkan perangkat uji alternatif selain AGX
> Orin... implementasi pada perangkat dengan spesifikasi yang lebih rendah"). Kata sambung
> "untuk" (bukan "pada") sudah dikonfirmasi penulis 2026-08-14 — judul di atas final. Perlu
> dicatat: judul memakai istilah generik "Edge Device", tapi isi Bab I-IV tetap menyebut
> **Jetson Orin Nano** secara eksplisit sebagai perangkat uji konkret yang dipakai (judul
> generik, implementasi spesifik).

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

Kebutuhan akan pemrosesan objek *real-time* pada perangkat *edge* kelas ADAS ini juga
didukung oleh sejumlah penelitian terapan terbaru yang secara khusus menguji keluarga model
YOLO pada perangkat *embedded*. Ayachi dkk. (2025) mengevaluasi model YOLO v1 hingga v9 pada
dataset BDD100K dan menyimpulkan YOLOv9 memberi keseimbangan optimal antara kecepatan (34 FPS)
dan akurasi (85,54% mAP) untuk aplikasi ADAS *real-time*, sementara Dhatrika dkk. (2025)
mengimplementasikan YOLOv9 pada NVIDIA Jetson Nano dengan optimasi TensorRT dan kerangka kerja
DeepStream, mencapai mAP tinggi (91,9%) yang mengungguli model YOLOv5/YOLOv8 versi lebih lama —
kombinasi TensorRT + DeepStream pada keluarga Jetson Nano/Orin Nano inilah yang menjadi rujukan
metodologis paling dekat dengan pendekatan penelitian ini. Chaman dkk. (2025) menunjukkan
YOLOv11 pada Jetson Nano dan Raspberry Pi 5 mampu mencapai mAP@50 98,1% untuk deteksi kendaraan
modern, Guerrouj dkk. (2025) membuktikan kuantisasi *post-training* INT8 pada YOLOv4 dapat
menaikkan kecepatan Jetson Nano dari <2 FPS menjadi 5 FPS dengan kehilangan akurasi minimal,
Bouazizi dkk. (2024) melatih ulang SSD-MobileNet untuk objek jalan raya dengan *recall* tinggi
(0,873) demi keselamatan deteksi pejalan kaki, Xie dkk. (2024) membuktikan efektivitas
**DeepStream SDK** (SDK yang sama dipakai penelitian ini) untuk deteksi objek *real-time* di
luar domain otomotif, dan Tsai & Hsieh (2025) menemukan YOLOv8n mencapai kecepatan inferensi
112 FPS dengan ukuran model hanya 4,5 MB pada sistem peringatan tabrakan kendaraan otonom.
Namun demikian, sebagian besar studi ini menguji satu generasi YOLO secara terisolasi atau
membandingkannya pada dataset umum (BDD100K, MS COCO, dataset kustom) — celah yang belum
banyak dijawab adalah perbandingan *head-to-head* beberapa generasi YOLO kelas *nano/tiny*
terbaru (termasuk arsitektur *NMS-free*) secara konsisten pada satu platform *edge* dan satu
dataset otomotif yang sama, yang menjadi salah satu fokus rumusan masalah #1 pada penelitian
ini (§1.2).

Selain sisi deteksi, tahap *post-processing* Non-Maximum Suppression (NMS) juga menjadi
sorotan riset akselerasi *edge* tersendiri, karena implementasi NMS standar yang sekuensial dan
sering dieksekusi di CPU berpotensi menjadi *bottleneck* pipeline inferensi *real-time*. Chen
dkk. (2022) memperkenalkan ShapoolNMS, akselerator perangkat keras skalabel yang mencapai
percepatan 305×–3626× dibandingkan *GreedyNMS* perangkat lunak; Oro dkk. (2022) mengembangkan
kernel CUDA skalabel berbasis matriks *adjacency boolean* untuk NMS paralel pada GPU tertanam
(Tegra X1/X2), mencapai percepatan 14×–40× dibanding metode berbasis CNN; dan Yang dkk. (2025)
mengusulkan akselerator perangkat keras *post-processing* yang mencapai percepatan 19,89× pada
tahap inferensi dan 7,55× pada tugas NMS dibanding sistem GPU tradisional. Ketiga penelitian
ini secara konsisten menunjukkan bahwa memindahkan NMS dari eksekusi CPU/*host* standar ke
eksekusi paralel di perangkat keras memberi percepatan signifikan, namun ketiganya membangun
akselerator/kernel *khusus dari nol* (*custom hardware* atau *custom* CUDA *kernel*) — berbeda
dari pendekatan yang lebih umum diadopsi pengembang aplikasi di industri, yaitu memanfaatkan
*plugin* optimasi siap pakai bawaan vendor *runtime* inferensi. Celah pada penerapan pendekatan
*plugin* vendor (bukan kernel kustom) inilah yang mendasari rumusan masalah #2 (§1.2).

Selain kedua klaster di atas, tinjauan terhadap literatur yang tersedia juga **tidak menemukan
studi yang secara langsung membandingkan efisiensi komputasi antar-algoritma *tracker*** (mis.
pendekatan berbasis fitur visual vs. berbasis gerak murni) pada *pipeline* NVIDIA DeepStream di
perangkat *edge* kelas Jetson Orin Nano — celah literatur inilah yang menjadi dasar rumusan
masalah #3, sebagai pengganti rumusan masalah berbasis DLA pada proposal awal yang tidak dapat
dilaksanakan pada perangkat target akhir (lihat paragraf berikut dan §1.2).

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

> `[VERIFIKASI]` Paragraf di atas diadaptasi dari latar belakang proposal asli, dengan
> penyesuaian: (a) referensi "Jetson AGX Orin" pada paragraf ke-4 diganti "Jetson Orin
> Nano", (b) kalimat penutup ditambah frasa yang mengaitkan latar belakang ke axis
> tracking (poin baru pengganti DLA) supaya alur ke 1.2 tidak terasa tiba-tiba. Semua
> sitasi (Costa et al., Neumann, dst.) dipertahankan apa adanya dari proposal — daftar
> pustaka lengkap ada di `../Proposal/`. Update 2026-08-14: seluruh 20 sitasi proposal
> (termasuk yang dikutip di paragraf ini) sudah disalin ke `../journal/daftar-referensi.md`
> (lihat `../log/log-perubahan.md` entri 11:55 WITA) — catatan "belum disalin" sebelumnya
> sudah tidak berlaku.

## 1.2 Rumusan Masalah

Berdasarkan latar belakang penelitian, rumusan masalah dalam penelitian ini adalah
sebagai berikut:

1. Bagaimana kinerja *real-time pipeline* Nvidia DeepStream pada proses deteksi
   kendaraan di Jetson Orin Nano?
2. Sejauh mana penerapan NMS paralel berbasis TensorRT plugin (EfficientNMS) dapat
   meningkatkan efisiensi *pipeline* deteksi kendaraan dibandingkan konfigurasi standar?
3. Sejauh mana pemilihan algoritma *tracking* (NvDCF vs. NvSORT) memengaruhi efisiensi
   komputasi *real-time pipeline* dibandingkan konfigurasi *baseline*?

> `[VERIFIKASI]` Perubahan dari proposal asli:
> - Poin 1: perangkat diganti dari Jetson AGX Orin → Jetson Orin Nano.
> - Poin 2: frasa "custom CUDA logic" diganti "NMS paralel berbasis TensorRT plugin
>   (EfficientNMS)" agar sesuai implementasi riil — realisasi teknisnya memakai plugin
>   `EfficientNMS_TRT` bawaan TensorRT yang diintegrasikan ke graph model (lihat
>   `../../utils/trt_efficientnms/README.md` dan `../../config/pgie_yolov8n_kitti_efficientnms.txt`),
>   bukan kernel CUDA yang ditulis penuh dari nol seperti pada diagram "Custom CUDA Logic"
>   di proposal asli (Gambar 1). Kata "custom" sengaja **tidak** dipakai (sempat ditulis
>   "custom NMS berbasis TensorRT plugin" di draf sebelumnya, dikoreksi 2026-08-14) karena
>   kontradiktif dengan penjelasan ini sendiri — plugin vendor bukan implementasi custom.
>   Penulis sudah punya narasi pembelaan untuk perbedaan pendekatan (plugin vs. kernel
>   custom) dari diskusi sesi sebelumnya — pastikan konsisten dipakai juga di seminar hasil.
> - Poin 3 (pengganti poin DLA): DLA tidak tersedia di Jetson Orin Nano (hanya di Jetson
>   AGX Orin dan Orin NX — lihat teks proposal asli halaman 10), sehingga tidak bisa
>   dieksekusi di perangkat final. Diganti perbandingan algoritma *tracking* (NvDCF vs.
>   NvSORT), karena:
>   (a) **infrastruktur pengujiannya sudah lengkap** — 2 profil tracker (`config/tracker_nvdcf.yml`,
>   `config/tracker_nvsort.yml`) dan runner otomatis `scripts/run_all_benchmark.sh` (12 skenario:
>   6 model × 2 konfigurasi tracker) sudah ada di kode, tetapi **eksekusi formal di Jetson dan
>   pengumpulan datanya belum dilakukan** (dicek 2026-08-14: `data/benchmark/` belum ada isinya)
>   — koreksi dari klaim draf sebelumnya yang menyebut "sudah diuji", supaya status pekerjaan
>   tidak dilebih-lebihkan; (b) tidak tumpang tindih dengan poin 2 (tracking adalah tahap pipeline
>   terpisah dari NMS); (c) **scope dibatasi pada efisiensi komputasi saja** (FPS,
>   `Lat_Tracker_ms`, utilisasi resource dari `tegrastats`) — evaluasi kualitas/akurasi
>   tracking (ID switch, MOTA/IDF1) sengaja di luar scope, lihat 1.5 poin 5.

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

> `[VERIFIKASI]` Manfaat teoritis poin DLA ("evaluasi penggunaan DLA pada perangkat
> Jetson AGX Orin") diganti sesuai poin 3 baru. Manfaat praktis & institusional hanya
> diganti nama perangkat, isinya tidak berubah dari proposal asli.

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
   dengan rumusan masalah #3 yang memang dirumuskan secara eksplisit sebagai pertanyaan
   "efisiensi komputasi" (§1.2), sejalan dengan sumbu akurasi-vs-komputasi yang sudah
   melekat pada desain profil NvDCF/NvSORT itu sendiri menurut dokumentasi resmi
   NVIDIA (lihat Bab II §2.5.3) — bukan sesuatu yang perlu diukur ulang penelitian ini agar
   pertanyaannya valid dijawab. Ketidaktersediaan dataset *tracking* berlabel (video
   berurutan dengan ID objek konsisten, mis. KITTI Tracking) pada ruang lingkup penelitian
   ini menjadi alasan pendukung tambahan.

> `[VERIFIKASI]` Poin 1 diganti nama perangkat. Poin 2 disesuaikan (DLA dihapus, frasa
> NMS disamakan dengan 1.2). Poin 3 dan 4 tidak berubah. Poin 5 baru — wajib ada supaya
> pembatasan scope tracking di poin 1.2/1.3 punya dasar formal di sini, bukan cuma
> disebutkan sekilas. Argumentasi poin 5 diperkuat 2026-08-14 (lihat Bab II §2.5.3, sejak
> 2026-08-19 dipindah dari §2.2.6 draf Tinjauan Pustaka lama — lihat uraian lengkap + sitasi)
> atas permintaan penulis, dengan tujuan spesifik: penggantian
> rumusan masalah #3 (DLA → tracking NvDCF vs NvSORT) **belum** dikonfirmasi ke dosen
> pembimbing — penulis memilih lanjut menulis draf lebih dulu dan baru merevisi di bimbingan
> berikutnya kalau tidak disetujui. Argumentasi ini disiapkan supaya, kalaupun rumusan
> masalah #3 disetujui, dosen tidak meminta penambahan metrik kualitas *tracking*
> (MOTA/IDF1/ID switch) yang membutuhkan dataset baru di luar ruang lingkup saat ini.

## 1.6 Sistematika Penulisan

TODO — biasanya berisi ringkasan 1 paragraf per BAB (BAB I–V), ditulis terakhir setelah
seluruh draf BAB lain stabil, supaya ringkasannya akurat.

# BAB IV KESIMPULAN DAN SARAN

## 4.1 Kesimpulan

Berdasarkan hasil pengujian dan pembahasan pada BAB III (60 *run*, terdiri atas 6 model × 2
*tracker* × 5 repetisi, pada Jetson Orin Nano), kesimpulan penelitian ini disusun untuk menjawab
ketiga rumusan masalah dan tujuan penelitian yang ditetapkan pada Bab I sebagai berikut:

1. **Kinerja *real-time pipeline* Nvidia DeepStream pada Jetson Orin Nano (menjawab RM1/Tujuan
   1).** Keempat model deteksi *baseline* (YOLOv8n, YOLOv9t, YOLOv10n, YOLO26n) berhasil berjalan
   *real-time* pada Jetson Orin Nano dengan margin yang besar terhadap ambang 30 FPS, dengan
   rerata FPS berkisar 51,52 (YOLOv9t) hingga 67,02 (YOLOv10n), dan **seluruh 60 *run* individual,
   tanpa kecuali, memenuhi ambang tersebut**, bahkan skenario paling lambat masih melampauinya
   sebesar 66%. Analisis latensi per-komponen mengidentifikasi YOLOv9t sebagai *outlier*
   konsisten: latensi *end-to-end*-nya (rerata 457,67 ms, rerata P95 536,32 ms) jauh di atas
   ketiga model lain, disebabkan *backpressure* pada latensi tahap pra-*multiplexing* (360,92 ms)
   akibat interaksi arsitektur PGI/GELAN yang lebih *sequential* dengan biaya *tracker* NvDCF di
   tahap hilir. Ditemukan pula bahwa GFLOPs teoretis tidak berbanding lurus dengan performa aktual
   di perangkat *edge* (YOLO26n memiliki GFLOPs terendah namun bukan model tercepat).

2. **Pengaruh NMS paralel berbasis TensorRT *plugin* (EfficientNMS) terhadap efisiensi *pipeline*
   (menjawab RM2/Tujuan 2).** EfficientNMS **tidak terbukti meningkatkan efisiensi *pipeline***
   secara signifikan pada kedua model yang diuji (YOLOv8n, YOLOv9t). Pada tahap inferensi, latensi
   tahap inferensi membaik, tetapi hanya dalam orde sub-milidetik, konsisten dengan biaya
   komputasi *plugin* EfficientNMS itu sendiri yang memang sudah sangat murah (~0,05 ms) pada
   TensorRT 10.3. Pada tingkat *throughput* keseluruhan, perbedaan FPS pada YOLOv8n tidak
   signifikan secara statistik (*p* = 0,592), sedangkan pada YOLOv9t perbedaannya justru
   **signifikan namun berlawanan arah dari hipotesis**: EfficientNMS 2,1% lebih lambat
   (*p* = 0,023). Temuan negatif ini bukan kegagalan implementasi, melainkan konsekuensi dari
   sifat *plugin* EfficientNMS sebagai tahap akhir (*tail*) yang tidak *overlap* dengan komputasi
   *backbone*, serta karena latensi tahap inferensi bukan *bottleneck* dominan pada kelas model
   *nano/tiny* ini. Pembahasan model NMS-*free* (YOLOv10n, YOLO26n) memperkuat simpulan ini:
   pendekatan menghilangkan NMS secara arsitektural pun tidak otomatis menjamin *throughput*
   tertinggi, karena latensi tahap pra-*multiplexing* dan latensi tahap *tracking* tetap menjadi
   kontributor lebih dominan terhadap FPS akhir dibanding latensi tahap inferensi.

3. **Pengaruh pemilihan algoritma *tracking* (NvDCF vs. NvSORT) terhadap efisiensi komputasi
   (menjawab RM3/Tujuan 3).** Pada level komponen, biaya komputasi NvDCF pada tahap *tracking*
   secara konsisten **~12×–32× lebih tinggi** daripada NvSORT di seluruh enam model yang diuji.
   Temuan ini bersifat arsitektural dan tidak bergantung pada model deteksi di hulunya, sejalan
   dengan perbedaan mendasar antara ekstraksi fitur berbasis piksel (NvDCF) dan pendekatan murni
   berbasis gerak (NvSORT). Namun demikian, penghematan ini **bersifat kondisional** pada level
   *throughput* akhir *pipeline*: peningkatan FPS yang besar dan sangat signifikan (+30% dan +33%,
   *p* < 0,0001) hanya teramati pada YOLOv9t, satu-satunya model yang *headroom*
   waktu-per-*frame*-nya sudah hampir habis, sedangkan pada YOLOv8n, YOLOv10n, dan YOLO26n
   selisihnya kecil secara praktis (0,09–1,63 FPS) karena biaya tambahan NvDCF "tersembunyi" di
   dalam *slack* yang masih tersedia. Dari sisi *resource*, NvSORT konsisten lebih hemat energi
   per *frame* di *seluruh* enam model, meski daya sesaat pada jalur utama (*VDD_IN*) tidak selalu
   lebih rendah. Implikasi praktisnya, **pemilihan *tracker* tidak dapat dievaluasi secara
   independen dari model deteksi yang dipasangkannya**: dampaknya terhadap sistem secara
   keseluruhan hanya dapat diketahui melalui pengukuran *end-to-end*, bukan dari spesifikasi
   komponen semata.

Secara keseluruhan, temuan di atas menunjukkan bahwa optimasi *real-time pipeline* Nvidia
DeepStream pada Jetson Orin Nano untuk aplikasi ADAS **tidak dapat digeneralisasi dengan satu
resep tunggal**, melainkan bergantung pada kombinasi model dan konfigurasi yang dipasangkan.
Rekomendasi berkondisi yang disusun pada Bab III merangkum hal ini: YOLOv8n dengan NvSORT untuk
prioritas akurasi maksimum, YOLO26n dengan NvSORT untuk prioritas efisiensi komputasi dengan
akurasi kompetitif, YOLOv10n sebagai kandidat *default* Pareto-*front* yang paling toleran
terhadap pilihan *tracker* apa pun, sedangkan YOLOv9t tidak direkomendasikan pada konfigurasi
*default* (NvDCF) kecuali dipasangkan dengan NvSORT. Perlu dicatat bahwa nilai mAP50-95 yang
mendasari rekomendasi ini masih berupa *proxy* FP32; verifikasi akurasi *as-deployed* FP16
**belum dieksekusi** pada penelitian ini, sehingga kesimpulan di atas, khususnya yang menyangkut
trade-off akurasi, masih bersyarat pada asumsi bahwa deviasi kuantisasi FP16 kecil, sebagaimana
umum dilaporkan pada literatur model YOLO, namun belum diverifikasi secara independen pada
perangkat target penelitian ini.

## 4.2 Saran

Berdasarkan keterbatasan metodologis dan temuan yang diuraikan pada Bab III, berikut saran yang
diajukan bagi penelitian lanjutan maupun pengembangan sistem, diurutkan dari usaha kecil/dampak
sedang ke usaha besar/dampak besar:

1. **Menyelesaikan verifikasi akurasi *as-deployed* FP16** sebagai prioritas utama. Infrastruktur
   pendukungnya, yaitu mekanisme pencatatan luaran deteksi mentah, utilitas konversi kumpulan
   gambar validasi menjadi video, dan utilitas evaluasi mAP berbasis standar COCO, sudah tersedia,
   sehingga langkah yang tersisa murni eksekusi lapangan di Jetson. Langkah ini penting karena
   seluruh rekomendasi *trade-off* akurasi pada penelitian ini masih bergantung pada *proxy* FP32.
2. **Menambahkan kanal pengukuran suhu SoC** pada mekanisme penguraian (*parsing*) log utilisasi
   perangkat keras (*tegrastats* sebenarnya sudah melaporkan suhu tetapi belum diekstraksi) untuk
   memverifikasi secara langsung efektivitas mitigasi *thermal throttling* (jeda *cooldown* 60
   detik), yang pada penelitian ini baru bersifat prosedural, bukan terverifikasi dengan data suhu
   aktual.
3. **Mengeksplorasi jalur optimasi lain di luar *plugin* EfficientNMS itu sendiri**, mengingat
   temuan negatif pada RM2, mengikuti urutan prioritas berikut: (a) menaikkan ambang skor
   kepercayaan (*score threshold*) atau menurunkan jumlah kandidat deteksi, selama evaluasi
   mAP/*recall* masih mengizinkan; (b) menguji batas maksimum *bounding box* keluaran yang lebih
   kecil (100/200) bila jumlah objek aktual tidak pernah mendekati batas tersebut, untuk
   mengurangi beban *tracker*/parser alih-alih *selection* internal *plugin*; (c) melakukan
   profil mendalam dengan perkakas profil TensorRT atau Nsight Systems sebelum menyimpulkan
   sumber *bottleneck* secara pasti, ketimbang hanya dari satu kali jalan *pipeline*;
   (d) mempertimbangkan arsitektur *head* NMS-*free* yang sudah tersedia pada penelitian ini
   (YOLOv10n, YOLO26n) sebagai alternatif yang lebih menjanjikan daripada menulis kernel NMS
   generik dari awal; (e) *custom plugin* baru layak dipertimbangkan bila dapat mem-*fuse*
   seluruh tahap *decode*–*threshold*–NMS–*compact output* dalam satu modul *plugin* TensorRT
   khusus untuk konfigurasi target.
4. **Melakukan eksperimen presisi INT8** sebagai variabel tambahan (di luar SKU tanpa DLA seperti
   Jetson Orin Nano yang dipakai penelitian ini), untuk melengkapi perbandingan FP16 vs. INT8
   secara terukur, bukan sekadar argumen teoretis.
5. **Menguji skenario tambahan**, seperti kepadatan lalu lintas tinggi, kondisi cahaya rendah,
   cuaca buruk, serta durasi klip video yang lebih panjang sesuai rekomendasi protokol pengujian
   yang digunakan pada penelitian ini, untuk menguji generalisasi temuan penelitian ini di luar
   satu klip video terkontrol yang dipakai.
6. **Mengukur kualitas/ketahanan *tracking*** (jumlah *ID switch*, MOTA/IDF1) secara terpisah dari
   efisiensi komputasi yang dievaluasi pada penelitian ini (di luar *scope* penelitian ini), agar
   gambaran *trade-off* NvDCF vs. NvSORT lebih menyeluruh, mencakup dimensi akurasi *tracking*,
   bukan hanya kecepatan dan konsumsi *resource*.
7. **Menguji skenario *deployment* multi-kamera/multi-model** pada SKU 4GB maupun 8GB Jetson Orin
   Nano, untuk mengonfirmasi secara empiris batas praktis keterbatasan memori yang teridentifikasi
   pada penelitian ini (penggunaan 31–35% dari kapasitas 4GB untuk satu *stream*/satu model) pada
   konfigurasi yang lebih kompleks dan lebih merepresentasikan kebutuhan ADAS produksi.
8. **Memanfaatkan kemampuan *depth-sensing* stereo kamera ZED**, yang pada penelitian ini hanya
   dipakai sebagai sumber video 2D (sebagaimana disebutkan pada batasan penelitian ini), untuk
   estimasi jarak kasar ke objek terdeteksi, sebagai langkah lanjutan dari "deteksi 2D" menuju
   "persepsi 3D" yang lebih dekat dengan kebutuhan fungsional sistem ADAS sesungguhnya.

Saran-saran di atas disusun agar penelitian lanjutan dapat membangun langsung di atas
infrastruktur pengujian (skrip *benchmark*, profil *tracker*, *tooling* NMS) yang sudah tersedia
dari penelitian ini, tanpa perlu mengulang tahapan yang sudah selesai divalidasi.

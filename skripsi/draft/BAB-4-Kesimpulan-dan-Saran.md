# BAB IV — KESIMPULAN DAN SARAN

> Status: draf isi penuh (2026-08-19), disusun berdasarkan hasil BAB III (data eksekusi 60 *run*
> — 6 model × 2 *tracker* × 5 repetisi — pada Jetson Orin Nano). Kesimpulan menjawab rumusan
> masalah/tujuan penelitian di `BAB-1-Pendahuluan.md` §1.2–§1.3 satu per satu. Penomoran bab
> mengikuti restrukturisasi format skripsi Unhas 2026-08-19 (BAB II Metode Penelitian, BAB III
> Hasil dan Pembahasan — lihat `../log/log-perubahan.md`). Bagian §3.4.2 (verifikasi akurasi
> *as-deployed* FP16) masih **belum dieksekusi** pada saat draf ini ditulis — kesimpulan di bawah
> mencantumkan keterbatasan ini secara eksplisit agar tidak melebih-lebihkan kepastian hasil.

## 4.1 Kesimpulan

Berdasarkan hasil pengujian dan pembahasan pada BAB III (60 *run* — 6 model × 2 *tracker* × 5
repetisi — pada Jetson Orin Nano), kesimpulan penelitian ini disusun untuk menjawab ketiga
rumusan masalah dan tujuan penelitian yang ditetapkan pada `BAB-1-Pendahuluan.md` §1.2–§1.3
sebagai berikut:

1. **Kinerja *real-time pipeline* Nvidia DeepStream pada Jetson Orin Nano (menjawab RM1/Tujuan
   1).** Keempat model deteksi *baseline* (YOLOv8n, YOLOv9t, YOLOv10n, YOLO26n) berhasil berjalan
   *real-time* pada Jetson Orin Nano dengan margin yang besar terhadap ambang 30 FPS
   (`BAB-2-Metode-Penelitian.md` §2.6.2) — rata-rata FPS berkisar 51,52 (YOLOv9t) hingga 67,02
   (YOLOv10n), dan **seluruh 60 *run* individual, tanpa kecuali, memenuhi ambang tersebut**
   (§3.1.1), bahkan skenario paling lambat masih melampauinya sebesar 66%. Analisis latensi
   per-komponen (§3.1.3) mengidentifikasi YOLOv9t sebagai *outlier* konsisten — latensi
   *end-to-end*-nya (median 454,38 ms, p99 547,15 ms) jauh di atas ketiga model lain, disebabkan
   *backpressure* pada `Lat_PreMux_ms` (360,92 ms) akibat interaksi arsitektur PGI/GELAN yang
   lebih *sequential* dengan biaya *tracker* NvDCF di tahap hilir. Ditemukan pula bahwa GFLOPs
   teoretis tidak berbanding lurus dengan performa aktual di perangkat *edge* (YOLO26n memiliki
   GFLOPs terendah namun bukan model tercepat, §3.1.3).

2. **Pengaruh NMS paralel berbasis TensorRT *plugin* (EfficientNMS) terhadap efisiensi *pipeline*
   (menjawab RM2/Tujuan 2).** EfficientNMS **tidak terbukti meningkatkan efisiensi *pipeline***
   secara signifikan pada kedua model yang diuji (YOLOv8n, YOLOv9t). Pada tahap inferensi,
   `Lat_Infer_ms` membaik tetapi hanya dalam orde sub-milidetik (§3.2.1), konsisten dengan biaya
   `EfficientNMS_TRT` yang memang sudah sangat murah (~0,05 ms) pada TensorRT 10.3. Pada tingkat
   *throughput* keseluruhan (§3.2.2), perbedaan FPS pada YOLOv8n tidak signifikan secara
   statistik (*p* = 0,592), sedangkan pada YOLOv9t perbedaannya justru **signifikan namun
   berlawanan arah dari hipotesis** — EfficientNMS 2,1% lebih lambat (*p* = 0,023). Temuan negatif
   ini bukan kegagalan implementasi, melainkan konsekuensi dari sifat `EfficientNMS_TRT` sebagai
   *tail* yang tidak *overlap* dengan komputasi *backbone*, serta karena `Lat_Infer_ms` bukan
   *bottleneck* dominan pada kelas model *nano/tiny* ini. Pembahasan model NMS-*free*
   (YOLOv10n, YOLO26n, §3.2.3) memperkuat simpulan ini: pendekatan menghilangkan NMS secara
   arsitektural pun tidak otomatis menjamin *throughput* tertinggi, karena `Lat_PreMux_ms` dan
   `Lat_Tracker_ms` tetap menjadi kontributor lebih dominan terhadap FPS akhir dibanding
   `Lat_Infer_ms`.

3. **Pengaruh pemilihan algoritma *tracking* (NvDCF vs. NvSORT) terhadap efisiensi komputasi
   (menjawab RM3/Tujuan 3).** Pada level komponen, biaya komputasi NvDCF (`Lat_Tracker_ms`)
   secara konsisten **10×–62× lebih tinggi** daripada NvSORT di seluruh enam model yang diuji
   (§3.3.1) — temuan yang bersifat arsitektural dan tidak bergantung pada model deteksi di
   hulunya, sejalan dengan perbedaan mendasar antara ekstraksi fitur berbasis piksel (NvDCF) dan
   pendekatan murni berbasis gerak (NvSORT). Namun demikian, penghematan ini **bersifat
   kondisional** pada level *throughput* akhir *pipeline* (§3.3.2): peningkatan FPS yang besar dan
   sangat signifikan (+30% dan +33%, *p* < 0,0001) hanya teramati pada YOLOv9t — satu-satunya
   model yang *headroom* waktu-per-*frame*-nya sudah hampir habis — sedangkan pada YOLOv8n,
   YOLOv10n, dan YOLO26n selisihnya kecil secara praktis (0,09–1,63 FPS) karena biaya tambahan
   NvDCF "tersembunyi" di dalam *slack* yang masih tersedia. Dari sisi *resource* (§3.3.3),
   NvSORT konsisten lebih hemat energi per *frame* di *seluruh* enam model, meski daya sesaat
   (`VDD_IN`) tidak selalu lebih rendah. Implikasi praktisnya: **pemilihan *tracker* tidak dapat
   dievaluasi secara independen dari model deteksi yang dipasangkannya** — dampaknya terhadap
   sistem secara keseluruhan hanya dapat diketahui melalui pengukuran *end-to-end*, bukan dari
   spesifikasi komponen semata.

Secara keseluruhan, hasil di atas menunjukkan bahwa optimasi *real-time pipeline* Nvidia
DeepStream pada Jetson Orin Nano untuk aplikasi ADAS **tidak dapat digeneralisasi dengan satu
resep tunggal**, melainkan bergantung pada kombinasi model dan konfigurasi yang dipasangkan.
Rekomendasi berkondisi yang disusun pada `BAB-3-Hasil-dan-Pembahasan.md` §3.5.1 merangkum hal
ini: YOLOv8n dengan NvSORT untuk prioritas akurasi maksimum, YOLO26n dengan NvSORT untuk
prioritas efisiensi komputasi dengan akurasi kompetitif, YOLOv10n sebagai kandidat *default*
Pareto-*front* yang paling toleran terhadap pilihan *tracker* apa pun, sedangkan YOLOv9t tidak
direkomendasikan pada konfigurasi *default* (NvDCF) kecuali dipasangkan dengan NvSORT. Perlu
dicatat bahwa nilai mAP50-95 yang mendasari rekomendasi ini masih berupa *proxy* FP32 (§3.4.1);
verifikasi akurasi *as-deployed* FP16 (§3.4.2) **belum dieksekusi** pada penelitian ini, sehingga
kesimpulan di atas — khususnya yang menyangkut trade-off akurasi — masih bersyarat pada asumsi
bahwa deviasi kuantisasi FP16 kecil, sebagaimana umum dilaporkan pada literatur model YOLO, namun
belum diverifikasi secara independen pada perangkat target penelitian ini.

## 4.2 Saran

Berdasarkan keterbatasan metodologis dan temuan yang diuraikan pada
`BAB-3-Hasil-dan-Pembahasan.md` §3.5.2, berikut saran yang diajukan bagi penelitian lanjutan
maupun pengembangan sistem, diurutkan dari usaha kecil/dampak sedang ke usaha besar/dampak besar:

1. **Menyelesaikan verifikasi akurasi *as-deployed* FP16** (§3.4.2) sebagai prioritas utama.
   Infrastrukturnya sudah tersedia di kode (`--dump-detections` pada `src/main.cpp`,
   `scripts/prepare_eval_video.sh`, `utils/eval_map/eval_deepstream_map.py`), sehingga langkah
   yang tersisa murni eksekusi lapangan di Jetson. Hasil ini penting karena seluruh rekomendasi
   *trade-off* akurasi pada penelitian ini (§3.5.1) masih bergantung pada *proxy* FP32.
2. **Menambahkan kanal pengukuran suhu SoC** pada `LogParser` — `tegrastats` sebenarnya sudah
   melaporkan suhu tetapi belum diekstraksi — untuk memverifikasi secara langsung efektivitas
   mitigasi *thermal throttling* (jeda *cooldown* 60 detik), yang pada penelitian ini baru
   bersifat prosedural, bukan terverifikasi dengan data suhu aktual.
3. **Mengeksplorasi jalur optimasi lain di luar *plugin* EfficientNMS itu sendiri**, mengingat
   temuan negatif pada RM2 (§3.2.2), mengikuti urutan yang direkomendasikan
   `../../utils/trt_efficientnms/README.md` §"Batas optimasi dan alternatif":
   (a) menaikkan `score-threshold` atau menurunkan kandidat deteksi, selama evaluasi mAP/*recall*
   masih mengizinkan; (b) menguji `max-output-boxes` yang lebih kecil (100/200) bila jumlah objek
   aktual tidak pernah mendekati batas tersebut, untuk mengurangi beban *tracker*/parser alih-alih
   *selection* internal *plugin*; (c) melakukan profil mendalam dengan `trtexec --dumpProfile`
   atau Nsight Systems sebelum menyimpulkan sumber *bottleneck* secara pasti, ketimbang hanya dari
   satu kali jalan *pipeline*; (d) mempertimbangkan arsitektur *head* NMS-*free* yang sudah tersedia
   pada penelitian ini (YOLOv10n, YOLO26n) sebagai alternatif yang lebih menjanjikan daripada
   menulis kernel NMS generik dari awal; (e) *custom plugin* baru baru layak dipertimbangkan bila
   dapat mem-*fuse* seluruh tahap *decode*–*threshold*–NMS–*compact output* dalam satu
   `IPluginV3` khusus untuk konfigurasi target.
4. **Melakukan eksperimen presisi INT8** sebagai variabel tambahan (di luar SKU tanpa DLA seperti
   Jetson Orin Nano yang dipakai penelitian ini), untuk melengkapi perbandingan FP16 vs. INT8
   secara terukur, bukan sekadar argumen teoretis.
5. **Menguji skenario tambahan** — kepadatan lalu lintas tinggi, kondisi cahaya rendah, cuaca
   buruk, serta durasi klip video yang lebih panjang sesuai rekomendasi protokol
   (`04_benchmark_protocol.md` §4.3) — untuk menguji generalisasi temuan §3.1–§3.3 di luar satu
   klip video terkontrol yang dipakai pada penelitian ini.
6. **Mengukur kualitas/ketahanan *tracking*** (jumlah *ID switch*, MOTA/IDF1) secara terpisah dari
   efisiensi komputasi yang dievaluasi pada penelitian ini (di luar *scope* sesuai
   `BAB-1-Pendahuluan.md` §1.5 poin 5), agar gambaran *trade-off* NvDCF vs. NvSORT lebih
   menyeluruh — mencakup dimensi akurasi *tracking*, bukan hanya kecepatan dan konsumsi *resource*.
7. **Menguji skenario *deployment* multi-kamera/multi-model** pada SKU 4GB maupun 8GB Jetson Orin
   Nano, untuk mengonfirmasi secara empiris batas praktis keterbatasan memori yang teridentifikasi
   pada penelitian ini (penggunaan 31–35% dari kapasitas 4GB untuk satu *stream*/satu model,
   §3.3.3) pada konfigurasi yang lebih kompleks dan lebih merepresentasikan kebutuhan ADAS
   produksi.
8. **Memanfaatkan kemampuan *depth-sensing* stereo kamera ZED**, yang pada penelitian ini hanya
   dipakai sebagai sumber video 2D (`../../docs/08_limitations_future_work.md` §8.1 poin 1), untuk
   estimasi jarak kasar ke objek terdeteksi — langkah lanjutan dari "deteksi 2D" menuju "persepsi
   3D" yang lebih dekat dengan kebutuhan fungsional sistem ADAS sesungguhnya.

Saran-saran di atas disusun agar penelitian lanjutan dapat membangun langsung di atas
infrastruktur pengujian (skrip *benchmark*, profil *tracker*, *tooling* NMS) yang sudah tersedia
dari penelitian ini, tanpa perlu mengulang tahapan yang sudah selesai divalidasi.

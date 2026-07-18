# 8. Limitasi & Pekerjaan Selanjutnya

Menyatakan limitasi secara eksplisit dan jujur adalah bagian dari kualitas ilmiah skripsi —
bagian ini sebaiknya jadi bagian dari Bab 5/6, bukan disembunyikan.

## 8.1 Limitasi Metodologis

1. **Kamera ZED dipakai hanya sebagai sumber 2D**, walaupun merupakan kamera stereo dengan
   kemampuan depth-sensing. Estimasi jarak berbasis stereo tidak dimanfaatkan pada perception
   layer ini. Ini adalah keputusan scope yang disengaja (lihat
   [01_scope_and_architecture.md](01_scope_and_architecture.md)), tapi tetap membatasi
   kelengkapan "persepsi ADAS" yang biasanya juga mencakup estimasi jarak.
2. **Akurasi diukur dari bobot FP32 (`.pt`)**, bukan dari engine TensorRT FP16 yang benar-benar
   berjalan di Jetson. Selisihnya diasumsikan kecil berdasarkan literatur umum, tapi tidak
   diverifikasi langsung pada proyek ini (lihat
   [03_deployment_pipeline.md](03_deployment_pipeline.md) §3.4).
3. **Ketimpangan kelas pada val set KITTI** — instance `truck` (150) dan `van` (405) jauh
   lebih sedikit dari `car` (4167). Metrik per-kelas untuk `truck`/`van` kurang stabil secara
   statistik dan sensitif terhadap sampel yang salah diklasifikasi.
4. **KITTI didominasi kondisi siang hari dan cuaca cerah** — performa model pada kondisi
   malam hari, hujan, atau silau tidak tercakup dalam evaluasi ini, walaupun ini skenario
   yang relevan untuk ADAS di dunia nyata.
5. **Benchmark runtime memakai satu klip video tetap** (bukan berbagai skenario lalu lintas).
   Ini disengaja untuk kontrol eksperimen (lihat
   [04_benchmark_protocol.md](04_benchmark_protocol.md) §4.1), tapi berarti hasil FPS/latensi
   mencerminkan satu kompleksitas skenario saja — jumlah objek per frame yang berbeda bisa
   memengaruhi FPS post-processing (NMS, tracker) secara berbeda antar model.
6. **Parser custom YOLO (`libnvdsinfer_custom_impl_Yolo.so`) belum diverifikasi eksplisit**
   untuk format output YOLO26 yang NMS-free/tanpa-DFL (lihat
   [02_dataset_and_training.md](02_dataset_and_training.md) dan
   [03_deployment_pipeline.md](03_deployment_pipeline.md) §3.5). Hasil visual yang wajar jadi
   indikasi tidak langsung bahwa parsing berjalan benar, tapi belum ada verifikasi kuantitatif
   sisi DeepStream untuk model ini secara khusus.
7. **Tidak ada perbandingan precision INT8.** FP16 dipilih sebagai default yang seimbang
   (lihat rasionalisasi di [01](01_scope_and_architecture.md)), tapi tanpa data INT8 aktual,
   klaim "FP16 adalah pilihan terbaik" masih berbasis argumen teoretis, bukan hasil terukur
   pada proyek ini.
8. **Jetson Orin Nano tidak memiliki DLA** (Deep Learning Accelerator) — berbeda dari Orin
   NX/AGX. Sebagian argumen efisiensi INT8/DLA yang lazim dibahas pada literatur Jetson secara
   umum tidak berlaku langsung untuk platform spesifik ini.

## 8.2 Pekerjaan Selanjutnya (Future Work)

Urutan berikut kira-kira dari "usaha kecil, dampak sedang" ke "usaha besar, dampak besar" —
berguna kalau ditanya penguji "kalau ada waktu lebih, apa yang akan dikembangkan?":

1. **Verifikasi akurasi as-deployed**: jalankan ke-4 model lewat pipeline DeepStream FP16
   yang sebenarnya pada 1010 gambar val yang sama, lalu hitung mAP dari output
   `NvDsObjectMeta` untuk memastikan tidak ada penurunan akurasi tersembunyi akibat
   kuantisasi/parsing custom.
2. **Eksperimen INT8** sebagai variabel tambahan (butuh dataset kalibrasi kecil dari data
   training) — bandingkan trade-off FP16 vs INT8 secara terukur, bukan cuma teoretis.
3. **Manfaatkan depth stereo ZED** untuk estimasi jarak kasar ke objek terdeteksi — melangkah
   dari "deteksi 2D" menuju "persepsi 3D" yang lebih dekat ke kebutuhan ADAS sesungguhnya.
4. **Uji skenario tambahan**: video dengan kepadatan lalu lintas tinggi, kondisi cahaya
   rendah, atau cuaca buruk — untuk menguji robustheid di luar distribusi data KITTI.
5. **Uji ketahanan tracker (NvDCF)** secara terpisah dari akurasi deteksi murni — misalnya
   mengukur seberapa sering ID objek "berpindah" (ID switch) saat terjadi oklusi sebagian.
6. **Otomatisasi penuh pipeline benchmark** (opsional): skrip yang menjalankan seluruh model
   secara berurutan dengan repetisi otomatis dan menghasilkan tabel/grafik akhir langsung —
   berguna kalau jumlah model/eksperimen bertambah banyak di masa depan, tapi sengaja *tidak*
   dibuat di proyek ini karena kontrol manual per-run lebih diinginkan pada skala eksperimen
   saat ini (lihat [04_benchmark_protocol.md](04_benchmark_protocol.md)).

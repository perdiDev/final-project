# Daftar Referensi Jurnal

Index untuk file jurnal yang didownload ke folder `journal/` ini. Tambahkan baris baru
setiap kali ada file baru yang ditambahkan (oleh AI maupun user).

> Kolom **File** untuk baris di bawah ini masih "—" karena PDF jurnal individualnya belum
> didownload ke folder ini — daftar ini baru dipindahkan dari Daftar Pustaka
> `../Proposal/Proposal Final Perdi - AGX Orin ADAS-1.pdf` (halaman 14-16) pada 2026-08-14
> supaya sitasi terkumpul di satu tempat. Isi kolom **File** begitu PDF benar-benar
> didownload ke `journal/`.
>
> **Update 2026-08-19 (restrukturisasi format Unhas):** `../draft/BAB-2-Tinjauan-Pustaka.md`
> (draf lama) sudah dihapus. Ringkasan *state of the art* (klaster deteksi objek YOLO pada
> *edge* + klaster akselerasi NMS, dulu §2.1.1–§2.1.2) kini ada di
> `../draft/BAB-1-Pendahuluan.md` §1.1; klaster efisiensi *real-time*/penjadwalan (dulu §2.1.3)
> dan konteks umum ADAS (dulu §2.1.4) sudah lebih dulu terwakili sebagai sitasi langsung di
> §1.1 sejak draf awal. Landasan teori (dulu §2.2) kini terintegrasi di
> `../draft/BAB-2-Metode-Penelitian.md` §2.2 dan §2.5. Kolom **Relevansi** di bawah masih
> menyebut nomor bagian lama ("BAB II §2.1.x"/"§2.2.x") sebagai jejak sejarah dari mana ringkasan
> tiap jurnal berasal; lihat pemetaan lokasi baru di paragraf ini sebelum menelusuri.
>
> **Catatan penghapusan (2026-08-14):** dari 20 sitasi asli Daftar Pustaka proposal, 1 sitasi
> — "Wu, J., dkk. (2024)" — **dihapus permanen** dari tabel ini atas instruksi eksplisit
> penulis, setelah verifikasi Crossref API mengonfirmasi DOI-nya milik artikel lain yang tidak
> berhubungan (bukan tentang YOLOv8/deteksi objek). Baris di bawah kini bernomor ulang 1–19.
> Audit trail lengkap (temuan verifikasi + keputusan penghapusan) tetap ada di
> `../log/log-perubahan.md` dan riwayat Git, sesuai prinsip tidak menyembunyikan catatan
> verifikasi.

| No | Judul | Penulis (Tahun) | File | Relevansi / Dipakai untuk |
|----|-------|-----------------|------|----------------------------|
| 1 | Assessing YOLO models for real-time object detection in urban environments for ADAS | Ayachi, R., dkk. (2025) | — | BAB II §2.1.1 — dasar pemilihan model YOLO kelas nano/tiny |
| 2 | Road object detection using SSD-MobileNet algorithm: case study for real-time ADAS applications | Bouazizi, O., dkk. (2024) | — | BAB II §2.1.1 — pembanding model ringan lain di luar keluarga YOLO |
| 3 | A real-time vehicle detection system for ADAS using YOLOv11 on embedded edge platforms | Chaman, M., dkk. (2025) | — | BAB II §2.1.1 |
| 4 | Scalable hardware acceleration of non-maximum suppression | Chen, C., dkk. (2022) | — | BAB II §2.1.2 — dasar literatur rumusan masalah #2 (NMS) |
| 5 | Real-time object detection and tracking based on embedded edge devices for local dynamic map generation | Choi, K., dkk. (2024) | — | BAB I §1.1 ¶2; BAB II §2.1.3 |
| 6 | Safety impact of advanced driver assistance systems in Europe in 2030 | Costa, A., dkk. (2025) | — | BAB I §1.1 ¶1; BAB II §2.1.4, §2.7 — dasar rasionalisasi ambang *real-time* ≥30 FPS |
| 7 | Real-time object recognition for ADAS using deep learning on edge devices | Dhatrika, S. K., dkk. (2025) | — | BAB II §2.1.1 — paling relevan metodologis dengan rumusan masalah #1 (DeepStream + TensorRT di Jetson Nano/Orin) |
| 8 | Quantized object detection for real-time inference on embedded GPU architectures | Guerrouj, F. Z., dkk. (2025) | — | BAB II §2.1.1 — pembanding trade-off presisi (INT8 vs. FP16); §2.7 — dasar kekhawatiran trade-off presisi pada metrik kualitas deteksi FP16 |
| 9 | Analysis of advanced driver-assistance systems for safe and comfortable driving of motor vehicles | Neumann, T. (2024) | — | BAB I §1.1 ¶1; BAB II §2.1.4 |
| 10 | Inference serving with end-to-end latency SLOs over dynamic edge networks | Nigade, V., dkk. (2024) | — | BAB I §1.1 ¶3; BAB II §2.1.3, §2.7 — dasar rasionalisasi pelaporan latensi persentil P95 |
| 11 | Work-Efficient Parallel Non-Maximum Suppression Kernels | Oro, D., Fernández, C., Martorell, X., & Hernando, J. (2022) — *The Computer Journal*, 65(4), 773–787, DOI: 10.1093/comjnl/bxaa108 — dikoreksi dari "Oro García, D. (2020)" di Daftar Pustaka proposal: surname penulis pertama dikonfirmasi via metadata CrossRef hanya "Oro" (bukan "Oro García"), dan tahun disesuaikan ke tahun terbit cetak jurnal (Vol. 65 No. 4, April 2022) — bukan tahun publikasi *online-first* (Agustus 2020) — supaya memenuhi syarat sitasi minimal tahun 2022 | — | BAB II §2.1.2 — dasar literatur rumusan masalah #2 (NMS) |
| 12 | Real-time unsupervised video object detection on the edge | Ruiz-Barroso, P., dkk. (2025) | — | BAB I §1.1 ¶4; BAB II §2.1.3 |
| 13 | Adaptive urgency-based real-time task scheduling in ADAS systems | Seyfipoor, M., dkk. (2026) | — | BAB I §1.1 ¶5; BAB II §2.1.3 |
| 14 | Advanced driver assistance system (ADAS) and machine learning (ML): the dynamic duo revolutionizing the automotive industry | Shah, H., dkk. (2025) | — | BAB I §1.1 ¶2; BAB II §2.1.3 |
| 15 | Power requirements evaluation of embedded devices for real-time video line detection | Suder, J., dkk. (2023) | — | BAB I §1.1 ¶4; BAB II §2.1.3, §2.7 — preseden metrik daya (`tegrastats`) |
| 16 | Real-time collision warning system based on YOLOv8 for advanced driver assistance systems | Tsai & Hsieh (2025) — dikoreksi dari "Tsai, Hsu, & Lin (2025)" di Daftar Pustaka proposal (nama penulis asli keliru, dikonfirmasi via DOI, lihat BAB II §2.1.1) | — | BAB II §2.1.1 |
| 17 | Real-time object detection from UAV inspection videos by combining YOLOv5s and DeepStream | Xie, S., dkk. (2024) | — | BAB II §2.1.1 — preseden penggunaan DeepStream SDK di luar domain otomotif |
| 18 | Object detection post processing accelerator based on co-design of hardware and software | Yang, D., dkk. (2025) | — | BAB II §2.1.2 — dasar literatur rumusan masalah #2 (NMS) |
| 19 | An edge server placement based on graph clustering in mobile edge computing | Zhang, S., dkk. (2024) | — | BAB I §1.1 ¶3; BAB II §2.1.3 |
| 20 | MLPerf Mobile Inference Benchmark | Janapa Reddi, V., Kanter, D., Mattson, P., dkk. (2022) — *Proceedings of the 5th MLSys Conference*, Santa Clara, CA, 2022 — diganti dari "MLPerf Inference Benchmark (Reddi dkk., 2020, arXiv:1911.02549)" pada draf sebelumnya karena sitasi asli berusia sebelum 2022; versi Mobile ini diverifikasi lewat PDF resmi prosiding MLSys 2022, dan secara konten lebih relevan (fokus benchmark performa on-device/edge, sejalan dengan konteks Jetson Orin Nano, bukan server datacenter) | — | BAB II §2.2.6, §2.7 — preseden metodologis: perbandingan algoritma murni dari sisi biaya komputasi (akurasi jadi ambang kelulusan tetap, bukan variabel dibandingkan), dasar argumentasi kenapa evaluasi tracker tidak perlu metrik MOTA/IDF1 |
| 21 | Gst-nvtracker — DeepStream documentation — dokumentasi resmi vendor, bukan jurnal | NVIDIA (t.t.) | — | BAB II §2.2.6 — sumber deskripsi resmi NvDCF (feature-based) vs NvSORT (motion-only, tanpa pemrosesan piksel). Diverifikasi langsung 2026-08-14, diakses dari https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_plugin_gst-nvtracker.html |
| 22 | State-of-the-art real-time multi-object trackers with NVIDIA DeepStream SDK 6.2 (NVIDIA Developer Blog) — bukan jurnal | Shin, P., & Li, F. (2023) — dikoreksi dari "NVIDIA (2023)" setelah verifikasi 2026-08-14: blog ini punya penulis individu tercantum (Paul Shin & Fangyu Li), bukan atribusi organisasi generik | — | BAB II §2.2.6 — sumber framing NvSORT "lightweight tapi competitively accurate" vs NvDCF "best accuracy and robustness". Diverifikasi langsung 2026-08-14, diakses dari https://developer.nvidia.com/blog/state-of-the-art-real-time-multi-object-trackers-with-nvidia-deepstream-sdk-6-2/ |
| 23 | NVIDIA Jetson Orin Nano Series Modules Data Sheet (DS-11105-001_v1.5, Desember 2024) — *datasheet* resmi, bukan jurnal | NVIDIA (2024) | — | BAB II §2.2.7 — sumber tabel spesifikasi resmi (CUDA/Tensor core, TOPS, CPU, memori, mode daya) untuk SKU Jetson Orin Nano **4GB** yang dipakai penelitian ini (dikonfirmasi via `nvpmodel -q` = 10W, unik untuk SKU 4GB). Dibaca langsung 2026-08-15 dari mirror https://www.esys.ir/images/img_Item/3029/Files/Jetson-Orin-Nano-Series-Modules-Datasheet_DS-11105-001_v1.5.pdf (tidak menemukan tautan `developer.nvidia.com` yang bisa diverifikasi langsung; isi dokumen dikonfirmasi asli — header/footer NVIDIA Corporation, nomor dokumen & versi konsisten dengan referensi ke datasheet ini di sumber lain) |

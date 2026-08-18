# Log Perubahan — Folder `skripsi/`

File ini adalah **satu-satunya** file log untuk semua perubahan di dalam folder `skripsi/`.
AI (Claude) wajib menambah entri baru di sini setiap kali membuat/mengubah/menghapus file
di dalam `skripsi/` atau subfoldernya.

**Format entri**: entri terbaru ditaruh paling atas (di bawah baris ini), diawali
`## YYYY-MM-DD HH:MM (zona waktu)`, diikuti bullet list ringkas berisi apa yang berubah dan
kenapa (kalau relevan).

---

## 2026-08-18 14:45 WITA — Eksekusi perdana `aggregate_runtime.py` dengan data benchmark awal

- **skripsi/eksperimen/runtime_summary.csv**, **runtime_per_run.csv**: Berhasil menjalanakan script agregasi dan menghasilkan data awal untuk BAB IV.
- **skripsi/eksperimen/plots/**: Menghasilkan grafik boxplot FPS per model dan per tracker.
- **utils/benchmark_analysis/common.py**: Memperbaiki logika `discover_runs` agar field `tracker` yang berupa path file (output dari `run_benchmark.sh`) dapat diproses dan dinormalisasi menjadi nama pendek (mis. `nvdcf`).
- **utils/benchmark_analysis/aggregate_runtime.py**: Memperbaiki parameter `labels` pada fungsi `ax.boxplot` untuk kompatibilitas dengan versi Matplotlib yang terpasang di sistem.
- **Lingkungan Python**: Downgrade `numpy` ke versi 1.26.4 untuk memperbaiki konflik dengan Matplotlib sistem yang belum mendukung NumPy 2.x.
- **Temuan Teknis**: Data awal pada Jetson Orin Nano menunjukkan NvSORT memiliki latensi tracker yang sangat rendah (~0.38ms) dibandingkan NvDCF (~12.17ms), serta penggunaan GPU yang lebih efisien (43% vs 88%), mengonfirmasi trade-off yang dibahas di BAB II.

## 2026-08-15 14:13 WITA — regenerasi Skripsi-Gabungan-BAB-I-III.docx untuk diajukan ke dosen pembimbing

- **draft/_build/BAB-1.clean.md**, **BAB-2.clean.md**, **BAB-3.clean.md**: regenerasi total dari isi terbaru `draft/BAB-1..3-*.md` (sebelumnya belum disinkronkan sejak build 2026-08-14 13:12, sehingga masih memuat NvDCF_perf, sitasi Wu dkk. yang sudah dihapus, dan placeholder spesifikasi Jetson Orin Nano yang sudah terisi). Semua anotasi kerja (blockquote `> ...`, tag `[VERIFIKASI]`, catatan proses semacam "dikoreksi 2026-08-14"/"sesi ini", serta rujukan path internal `../../docs/...`) dihapus dari badan bab agar terbaca sebagai naskah akademis yang tuntas; substansi klaim yang masih valid dipertahankan dan dirapikan ke kalimat baku, termasuk perbaikan kecil: rujukan silang "§1.4 Batasan Masalah" di Bab II §2.2.7 dikoreksi menjadi "§1.5" (Batasan Masalah ada di 1.5, bukan 1.4).
- **draft/_build/BAB-3.clean.md** §3.2.1: sel spesifikasi *compute board* yang sebelumnya kosong (menunggu keputusan sesi 2026-08-14) diisi ringkas mengacu ke tabel spesifikasi Jetson Orin Nano 4GB yang sudah lengkap di Bab II §2.2.7 (bukan data baru — hanya konsistensi lintas-bab).
- **draft/_build/BAB-3.clean.md** §3.4: diagram mermaid pipeline diganti representasi naratif (daftar bernomor) agar tampil rapi di Word; jumlah tracker disesuaikan dari 3 (versi lama) menjadi 2 (NvDCF/NvSORT, sesuai keputusan 07:15 WITA).
- **draft/_build/generate_docx.py**: perbarui subjudul halaman judul ("dokumen kerja, belum final" → "draf untuk diperiksa dosen pembimbing"); rombak daftar `VERIFICATION_NOTES` (lampiran "Catatan Verifikasi dan Tindak Lanjut Penulis") supaya sinkron dengan status terkini: hapus butir yang sudah selesai (spesifikasi Jetson Orin Nano, ambang FPS real-time, gaya sitasi NVIDIA), tambah butir baru (status verifikasi akurasi as-deployed FP16 baru sebatas infrastruktur kode, verifikasi independen varian 4GB via device-tree); tambah kalimat di paragraf pembuka lampiran yang menjelaskan Bab IV & V belum disertakan karena menunggu data pengujian aktual.
- **draft/Skripsi-Gabungan-BAB-I-III.docx**: dibangun ulang dari clean.md terbaru via `generate_docx.py` (dijalankan dengan `.venv-docx`); diverifikasi terprogram bahwa tag `[VERIFIKASI]` dan `NvDCF_perf` sudah nihil di seluruh isi dokumen, dan sitasi Wu dkk. hanya tersisa di lampiran catatan verifikasi (bukan di badan Bab I–III).
- Alasan: pengguna ingin mengajukan hasil sementara (Bab I–III) ke dosen pembimbing; dokumen gabungan lama (build 13:12 tanggal 14) sudah kedaluwarsa terhadap draf per-bab yang diperbarui pada 15 Agustus, dan masih memuat rincian kerja internal yang tidak pantas tampil di naskah akademis yang diajukan.

## 2026-08-15 07:45 WITA — kunci definisi real-time target ke **30 FPS** (bukan 15 FPS)

- **BAB-3-Metodologi-Penelitian.md** §3.6: ubah kriteria RM1 FPS dari "relatif terhadap FPS sumber (30 FPS — [VERIFIKASI])" → **"throughput ≥ 30 FPS"**; hapus tag `[VERIFIKASI]`, tambah justifikasi (ZED 30 FPS HD, standar ADAS safety-critical, default benchmark script).
- **docs/07_tradeoff_analysis.md** §7.5: ganti "15 FPS berdasarkan config/deepstream_app.txt" → **"≥ 30 FPS mengikuti default kamera ZED & protokol benchmark"**.
- **docs/04_benchmark_protocol.md** §4.1: tambah baris "Target real-time: ≥ 30 FPS" ke tabel variabel terkontrol.
- **scripts/run_benchmark.sh**: tambah komentar di default `CAMERA_FPS=30` menjelaskan alasan (ADAS safety-critical, ZED HD capability).
- Alasan: 30 FPS lebih ketat & defendable untuk ADAS perception layer; ZED camera mendukung 30 FPS HD; benchmark script sudah default 30 FPS; klaim 30 FPS implikasikan 15 FPS dengan margin; menetapkan *sebelum* eksperimen menghindari p-hacking.
- Catatan: `config/deepstream_app.txt` tetap punya `camera-fps-n=15` (legacy/fallback), tapi **bukan** acuan utama penelitian ini.

## 2026-08-15 07:15 WITA — standarkan rumusan masalah #3 ke NvDCF vs NvSORT (hapus NvDCF_perf)

- **BAB-1-Pendahuluan.md**: §1.2 poin 3 (rumusan masalah), §1.3 poin 3 (tujuan), §1.5 poin 2 & catatan VERIFIKASI — ganti "NvDCF vs NvSORT vs NvDCF_perf" → "NvDCF vs NvSORT"; update jumlah skenario 18→12 (6 model × 2 tracker).
- **BAB-2-Tinjauan-Pustaka.md**: §2.2.6 — hapus paragraf NvDCF_perf, sertakan hanya NvDCF & NvSORT; perbaiki narasi trade-off ke dua profil.
- **BAB-3-Metodologi-Penelitian.md**: §3.1 (RM3), §3.2.5 (tabel tracker hapus baris NvDCF_perf), §3.2.6 (tooling 18→12 skenario), §3.5 poin 3 (18→12 skenario), §3.5 poin 6 (18→12 skenario).
- **PANDUAN-AI.md**: update deskripsi keputusan rumusan masalah #3.
- **scripts/run_all_benchmark.sh**: array TRACKERS hanya (nvdcf, nvsort); komentar 18→12 skenario; output echo 3→2 tracker.
- File config `tracker_nvdcf_perf.yml` tetap disimpan (tidak dihapus) tapi tidak dipakai di benchmark formal.
- Alasan: user memutuskan hanya membandingkan NvDCF (feature-based, berat) vs NvSORT (motion-only, ringan) — dua ujung spektrum efisiensi komputasi; NvDCF_perf adalah varian menengah yang tidak diperlukan untuk menjawab rumusan masalah.

## 2026-08-15 07:02 WITA — siapkan script analisis BAB IV (runtime & trade-off), belum ada data asli

- User diminta pilih task lanjutan (device Jetson masih belum di tangan) dan memilih
  "Siapkan script analisis BAB IV" — kerangka parsing `fps.csv`/`hardware_analysis.csv`
  disiapkan lebih dulu supaya begitu Jetson tersedia, tinggal dijalankan tanpa perlu
  menulis kode lagi.
- File baru dibuat di **`utils/benchmark_analysis/`** (root proyek, bukan di dalam
  `skripsi/` — ikut konvensi `utils/eval_map/` dan `utils/trt_efficientnms/`), dicatat di
  sini karena outputnya jadi sumber data langsung untuk BAB IV:
  - `common.py` — parsing `run_info.txt`, `fps.csv`, `hardware_analysis.csv`; deteksi
    otomatis kolom dinamis (rail daya `*_mW`, per-core CPU) karena kolom itu baru
    diketahui setelah `LogParser` (`src/log_parser.cpp`) jalan, tidak fixed di semua run.
  - `aggregate_runtime.py` — agregasi 18 skenario (6 model x 3 tracker, folder gabungan
    `<model>_<tracker>` dari `scripts/run_all_benchmark.sh`) menjadi `runtime_summary.csv`
    + `runtime_per_run.csv` + boxplot FPS per model/tracker. Model asli diturunkan dari
    field `tracker` di `run_info.txt` (bukan parsing string nama folder), karena tracker
    `nvdcf_perf` sendiri mengandung underscore sehingga split naif ambigu.
  - `accuracy_reference.csv` — **transkrip manual** angka akurasi dari
    `docs/05_accuracy_results.md` §5.1 (Params/GFLOPs/mAP50/mAP50-95/Precision/Recall
    untuk 4 model dasar). Dua varian `*_efficientnms` diberi angka akurasi yang sama
    dengan model basenya (asumsi: EfficientNMS_TRT cuma ganti lokasi eksekusi NMS, bukan
    kandidat deteksi) — ditandai eksplisit sebagai asumsi yang perlu diverifikasi ulang
    dengan dump deteksi as-deployed sebelum dipakai final di BAB IV.
  - `tradeoff_analysis.py` — gabung `runtime_summary.csv` + `accuracy_reference.csv` jadi
    `tradeoff_summary.csv`, scatter plot Pareto front (mAP50-95 vs FPS, vs Power), dan uji
    signifikansi opsional (`scipy.stats.ttest_ind`) lewat flag `--significance`.
  - `README.md` — urutan pemakaian singkat.
- **Tidak ada angka eksperimen yang dikarang.** Kedua script diverifikasi hanya dengan data
  CSV sintetis di direktori temp (`/tmp/...`, dibuat lalu dihapus lagi setelah verifikasi,
  tidak pernah disentuh ke `data/` atau `skripsi/eksperimen/`) — dicek: 6 skenario
  teragregasi benar, kolom rail daya berbeda antar run tertangani, satu run tanpa
  `hardware_analysis.csv` di-skip tanpa crash, dan kedua script berhenti dengan pesan jelas
  (bukan traceback atau file kosong) saat `data/benchmark/` belum ada sama sekali.
  `git status --short` dicek bersih sesudahnya untuk `data/` dan `skripsi/eksperimen/`.
- `skripsi/draft/BAB-4-Hasil-dan-Pembahasan.md` **tidak disentuh** — tetap semua TODO
  sampai `skripsi/eksperimen/runtime_summary.csv`/`tradeoff_summary.csv` benar-benar berisi
  data asli dari Jetson.

## 2026-08-15 06:39 WITA — percobaan download PDF jurnal dihentikan, sisa file rusak dibersihkan

- User minta lanjut opsi #2 (download PDF ke `journal/` + isi kolom File di
  `daftar-referensi.md`). Dijalankan 5 agent paralel (masing-masing 4 sitasi dari 20 sitasi
  di `daftar-referensi.md`) untuk cari & download PDF open-access legal. Semua 5 agent kena
  rate limit API (429) sebelum selesai — hasil sebagian saja yang sempat ter-download ke
  `journal/` sebelum terpotong.
- User kemudian minta **berhenti** — tidak usah lanjut download, user akan cari sendiri PDF
  jurnalnya nanti. Sesi ini tidak melanjutkan resume agent atau update kolom File di
  `daftar-referensi.md` (dibiarkan seperti semula, "—" semua), sesuai permintaan user.
- Housekeeping minimal saja yang dilakukan: 3 file hasil download yang **gagal/rusak**
  (ternyata bukan PDF asli, melainkan halaman "Access Denied"/error dari CDN penerbit
  MDPI, tersimpan dengan ekstensi `.pdf` yang menyesatkan) dihapus dari `journal/`:
  `14-shah-2025.pdf`, `15-suder-2023.pdf`, `16-tsai-hsieh-2025.pdf`.
- **Sisa 7 file yang berhasil ter-download dan valid** (diverifikasi `file` command = "PDF
  document", bukan halaman error) TETAP dibiarkan di `journal/` apa adanya, BELUM
  diverifikasi isinya cocok dengan sitasi yang dimaksud dan BELUM dicatat di kolom File
  `daftar-referensi.md` — perlu ditindaklanjuti (oleh AI sesi berikutnya atau user) sebelum
  dipakai: `02-bouazizi-2024.pdf`, `03-chaman-2025.pdf`, `04-chen-2022.pdf`,
  `09-neumann-2024.pdf`, `11-oro-2022.pdf`, `12-ruizbarroso-2025.pdf`, `17-xie-2024.pdf`,
  `20-janapareddi-2022.pdf` (8 file, bukan 7 — dikoreksi jumlahnya).
- Belum ter-download sama sekali (bukan gagal, tapi memang belum sempat dicoba/dilaporkan
  karena rate limit): sitasi #1, #5, #6, #7, #8, #10, #13, #18, #19.

## 2026-08-15 00:57 WITA — isi spesifikasi hardware Jetson Orin Nano di BAB II §2.2.7

- Item 3 dari sesi 2026-08-14 (yang sengaja di-skip) dikerjakan: mengisi tabel spesifikasi
  resmi Jetson Orin Nano di `draft/BAB-2-Tinjauan-Pustaka.md` §2.2.7, sebelumnya ditandai
  `[VERIFIKASI]` kosong karena beda tergantung SKU (4GB/8GB) dan mode daya.
- **Identifikasi SKU**: user awalnya menjawab unit yang dipakai adalah **8GB**, tapi mode daya
  maksimum yang dilihat via `nvpmodel -q` di device adalah **10W**. Dicek silang ke datasheet
  resmi NVIDIA (`WebSearch` + baca langsung PDF datasheet DS-11105-001 v1.1 dan v1.5 via
  `WebFetch`+`Read`): mode 10W ternyata **tidak ada** pada SKU 8GB (yang hanya 7W/15W/25W),
  dan justru cocok persis dengan SKU **4GB** (7W/10W/25W-MAXN_SUPER). Dikonfirmasi ke user,
  dan user setuju unit yang dipakai adalah **4GB**, bukan 8GB seperti jawaban awal — koreksi
  ini penting karena CUDA core/TOPS/RAM beda jauh antara kedua SKU.
- Karena mode maksimum yang terbaca 10W (bukan 25W), disimpulkan juga bahwa unit berjalan pada
  firmware **default/non-`MAXN_SUPER`** — dicatat di draf karena mempengaruhi batas atas
  performa yang jadi acuan BAB IV nanti.
- §2.2.7 ditulis ulang: paragraf baru menjelaskan identifikasi SKU 4GB dari bacaan `nvpmodel`,
  tabel spesifikasi resmi (GPU: 512 CUDA core/16 Tensor core; AI: 10/20 TOPS dense/sparse
  default, 17/34 TOPS jika `MAXN_SUPER`; CPU: 6-core Cortex-A78AE; memori: 4GB LPDDR5 64-bit;
  mode daya 7W/10W/25W), semua angka dari datasheet resmi NVIDIA (DS-11105-001_v1.5, Desember
  2024), bukan tebakan. `[VERIFIKASI]` baru ditambahkan: penulis disarankan konfirmasi ulang
  SKU via `cat /proc/device-tree/model` langsung di device saat tersedia lagi, karena
  identifikasi sesi ini murni dari inferensi mode daya, bukan inspeksi fisik/software langsung.
- `journal/daftar-referensi.md` ditambah baris **#23**: sitasi datasheet NVIDIA (2024) —
  dokumen dibaca langsung (bukan hanya cuplikan pencarian), diakses via mirror karena tautan
  `developer.nvidia.com` langsung tidak berhasil diverifikasi (`WebFetch` gagal/`ECONNRESET`).

## 2026-08-14 23:58 WITA (lanjutan) — koreksi klaim status tracking

- Saat menjawab pertanyaan user ("apakah tracking perlu dipertahankan atau dihapus"), dicek
  ulang `data/` di root proyek: hanya ada `data/input/` (video sampel), **tidak ada**
  `data/benchmark/` sama sekali. Ini mengoreksi klaim di `draft/BAB-1-Pendahuluan.md` §1.2
  ("sudah terimplementasi & sudah diuji di Orin Nano") yang ternyata melebih-lebihkan status
  — yang benar: infrastruktur (config tracker + runner `run_all_benchmark.sh`) **sudah
  lengkap**, tapi eksekusi 18 skenario dan pengumpulan datanya **belum pernah dilakukan**,
  sama seperti status RM1/RM2 baseline di `docs/06_runtime_results.md` ("🔲 Belum diisi").
  Diperbaiki di `draft/BAB-1-Pendahuluan.md` §1.2 `[VERIFIKASI]` supaya tidak mengklaim
  pekerjaan yang belum benar-benar terjadi (prinsip PANDUAN-AI.md: jangan mengarang status).

## 2026-08-14 23:58 WITA

- Permintaan user: hapus sitasi Wu dkk. (2024) dari Daftar Pustaka (bukan sekadar ditandai
  tidak valid seperti sesi sebelumnya).
- `journal/daftar-referensi.md`: baris #17 (Wu) dihapus permanen; baris #18–23 dinomori ulang
  jadi #17–22. Ditambah catatan penghapusan di bagian atas tabel (di bawah blockquote existing)
  yang menjelaskan alasan (DOI Crossref terbukti milik artikel lain, tidak berhubungan) dan
  merujuk balik ke log ini + histori Git sebagai audit trail permanen.
- `draft/BAB-2-Tinjauan-Pustaka.md`: paragraf di §2.1.1 yang sebelumnya berjudul "Catatan
  penting — sitasi tidak valid" diubah jadi "Catatan penghapusan sitasi" (bahasa lampau,
  sudah dihapus, bukan lagi rekomendasi). Bagian "Catatan referensi jurnal lain" diperbarui:
  hitungan sitasi disesuaikan dari "20 sitasi" jadi "19 sitasi tersisa dari 20 sitasi asli",
  status Wu diubah dari "Dikonfirmasi TIDAK VALID" jadi "DIHAPUS — terkonfirmasi tidak valid".
- Tidak ada perubahan substansi lain — sitasi Wu memang tidak pernah dipakai untuk mendukung
  klaim apa pun di draf manapun sejak awal, sehingga penghapusan ini murni bibliografis.
- **Catatan untuk penulis**: sitasi ini bagian dari Daftar Pustaka yang sudah disetujui di
  seminar proposal — tetap disarankan menginformasikan penghapusan ini ke dosen pembimbing
  (bawa bukti verifikasi Crossref di entri log 2026-08-14 23:50 WITA), walau secara teknis
  tidak ada isi ilmiah skripsi yang berubah akibat penghapusan ini.

## 2026-08-14 23:50 WITA

- Permintaan user: lanjutkan menyelesaikan poin `[VERIFIKASI]` di BAB I-III (dari 20 poin yang
  teridentifikasi sebelumnya), difokuskan ke item yang bisa diverifikasi tanpa akses hardware
  Jetson dan tanpa perlu persetujuan dosen pembimbing dulu.
- **Temuan penting — sitasi Wu dkk. (2024) tidak valid**: dicek langsung via Crossref API resmi
  (`https://api.crossref.org/works/10.3390/s24031023`). DOI yang dikutip Daftar Pustaka proposal
  untuk "Road object detection based on improved YOLOv8 for real-time traffic scenarios"
  ternyata milik artikel yang sama sekali berbeda: "Railway Catenary Condition Monitoring: A
  Systematic Mapping of Recent Research" (Chen, Frøseth, Derosa, Lau, & Rønnquist, 2024,
  *Sensors* 24(3), 1023) — topik pemantauan kabel listrik kereta api, tidak berhubungan dengan
  YOLOv8/deteksi objek. Pencarian tambahan (search_web, beberapa variasi kata kunci judul) juga
  tidak menemukan artikel berjudul tersebut oleh penulis manapun bernama Wu. Kesimpulan: sitasi
  ini kemungkinan besar salah kutip/DOI salah tempel pada proposal asli, bukan sekadar "belum
  ditemukan" seperti status sebelumnya. Diperbarui di 3 tempat: `draft/BAB-2-Tinjauan-Pustaka.md`
  (paragraf di §2.1.1 + ringkasan status di "Catatan referensi jurnal lain"), dan
  `journal/daftar-referensi.md` baris #17 (dicoret + ditandai TIDAK VALID). Rekomendasi ke
  penulis: informasikan temuan ini ke dosen pembimbing (bawa bukti verifikasi Crossref) sebelum
  menghapus resmi dari Daftar Pustaka final, karena sitasi ini bagian dari Daftar Pustaka yang
  sudah disetujui di seminar proposal.
- **Verifikasi sitasi NvDCF/NvSORT (Bab II §2.2.6) — selesai**: kedua sumber resmi NVIDIA
  dibuka & dibaca langsung (`fetch`) hari ini. Dokumentasi *Gst-nvtracker* dan NVIDIA Developer
  Blog ("State-of-the-Art Real-Time Multi-Object Trackers with DeepStream SDK 6.2", 19 April
  2023, penulis **Paul Shin & Fangyu Li** — bukan atribusi organisasi generik "NVIDIA") keduanya
  dikonfirmasi berisi kutipan yang sudah dipakai di draf (NvSORT "lightweight...competitively
  accurate"/"does not involve any pixel data processing"; NvDCF "best accuracy and robustness").
  `[VERIFIKASI]` di `draft/BAB-2-Tinjauan-Pustaka.md` §2.2.6 diperbarui: status jadi terverifikasi
  dengan tanggal akses, plus draf format sitasi APA gaya kampus (ikut pola di
  `referensi-skripsi/`, organisasi/penulis sebagai "tahun" + judul + URL). `journal/daftar-referensi.md`
  baris #23 dikoreksi dari "NVIDIA (2023)" menjadi "Shin, P., & Li, F. (2023)" karena blog
  tersebut punya penulis individu tercantum; baris #22 ditambah info tanggal verifikasi & URL.
- `draft/BAB-1-Pendahuluan.md` §1.1: catatan `[VERIFIKASI]` yang menyebut "daftar pustaka...
  belum disalin ke `../journal/daftar-referensi.md`" sudah usang (transfer 20 sitasi sudah
  selesai sejak entri 11:55 WITA) — diperbarui supaya tidak menyesatkan pembaca draf berikutnya.
- `draft/BAB-2-Tinjauan-Pustaka.md` header status (baris atas): kalimat "Sitasi jurnal individual
  belum disalin..." juga diperbarui jadi "sudah disalin", konsisten dengan status riil.
- **Item yang sengaja TIDAK dikerjakan sesi ini** (di luar scope yang bisa diverifikasi dari
  sandbox ini, perlu dilanjutkan penulis):
  - Spesifikasi resmi Jetson Orin Nano (Bab II §2.2.7, Bab III §3.2.1) — sengaja dikosongkan
    sesuai keputusan eksplisit user 2026-08-14 12:05 WITA ("akan ditulis manual sendiri nanti"),
    tidak diubah.
  - Versi software (GStreamer/CUDA/JetPack) di Bab III §3.2.2 — butuh akses langsung ke
    perangkat Jetson fisik yang dipakai untuk pengujian, tidak tersedia di sandbox ini.
  - Hyperparameter training (Bab III §3.2.3 / `docs/02_dataset_and_training.md`) — butuh
    `args.yaml` dari notebook Kaggle penulis; dicek tidak ada salinannya di repo ini.
  - Bacaan mendalam 7 jurnal di Bab II §2.1.3 (Choi, Nigade, Zhang, Ruiz-Barroso, Suder,
    Seyfipoor, Shah) — di luar scope permintaan sesi ini, tetap berstatus klaim tematik.
  - Konfirmasi ke dosen pembimbing soal penggantian rumusan masalah #3 (DLA → tracking) —
    keputusan non-teknis, tetap jadi tanggung jawab penulis.

## 2026-08-14 22:53 WITA

- Permintaan user: gabungkan bab yang sudah selesai (BAB I-III) jadi satu file `.docx`,
  bahasa akademis, sitasi minimal tahun 2022 (yang lebih lama diganti/dihapus). BAB IV-V
  tidak disertakan karena masih skeleton TODO kosong.
- Cek sitasi: `BAB-1-Pendahuluan.md` sudah seluruhnya ≥2022, tidak ada perubahan. Di
  `BAB-2-Tinjauan-Pustaka.md` ditemukan 2 sitasi sebelum 2022 yang **sudah diperbaiki
  sebelumnya** (bukan di sesi ini) menjadi Oro dkk. (2022) dan Janapa Reddi dkk. (2022,
  MLPerf Mobile) — diverifikasi ulang saat re-read, tidak perlu perubahan tambahan.
- Dibuat folder `draft/_build/` berisi versi "clean" tiap bab untuk keperluan render docx
  (bukan pengganti file sumber di `draft/`, sumber asli tidak diubah):
  - `_build/BAB-1.clean.md` — hapus header status draf & seksi "Judul (revisi)", hapus
    blockquote `[VERIFIKASI]`, §1.6 (Sistematika Penulisan) diisi prosa singkat pengganti
    stub TODO (menjelaskan bahwa seksi ini akan disusun setelah BAB I-V lengkap — bukan
    fabrikasi isi, hanya penjelasan status).
  - `_build/BAB-2.clean.md` — hapus header status & seksi "Catatan referensi jurnal lain",
    rapikan markup `[VERIFIKASI]` jadi kalimat naratif yang tetap jujur soal status
    verifikasi (tidak menghapus substansi disclosure, hanya mengubah format markup jadi
    prosa akademis).
  - `_build/BAB-3.clean.md` — sama seperti di atas; diagram Mermaid pipeline di §3.4
    (tidak bisa dirender langsung ke docx tanpa pandoc/mermaid-cli yang tidak tersedia di
    environment ini) dikonversi manual jadi deskripsi prosa 5 langkah pipeline yang setara
    isinya dengan diagram asli.
  - Prinsip yang dipakai di seluruh proses "cleaning": catatan `[VERIFIKASI]`/status
    pending tidak pernah dihapus diam-diam — dipindahkan utuh ke lampiran khusus di akhir
    dokumen gabungan ("CATATAN VERIFIKASI DAN TINDAK LANJUT PENULIS", 9 butir berprioritas
    tinggi/sedang/rendah) supaya isi BAB I-III terbaca sebagai naskah akademis yang mengalir,
    tanpa menyembunyikan bagian yang memang belum tuntas.
- Dibuat script `_build/generate_docx.py` (python-docx): parser Markdown→blok, tokenizer
  format inline (bold/italic/inline-code), title page, TOC (field otomatis, perlu
  klik-kanan Update Field di Word), heading H1-H3, tabel, dan lampiran verifikasi. Styling:
  Times New Roman 12pt, spasi 1.5, indent baris pertama 1.25cm, rata kanan-kiri, margin
  4/4/3/3 cm (kiri/atas/kanan/bawah).
- Jalankan via venv terisolasi `.venv-docx` (dibuat ulang karena sempat hilang; PEP 668
  externally-managed-environment mencegah `pip install` langsung ke sistem).
- Output: `draft/Skripsi-Gabungan-BAB-I-III.docx` — diverifikasi struktur hasil (192
  paragraf, 5 tabel, 3 heading H1 / 15 H2 / 18 H3, tidak ada sisa markup `[VERIFIKASI]`
  atau artefak Markdown yang tidak terkonversi).
- File `_build/` dibiarkan ada sebagai build artifact (bukan file final, tidak perlu
  disentuh manual — akan di-regenerate ulang oleh script jika bab sumber berubah).

## 2026-08-14 12:37 WITA

- User minta lanjut ke BAB III ("ya, mulai dari bab 3 dulu") setelah menu prioritas
  ditawarkan. Draf penuh `draft/BAB-3-Metodologi-Penelitian.md` ditulis (sebelumnya skeleton
  TODO), digrounding ke `docs/01-05`, `docs/08`, `CMakeLists.txt`, `scripts/build.sh`,
  `scripts/run_benchmark.sh`, `scripts/run_all_benchmark.sh`, `scripts/prepare_eval_video.sh`,
  `utils/eval_map/eval_deepstream_map.py`, dan isi folder `config/` — bukan generalisasi buku
  teks metodologi.
- §3.1 Jenis dan Pendekatan Penelitian: eksperimental kuantitatif, 3 sumbu perbandingan
  (baseline RM1, NMS standar-vs-paralel RM2, tracker RM3) masing-masing dipetakan eksplisit ke
  satu rumusan masalah.
- §3.2 Alat dan Bahan: tabel hardware (spek Jetson tetap dikosongkan sesuai keputusan user
  12:05 WITA — diberi `[VERIFIKASI]` merujuk ke keputusan itu), software (DeepStream 7.1,
  C++17/CMake, GStreamer/CUDA versi sistem — ditandai `[VERIFIKASI]` karena tidak dipin
  eksplisit di `CMakeLists.txt`), dataset (1010 gambar val, identik proposal), tabel 6
  konfigurasi model (dengan catatan RM2 hanya berlaku untuk pasangan YOLOv8n/YOLOv9t karena
  YOLOv10n/YOLO26n sudah NMS-free arsitektural — YOLOv8n-COCO dicatat eksplisit sebagai
  sanity-check saja, bukan komparator), tabel 3 tracker, dan daftar tooling otomasi
  (`build.sh`, `run_benchmark.sh`, `run_all_benchmark.sh`, `prepare_eval_video.sh` +
  `eval_deepstream_map.py`).
- §3.3 Tahapan Penelitian: 7 langkah eksplisit dari build sampai analisis, dengan rekomendasi
  repetisi 3-5x + cooldown termal dari `docs/04_benchmark_protocol.md`.
- §3.4 Arsitektur/Desain Pipeline: diagram mermaid pipeline (ditambah cabang probe
  `--dump-detections` yang belum ada di diagram asli `docs/01_*`) + 5 poin rasionalisasi
  desain (DeepStream vs OpenCV manual, model nano-class, FP16, tegrastats, logger thread
  terpisah) diringkas dari `docs/01_scope_and_architecture.md` §1.4.
- §3.5 Skenario Pengujian: 4 skenario (RM1/RM2/RM3 + akurasi as-deployed FP16 pendukung).
  **Temuan penting saat riset grounding**: `utils/eval_map/eval_deepstream_map.py` dan
  `scripts/prepare_eval_video.sh` sudah **ada dan lengkap di kode** (dicek via `git log` —
  komit "feat: add as-deployed detection dump for FP16 verification" — dan dibaca isi
  filenya), berbeda dari asumsi rencana FP16-verification sebelumnya (plan mode) yang
  mengira script ini belum ditulis. `src/main.cpp` juga sudah punya flag `--dump-detections`
  lengkap dengan probe async (dicek via `grep`). Poin 4 di §3.5 menulis status ini secara
  jujur: infrastruktur kode **sudah selesai**, tapi eksekusi nyata (ekspor dataset dari
  Kaggle, run di Jetson, verifikasi visual bbox, update `docs/05`/`docs/08`) **belum
  dilakukan** — ditulis sebagai "rencana pengujian pendukung", bukan hasil yang sudah ada,
  supaya BAB III tidak mengklaim sesuatu yang belum benar-benar dijalankan.
- §3.6 Kriteria Evaluasi: tabel per-sumbu (RM1 FPS+latensi p95/p99, RM2 delta FPS/Lat_Infer,
  RM3 murni efisiensi komputasi dengan cross-reference eksplisit ke argumentasi MLPerf Bab II
  §2.2.6, akurasi pendukung sebagai kriteria pass/fail bukan variabel dibandingkan). Ambang
  angka pasti (target FPS kamera, ambang delta mAP "dapat diabaikan") sengaja ditandai
  `[VERIFIKASI]` — dikunci penulis setelah data aktual tersedia, supaya kriteria tidak
  disesuaikan setelah melihat hasil.
- **Belum dilakukan**: eksekusi eksperimen itu sendiri (Bab III baru berisi rencana/metodologi,
  bukan hasil) — hasil aktual akan mengisi Bab IV setelah pengujian di Jetson selesai.

## 2026-08-14 12:22 WITA

- User mengonfirmasi ulang (2x) bahwa judul memakai "untuk" bukan "pada" — sudah sesuai,
  tidak ada perubahan tambahan diperlukan (lihat entri 12:05 WITA).
- User memutuskan: lanjut tulis draf dengan rumusan masalah #3 (pengganti DLA) apa adanya,
  **belum** dikonfirmasi ke dosen pembimbing — akan direvisi nanti di bimbingan kalau tidak
  disetujui. Syarat dari user: argumentasi kenapa evaluasi *tracking* tidak perlu metrik
  kualitas (MOTA/IDF1/ID switch) harus diperkuat, supaya tidak diminta menambah metrik baru
  saat bimbingan.
- Riset (via agent + `WebSearch`, sumber diverifikasi bukan ditebak) menghasilkan 3 argumen
  baru yang ditambahkan ke draf:
  1. Rumusan masalah #3 sendiri sudah eksplisit bertanya "efisiensi komputasi" (§1.2) —
     bukan pembatasan post-hoc, tapi cakupan pertanyaan yang memang didefinisikan sejak awal.
  2. Sumbu akurasi-vs-komputasi pada NvDCF/NvDCF_perf/NvSORT sudah didesain vendor sendiri
     (dikonfirmasi lewat dokumentasi resmi *Gst-nvtracker* plugin manual + NVIDIA Developer
     Blog "State-of-the-Art Real-Time Multi-Object Trackers with DeepStream SDK 6.2") —
     preset `NvDCF_perf` adalah tuning resmi NVIDIA pada sumbu yang sama.
  3. Preseden metodologis: **MLPerf Inference** (Reddi dkk., 2020) memperlakukan akurasi
     sebagai kriteria kelulusan tetap, bukan variabel yang dibandingkan — struktur yang sama
     dengan pendekatan penelitian ini.
  - Ketidaktersediaan dataset *tracking* berlabel (KITTI Tracking, MOTChallenge) tetap
    dipertahankan sebagai alasan pendukung tambahan, bukan lagi alasan tunggal.
- Perubahan file:
  - `draft/BAB-2-Tinjauan-Pustaka.md` §2.2.6: ditambah 3 paragraf baru berisi argumentasi di
    atas beserta sitasi, plus `[VERIFIKASI]` yang menjelaskan kutipan NVIDIA disintesis dari
    2 sumber (bukan kutipan verbatim satu kalimat) dan perlu dicek gaya sitasi non-jurnal.
  - `draft/BAB-1-Pendahuluan.md` §1.5 poin 5: argumentasi diperkuat dengan cross-reference ke
    Bab II §2.2.6; catatan `[VERIFIKASI]` di bawahnya diupdate menjelaskan konteks (rumusan
    masalah #3 belum dikonfirmasi dosen, argumentasi disiapkan sebagai antisipasi).
  - `journal/daftar-referensi.md`: ditambah 3 entri baru (#21 MLPerf Inference/Reddi dkk.
    2020, #22 Gst-nvtracker plugin manual NVIDIA, #23 NVIDIA Developer Blog DeepStream SDK
    6.2) — ketiganya sumber baru di luar 20 sitasi Daftar Pustaka proposal asli, dipakai
    khusus untuk argumentasi di §2.2.6.

## 2026-08-14 12:05 WITA

- User memberi 3 keputusan atas pertanyaan sebelumnya:
  - **Judul**: kata sambung "untuk" dikonfirmasi benar (bukan "pada"). `[VERIFIKASI]` di
    `draft/BAB-1-Pendahuluan.md` "## Judul (revisi)" dihapus, diganti catatan konfirmasi —
    judul dianggap final.
  - **Sitasi Tsai**: user minta pakai versi yang ditemukan AI via pencarian DOI (**Tsai &
    Hsieh**, bukan "Tsai, Hsu, & Lin" seperti di Daftar Pustaka proposal). Dikoreksi di 3
    tempat: `draft/BAB-2-Tinjauan-Pustaka.md` §2.1.1 (paragraf utama + "Catatan referensi
    jurnal lain") dan `journal/daftar-referensi.md` baris #16 — semua `[VERIFIKASI]` terkait
    ini dihapus, diganti catatan "dikonfirmasi penulis 2026-08-14".
  - **Spek Jetson Orin Nano**: user tidak minta diisi — akan ditulis manual sendiri nanti.
    Tidak ada perubahan di §2.2.7 (memang sudah sengaja dikosongkan sejak draf awal,
    keputusan ini hanya mengonfirmasi pendekatan yang sudah ada, bukan tugas baru).
- User menambahkan file rujukan skripsi baru ke `referensi-skripsi/`: *"revisi AUDY
  FEBRYANTI - SISTEM DETEKSI DAN HITUNG OBJEK KENTANG MULTI-SKALA SECARA REAL-TIME"*
  (Prodi Teknik Informatika, Fakultas Teknik, Universitas Hasanuddin, 2026, NIM D121211005).
  Dicek ulang: PDF terbaca, halaman judul lengkap (judul, penulis, prodi/fakultas/kampus
  sama dengan target skripsi ini, tahun 2026), sesuai kriteria di
  `referensi-skripsi/README.md` (rujukan struktur bab & gaya penulisan, prodi/kampus sama).
  Tema (deteksi objek real-time) juga relevan sebagai pembanding gaya penulisan BAB
  metodologi/hasil. Belum dibaca isi penuh (BAB I-V) — baru verifikasi halaman judul.

## 2026-08-14 11:55 WITA

- Lanjutan permintaan user "1 dan 2" (baca/rangkum 10 sitasi tambahan yang sebelumnya cuma
  klaim tematik + pindahkan 20 sitasi ke `journal/daftar-referensi.md`).
- **Item 1** — `draft/BAB-2-Tinjauan-Pustaka.md` §2.1.1 ditambah 2 paragraf baru hasil
  `WebSearch` (bukan tebakan):
  - Tsai & Hsieh (2025, DOI `10.3390/electronics14214275`) — sistem *collision warning*
    real-time berbasis YOLOv8n + stereo vision SGBM, 112 FPS, model 4,5 MB. Ditandai
    `[VERIFIKASI]`: nama penulis asli di DOI tsb adalah **Tsai & Hsieh**, sedangkan Daftar
    Pustaka proposal menulis "Tsai, Hsu, & Lin" — kemungkinan salah kutip di proposal asli,
    perlu dicek user ke PDF jurnal langsung sebelum diputuskan versi mana yang benar (belum
    diubah di `journal/daftar-referensi.md`, sengaja dibiarkan sesuai teks proposal supaya
    tidak "memperbaiki" sesuatu yang belum terkonfirmasi).
  - Wu dkk. (2024, DOI `10.3390/s24031023`) — artikel **tidak ditemukan** lewat pencarian;
    sengaja **tidak** ditulis ringkasan isi (hanya dicatat sebagai belum terverifikasi) supaya
    tidak mengarang.
  - Subbagian baru **§2.1.3 "Efisiensi Real-Time dan Penjadwalan pada Edge Device"**
    ditambahkan, merangkum klaim tematik (bukan ringkasan metodologi independen) untuk Choi
    2024, Nigade 2024, Zhang 2024, Ruiz-Barroso 2025, Suder 2023, Seyfipoor 2026, Shah 2025 —
    reuse dari klaim yang sudah sah dipakai di `BAB-1-Pendahuluan.md` §1.1 (sudah bersumber
    dari proposal asli), bukan riset baru. Diberi `[VERIFIKASI]` eksplisit bahwa ini baru
    setingkat klaim tematik, bukan ringkasan metodologi/hasil.
  - §2.1.3 lama ("Konteks Umum ADAS dan Keselamatan") di-renumber jadi **§2.1.4**, ditambah
    kalimat cross-reference ke BAB I §1.1 ¶1 untuk sitasi Costa (2025).
  - Bagian "Catatan referensi jurnal lain" ditulis ulang jadi kategorisasi status verifikasi
    ke-20 sitasi (sudah ada ringkasan tabel / baru klaim tematik / diverifikasi via DOI sesi
    ini / belum terverifikasi sama sekali).
- **Item 2** — `journal/daftar-referensi.md` diisi penuh dengan ke-20 sitasi dari Daftar
  Pustaka proposal (kolom File = "—" karena belum ada PDF yang didownload ke folder ini;
  kolom Relevansi menunjuk ke BAB I §1.1 paragraf terkait dan/atau BAB II §2.1.x). Baris #16
  (Tsai) dan #17 (Wu) diberi catatan `[VERIFIKASI]` merujuk ke BAB II §2.1.1 untuk detail
  masalah sitasi di atas.
- Catatan penutup §2.1 di BAB-2 diupdate: kalimat yang sebelumnya menyatakan transfer ke
  `daftar-referensi.md` "belum dilakukan" diganti jadi "sudah dipindahkan" (kolom File masih
  kosong, itu status terpisah dari transfer sitasinya sendiri).
- Item 3 (isi spek hardware Jetson Orin Nano di §2.2.7) **tidak** dikerjakan pada sesi ini —
  user secara eksplisit hanya minta "1 dan 2".

## 2026-08-14 06:11 WITA

- User minta cek ulang draf BAB I/II/III — AI mereview dan menemukan inkonsistensi:
  frasa "*custom* NMS berbasis TensorRT plugin (EfficientNMS)" di 4 tempat BAB I (rumusan
  masalah #2, tujuan #2, manfaat teoritis, batasan #2) kontradiktif dengan catatan
  `[VERIFIKASI]` di bawahnya sendiri, yang menjelaskan bahwa implementasi ini justru
  **bukan** custom (plugin siap pakai vendor, bukan kernel CUDA ditulis dari nol).
- User setuju perbaikan: "custom NMS" → **"NMS paralel"** di keempat tempat tsb
  (`draft/BAB-1-Pendahuluan.md` §1.2 poin 2, §1.3 poin 2, §1.4 manfaat teoritis, §1.5
  poin 2), termasuk penyesuaian kalimat rumusan masalah #2 supaya tidak lagi redundan
  (sebelumnya "...NMS ... untuk *parallel* Non-Maximum Suppression dapat..." — frasa
  "untuk parallel Non-Maximum Suppression" dihapus karena kata "paralel" sudah disebut
  di depan). Catatan `[VERIFIKASI]` di §1.2 poin 2 diupdate untuk menjelaskan alasan kata
  "custom" sengaja dihindari.
- Dicek BAB II & BAB III: tidak ada pemakaian "custom" yang bermasalah serupa — 3
  kemunculan "custom" di BAB II semuanya merujuk ke diagram/kernel CUDA custom pada
  **proposal asli** (kontras yang memang dimaksud), bukan menyebut implementasi final
  sebagai custom. Tidak ada perubahan di BAB II/III.

## 2026-08-14 06:05 WITA

- Ditulis draf penuh `draft/BAB-2-Tinjauan-Pustaka.md` (sebelumnya skeleton kosong).
- §2.1 Penelitian Terkait: dirangkum dari tabel "Penelitian Terkait" (10 jurnal) dan Daftar
  Pustaka (20 sitasi total) di `Proposal/Proposal Final Perdi - AGX Orin ADAS-1.pdf` (dibaca
  ulang halaman 7-9 dan 14-16 untuk mengambil isi tabel + daftar pustaka lengkap secara
  akurat, bukan dari ingatan/ringkasan sesi sebelumnya). Dikelompokkan jadi 3 klaster tema:
  deteksi objek real-time ADAS di edge device, optimasi/akselerasi NMS, konteks umum
  keselamatan ADAS — plus catatan celah literatur (tidak ada studi pembanding efisiensi
  tracker NvDCF vs NvSORT di edge device) sebagai landasan rumusan masalah #3.
- §2.2 Landasan Teori: 8 sub-bagian (ADAS/perception layer, DeepStream SDK, TensorRT &
  presisi FP32/FP16/INT8, arsitektur YOLO, NMS & EfficientNMS_TRT, tracking NvDCF/NvSORT,
  Jetson Orin Nano, metrik evaluasi) — sebagian digrounding ke
  `../../docs/01_scope_and_architecture.md` dan `../../utils/trt_efficientnms/README.md`
  supaya istilah teknis konsisten dengan implementasi nyata (bukan generalisasi buku teks).
  Spesifikasi hardware Jetson Orin Nano (CUDA core/RAM/TOPS) sengaja **tidak** diisi angka —
  ditandai `[VERIFIKASI]` karena beda tergantung SKU (4GB/8GB) dan mode daya, harus dicek ke
  datasheet resmi oleh penulis, bukan ditebak.
- §2.3 Kerangka Berpikir: narasi yang menghubungkan celah literatur (§2.1) ke 3 rumusan
  masalah (Bab I §1.2) sebagai evaluasi bertahap satu pipeline yang sama.
- **Belum dilakukan**: transfer 20 sitasi ke `journal/daftar-referensi.md` — file itu
  berfungsi sebagai index file PDF yang didownload ke `journal/`, dan sampai saat ini belum
  ada PDF jurnal individual yang didownload ke folder tersebut (hanya 1 file skripsi rujukan
  di `referensi-skripsi/`), jadi tabel itu sengaja tidak diisi supaya kolom "File" tidak
  merujuk ke file yang tidak ada. Perlu diisi begitu jurnal-jurnal itu benar-benar
  didownload.

## 2026-08-14 05:59 WITA

- User mengoreksi info: judul skripsi **sudah disetujui dosen pembimbing** untuk diganti
  dari "...Berbasis Jetson AGX Orin" menjadi bentuk generik "...Berbasis **Edge Device**"
  — bukan "...Berbasis Jetson Orin Nano" seperti yang sempat ditulis di draf BAB I pada
  entri sebelumnya (05:52 WITA). Ini keputusan judul yang terpisah dari keputusan
  rumusan-masalah-#3 (tracker), disampaikan belakangan oleh user.
- Dikoreksi `draft/BAB-1-Pendahuluan.md` bagian "## Judul (revisi)": judul diubah jadi
  "...Berbasis Edge Device", catatan `[VERIFIKASI]` diupdate untuk menjelaskan bahwa judul
  memakai istilah generik "Edge Device" sementara isi bab tetap menyebut Jetson Orin Nano
  sebagai perangkat uji konkret — ditandai perlu konfirmasi ke dosen soal kata sambung
  persis di judul ("pada" vs "untuk", dll) karena belum disampaikan user secara verbatim.
- Dicek `PANDUAN-AI.md` — baris judul di sana (`Judul: Optimasi Realtime Pipeline Nvidia
  Deepstream pada Aplikasi ADAS Berbasis Edge Device`) ternyata **sudah** memakai "Edge
  Device" sejak awal (ditulis berdasarkan permintaan awal user sebelum proposal PDF
  dibaca), jadi tidak perlu diubah — sudah konsisten dengan koreksi ini.

## 2026-08-14 05:52 WITA

- User menambahkan `Proposal/Proposal Final Perdi - AGX Orin ADAS-1.pdf` (proposal asli
  hasil seminar proposal, 16 halaman) ke `Proposal/`.
- Dibaca isi lengkap proposal: judul, rumusan masalah/tujuan/manfaat/batasan asli (3 poin,
  poin #3 = DLA vs GPU baseline), form revisi seminar proposal (dosen minta perangkat uji
  alternatif spek lebih rendah dari AGX Orin), langkah penelitian & diagram pipeline,
  daftar pustaka.
- Diskusi dengan user soal pengganti rumusan masalah #3 (DLA, tidak tersedia di Orin
  Nano). Sempat dipertimbangkan: FP16 vs FP32 precision, topk/IoU threshold tuning,
  perbandingan versi YOLO, perbandingan algoritma tracker.
- **Keputusan final**: rumusan masalah #3 diganti jadi perbandingan efisiensi komputasi
  algoritma tracking (NvDCF vs NvSORT vs NvDCF_perf), memakai infrastruktur yang sudah
  ada & sudah berjalan di Orin Nano (`scripts/run_all_benchmark.sh`, 18 skenario = 6
  model × 3 tracker). Di-scope **hanya efisiensi komputasi** (FPS, `Lat_Tracker_ms` dari
  `fps.csv`, resource usage dari `tegrastats`/`hardware_analysis.csv`) — evaluasi
  kualitas/akurasi tracking (ID switch, MOTA/IDF1) eksplisit di luar scope karena butuh
  dataset tracking berlabel terpisah (KITTI Tracking) yang belum tersedia. Alasan
  topk/IoU tuning dan perbandingan versi YOLO tidak dipakai sebagai rumusan masalah
  tersendiri: keduanya tumpang tindih/lebih pas jadi variabel eksperimen di dalam poin
  lain, bukan axis baru yang berdiri sendiri.
- Ditulis draf `draft/BAB-1-Pendahuluan.md` (latar belakang diadaptasi dari proposal asli
  dengan nama perangkat diganti; rumusan masalah, tujuan, manfaat, batasan direvisi penuh
  sesuai keputusan di atas; ditambah batasan baru poin 5 soal scope tracking) dan
  `draft/BAB-3-Metodologi-Penelitian.md` (§3.2 alat/bahan, §3.5 skenario pengujian, §3.6
  kriteria evaluasi diupdate untuk axis tracking).
- Setiap bagian yang diadaptasi/direvisi ditandai `[VERIFIKASI]` di BAB I — penulis masih
  perlu mengecek ulang sebelum dianggap final, terutama soal penamaan "custom NMS
  berbasis TensorRT plugin (EfficientNMS)" vs "custom CUDA logic" di proposal asli (beda
  dari yang digambarkan di diagram proposal — plugin siap pakai, bukan kernel CUDA
  ditulis dari nol).
- Update `PANDUAN-AI.md`: status keputusan DLA→tracking diubah dari "belum diputuskan"
  jadi "sudah diputuskan", dengan ringkasan keputusan.

## 2026-08-14 04:18 WITA

- Setup awal struktur folder `skripsi/` oleh asisten AI, sesuai permintaan user.
- Dibuat: `Proposal/`, `referensi-skripsi/`, `draft/` (skeleton 5 BAB), `journal/`
  (+ `daftar-referensi.md`), `eksperimen/`, `log/` (file ini).
- Dibuat `PANDUAN-AI.md` di root `skripsi/` sebagai catatan konteks & aturan kerja untuk AI.
- Dibuat skill Claude Code di `.claude/skills/skripsi-log/SKILL.md` supaya AI otomatis
  membaca `PANDUAN-AI.md` dan mengupdate log ini setiap mengerjakan sesuatu di `skripsi/`.
- Dicatat sebagai konteks penting (belum diputuskan): proposal lama pakai Jetson AGX Orin
  dengan tujuan penelitian implementasi DLA; implementasi final pakai Jetson Orin Nano
  (tidak ada DLA yang relevan), sehingga rumusan masalah/tujuan terkait DLA perlu
  dihapus atau diganti — keputusan ini belum diambil user, harus ditanyakan sebelum
  mengisi BAB I/BAB III secara final.
- Isi `draft/` BAB I-V masih skeleton (judul sub-bab saja, belum ada konten) karena
  `Proposal/` dan `referensi-skripsi/` masih kosong saat scaffolding ini dibuat.

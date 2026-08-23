# Log Perubahan — Folder `skripsi/`

File ini adalah **satu-satunya** file log untuk semua perubahan di dalam folder `skripsi/`.
AI (Claude) wajib menambah entri baru di sini setiap kali membuat/mengubah/menghapus file
di dalam `skripsi/` atau subfoldernya.

**Format entri**: entri terbaru ditaruh paling atas (di bawah baris ini), diawali
`## YYYY-MM-DD HH:MM (zona waktu)`, diikuti bullet list ringkas berisi apa yang berubah dan
kenapa (kalau relevan).

---

## 2026-08-23 22:55 WITA — Standardisasi "kotak" → *bounding box* di prosa BAB I–IV, berdasar riset referensi

- Permintaan user: cari referensi apakah skripsi/paper Indonesia menerjemahkan istilah
  seperti "NMS box count" ke bahasa Indonesia atau tidak, untuk memverifikasi aturan
  istilah teknis yang sudah diterapkan sebelumnya alih-alih menebak.
- Riset: dicek `skripsi/referensi-skripsi/` (skripsi Audy Febryanti — deteksi objek
  *real-time*, domain sangat relevan) dan satu skripsi Unhas sejenis (SSD *obstacle
  detection*, unduhan `repository.unhas.ac.id`). Temuan: *preprocessing*/*postprocessing*,
  *tracking* (kategori formal SOT/MOT), "skor keyakinan (*confidence score*)", "ambang
  batas (*threshold*)" — semua sudah konsisten dengan pola yang diterapkan pada entri log
  sebelumnya, tidak perlu revisi. Temuan baru: kedua referensi **tidak pernah** memakai
  kata "kotak" polos untuk *bounding box* di prosa berjalan — istilahnya digloss sekali
  ("kotak pembatas/*bounding box*") lalu selanjutnya konsisten memakai "*bounding box*"
  di setiap kemunculan berikutnya, diperlakukan sebagai istilah pinjaman tetap (mirip
  "piksel" yang tidak pernah diterjemahkan). Draf sebelumnya justru mencampur
  "*bounding box*"/"*box*"/"kotak" untuk konsep yang sama dalam satu paragraf yang sama
  (mis. BAB I §1.2.5).
- Diganti (bare "kotak"/"*box*" yang merujuk konsep *bounding box* deteksi, bukan
  kotak-diagram/boxplot): BAB I §1.2.4–1.2.5 (4 titik: "kotak redundan", "kandidat
  kotak", "seluruh *box*", "kotak"×3 di kalimat NMS); BAB II §2.5.4 (1 titik:
  "penyaringan kotak pembatas"), §2.5.5 (1 titik: "kotak per *frame*"), §2.6 (2 titik:
  "penskalaan-ulang kotak pembatas" ×2); BAB III §3.3.2 (1 titik: "batas jumlah kotak
  keluaran" — ini contoh persis "NMS box count" yang ditanyakan user); BAB IV saran #3
  (1 titik: "batas maksimum kotak keluaran").
- Sengaja **tidak** diubah: BAB II §2.5.4 baris 408–409 ("kotak merah"/"kotak hijau" —
  warna node diagram mermaid, bukan *bounding box*); BAB II §2.6 baris 625 ("kotak
  pembatas/*bounding box*" — gloss pertama yang memang sudah benar, dipertahankan
  sebagai titik pengenalan istilah); BAB III §3.2.1/§3.4.2 ("kotak-garis" = istilah
  boxplot, tidak berkaitan dengan *bounding box* deteksi).
- Verifikasi: `grep -n "kotak" skripsi/draft/BAB-*.md` menyisakan hanya 6 titik yang
  memang sengaja dipertahankan (di atas). Tidak ada perubahan pada skrip *chart* atau
  angka — murni istilah prosa.

## 2026-08-23 22:33 WITA — Perluas perbaikan istilah: "pelacakan"/"keluaran"/"pemrosesan" jadi *tracking*/*output*/*processing* saat menjadi nama tahap/metrik formal

- Permintaan lanjutan user setelah entri sebelumnya (pemaduan/tampilan): "yang
  lain juga, kayak pelacakan, keluaran dan pemrosesan, kata pra juga, dll yang
  kamu rasa itu bahasa teknis" — meminta perluasan aturan yang sama ke istilah
  lain, dengan penilaian diserahkan ke AI.
- Prinsip yang diterapkan (revisi dari entri sebelumnya, yang keliru
  menganggap pelacakan/keluaran/pemrosesan selalu aman dipertahankan):
  bedakan **penggunaan sebagai nama tahap/metrik formal** (mis. kolom Tabel
  3.2.3/3.4.1, nama tahap *pipeline* yang dirujuk berulang sebagai satuan
  pengukuran) — untuk kasus ini istilah asing/teknis dipakai (*tracking*,
  *output*, *pre-processing*, *post-processing*) — vs. **penggunaan naratif/
  deskriptif generik** (mis. "kotak keluaran" pada NMS, "mode keluaran" I/O,
  "algoritma pelacakan objek" sebagai nama umum tugas *computer vision*,
  "efisiensi pemrosesan piksel" pada penjelasan mekanisme tracker) — untuk
  kasus ini bentuk Indonesia tetap dipertahankan karena tidak merujuk metrik
  bernama tertentu.
- Diganti (khusus penggunaan formal di atas): "pelacakan"/"tahap pelacakan" →
  "*tracking*"/"tahap *tracking*"; "keluaran" (sebagai nama tahap *output*
  *pipeline*) → "*output*"; "pra-pemrosesan"/"pasca-pemrosesan"/
  "prapemrosesan"/"pascapemrosesan" → "*pre-processing*"/"*post-processing*";
  "dekode" → "*decode*" (satu daftar tahapan NVMM di BAB I).
- Lokasi yang diubah: BAB I (baris ~192, ~231, ~324), BAB II (baris ~364,
  ~485, ~613, ~660–664 definisi kanonik §2.7, ~694, ~708 tabel RM3), BAB III
  (header Tabel 3.4.1 + ~11 titik prosa di §3.2.3/§3.4.1/§3.4.2/§3.4.3
  termasuk caption Gambar 3.4), BAB IV (baris ~50, ~54, rangkuman RM2/RM3).
  Sengaja **tidak** diubah: "kotak keluaran" (jumlah kandidat NMS, BAB III/IV)
  dan "mode keluaran" (deskriptor I/O, BAB III) karena bukan nama tahap
  *pipeline* yang diukur; juga deskripsi mekanisme tracker di BAB II §2.5.5
  yang memakai "pemrosesan piksel" secara generik.
- `utils/benchmark_analysis/extra_plots.py`: `LATENCY_COMPONENTS` diubah dari
  "Tracking"/"Output" (sebelumnya "Pelacakan"/"Keluaran"), judul & label sumbu
  `plot_tracker_latency_comparison()` disamakan ("Latensi tahap tracking...",
  "Perbandingan Latensi Tahap Tracking..."). Grafik digenerate ulang lewat
  `.venv-thesis-plots/bin/python utils/benchmark_analysis/extra_plots.py` dan
  diverifikasi visual (Gambar 3.2 `latency_decomposition_stacked_bar.png`,
  Gambar 3.4 `tracker_latency_comparison.png`) — label legenda/sumbu baru
  terbaca jelas, tidak ada angka yang berubah.
- Verifikasi: `grep -n "tahap pelacakan\|tahap keluaran\|pelacakan (\*tracking\*)\|keluaran (\*output\*)\|pra-pemrosesan\|pasca-pemrosesan\|pascapemrosesan\|prapemrosesan" skripsi/draft/BAB-*.md` mengembalikan kosong di keempat bab.

## 2026-08-23 22:22 WITA — Perbaikan istilah "pemaduan"/"tampilan" jadi *multiplexing*/OSD di BAB I–IV

- Permintaan user: bahasa draf terlalu baku/kaku karena istilah teknis dipaksa
  di-Indonesiakan menjadi neologisme yang tidak lazim dipakai orang Indonesia
  (contoh yang diberikan: "pemaduan" untuk *multiplexing*). Aturan yang diminta:
  istilah teknis yang memang lazim di-Indonesiakan (inferensi, pelacakan,
  keluaran, pemrosesan, dst.) tetap dipertahankan; istilah yang masih asing
  dalam penggunaan sehari-hari orang Indonesia dibiarkan memakai bentuk
  asing/teknisnya (bercetak miring, mengikuti konvensi yang sudah ada).
- Diganti di seluruh draf: "pemaduan"/"pra-pemaduan" → "*multiplexing*"/
  "pra-*multiplexing*"; "tampilan"/"pra-tampilan" (sebagai nama tahap OSD)
  → "OSD"/"pra-OSD". Definisi kanonik istilah 7-tahap *pipeline* di BAB II
  §2.7 disederhanakan agar tidak lagi menyertakan gloss Indonesia yang
  redundan (mis. "pemaduan (*multiplexing*)" → cukup "*multiplexing*").
- Lokasi yang diubah: BAB I §1.5.3 (1 titik), BAB II §2.5.3 (nama elemen
  *nvstreammux* + label diagram mermaid) dan §2.7 (definisi kanonik), BAB III
  §3.2.3 (header Tabel 3.2.3 + 6 titik prosa terkait Gambar 3.2), §3.3.3, dan
  §3.4.2, BAB IV (2 titik pada rangkuman RM1/RM2). Istilah Indonesia lain yang
  sudah lazim (inferensi, pelacakan, keluaran, pemrosesan, pemetaan,
  pencuplikan, dll.) sengaja **tidak** diubah — sudah sesuai konvensi lazim.
- Label legenda grafik `latency_decomposition_stacked_bar.png` (Gambar 3.2) di
  `utils/benchmark_analysis/extra_plots.py` (`LATENCY_COMPONENTS`) disamakan
  dengan header Tabel 3.2.3 yang baru ("Pra-multiplexing", "Multiplexing",
  "Pra-OSD", "OSD"), lalu grafik digenerate ulang lewat
  `.venv-thesis-plots/bin/python utils/benchmark_analysis/extra_plots.py` agar
  konsisten dengan prosa — tidak ada angka yang berubah, murni label.

## 2026-08-22 20:22 WITA — BAB III: sisipkan gambar/diagram, tambah 3 grafik baru, perbaiki 2 grafik trade-off

- Konteks: `BAB-3-Hasil-dan-Pembahasan.md` sudah lengkap secara narasi tapi belum pernah memakai
  sintaks gambar Markdown — 4 plot yang sudah ada di `../eksperimen/plots/` hanya disebut samar di
  prosa, tidak pernah benar-benar disisipkan. User meminta bab ini dibuat komprehensif dengan
  gambar/diagram, dikonfirmasi via pertanyaan pilihan: (1) tambah 3 grafik baru dari data yang
  sudah ada, (2) regenerasi 2 grafik *trade-off* lama karena label titiknya tumpang-tindih.
- **Perbaikan label 2 grafik *trade-off* lama** (`utils/benchmark_analysis/tradeoff_analysis.py`,
  fungsi `plot_scatter`): logika `ax.annotate` polos penyebab tumpang-tindih diganti algoritma
  penghindaran-tabrakan label buatan sendiri (union-find klaster berbasis jarak Euclidean
  ternormalisasi + penyebaran vertikal + garis penunjuk tipis), tanpa dependensi baru. Grafik
  `tradeoff_map_vs_fps.png` dan `tradeoff_map_vs_power.png` diregenerasi di tempat — data/angka
  identik, hanya posisi label yang berubah.
- **3 grafik baru** dibuat lewat skrip mandiri baru `utils/benchmark_analysis/extra_plots.py` yang
  membaca langsung `../eksperimen/runtime_summary.csv` (bukan menambah fungsi ke `main()`
  `aggregate_runtime.py` seperti rencana awal — **deviasi dari rencana**: folder data mentah
  `data/benchmark/` yang dibutuhkan `aggregate_runtime.py --bench-root` untuk menjalankan ulang
  `discover_runs()` tidak tersedia di mesin pengembangan ini, sehingga skrip mandiri yang membaca
  CSV agregat yang sudah ada dipakai sebagai gantinya — sumber angka tetap sama, tidak ada data
  yang dikarang):
  - `latency_decomposition_stacked_bar.png` — dekomposisi latensi 7-komponen, 4 model *baseline*
    (tracker NvDCF), sumber data identik Tabel 3.2.3.
  - `tracker_latency_comparison.png` — latensi tahap pelacakan NvDCF vs. NvSORT (skala-y
    logaritmik), 6 model, sumber data identik Tabel 3.4.1.
  - `energy_per_frame.png` — estimasi energi per-*frame*, 6 model × 2 *tracker*, sumber data
    identik Tabel 3.4.3 kolom terakhir.
- **7 gambar disisipkan** ke `BAB-3-Hasil-dan-Pembahasan.md` dengan caption bold "**Gambar 3.N**"
  (pola baru, mengikuti gaya caption "**Tabel 3.4.x**" yang sudah ada), tiap gambar diikuti 1–2
  kalimat penunjuk (bukan analisis baru): Gambar 3.1 (boxplot FPS per model) di §3.2.1, Gambar 3.2
  (dekomposisi latensi) di §3.2.3, Gambar 3.3 (boxplot FPS per tracker) di §3.4.2, Gambar 3.4
  (perbandingan latensi tracker) di §3.4.1, Gambar 3.5 (energi per frame) di §3.4.3, Gambar 3.6–3.7
  (trade-off vs. FPS dan vs. daya, hasil regenerasi) di §3.6.1.
- **1 diagram mermaid baru** (tanpa caption "Gambar", konsisten gaya diagram arsitektur Bab II
  §2.5.3–§2.5.4) disisipkan di §3.1 setelah paragraf "Data mentah 60 *run* diagregasi...",
  menggambarkan alur 60 *run* → agregasi per-skenario/data mentah per-*run* → tabel/grafik
  *runtime* → digabung akurasi KITTI → rangkuman *trade-off*/grafik Pareto.
- Tidak ada angka eksperimen baru — seluruh grafik baru murni memvisualkan kolom yang sudah ada di
  `runtime_summary.csv`/`tradeoff_summary.csv`, sumber yang sama dipakai tabel-tabel yang sudah ada
  di bab ini. Diverifikasi: setiap PNG dibuka visual sebelum disisipkan, grep ulang bab untuk
  memastikan tidak ada nama file/identifier kode bocor ke prosa pembaca di luar blok gambar dan
  blockquote provenance (konsisten dengan aturan gaya akademis draf).

---

## 2026-08-22 19:03 WITA — Perluasan tulis-ulang gaya bahasa akademis ke seluruh draf (BAB I–IV)

- User mengonfirmasi bahwa perbaikan gaya "jangan ai slop" (lihat entri 13:32 di bawah,
  awalnya dibatasi hanya §2.6–§2.7) perlu diperluas ke seluruh draf skripsi ("Bab I etc.").
- Dikerjakan sendiri: `BAB-1-Pendahuluan.md` bagian "B. Latensi dan Persentil (P95) Latensi" —
  identifier metrik mentah (`Lat_Infer_ms`, `Lat_Tracker_ms`) dan nama elemen pipeline
  (`nvstreammux`) diganti frasa deskriptif akademis (2 kemunculan).
- Dikerjakan via 3 agen paralel (fork), masing-masing dibekali §2.6–§2.7 sebagai contoh gaya
  target dan instruksi eksplisit untuk tidak mengubah/menghapus angka hasil eksperimen apa pun:
  - `BAB-2-Metode-Penelitian.md` §2.1–§2.5 (termasuk §2.5.1–§2.5.5): ±30 edit — path config
    (`config/pgie_yolov8n_kitti.txt`, `tracker_nvdcf.yml`, dll.), path kode sumber
    (`src/main.cpp`, `src/efficientnms_parser.cpp`), path dokumentasi internal
    (`../../docs/...`), nama elemen GStreamer bertanda kutip siku, dan referensi silang antar-
    bab bergaya nama berkas markdown — semua diganti prosa akademis atau istilah dicetak
    miring. Dua diagram mermaid (§2.5.3, §2.5.4) turut dibersihkan dari label berkas literal.
    Kolom tabel "File config" (§2.5.2, §2.5.5) yang murni berisi path berkas dihapus karena
    tidak memuat data eksperimen; data model/parameter/GFLOPs pada tabel yang sama tetap utuh.
  - `BAB-3-Hasil-dan-Pembahasan.md` (§3.1–§3.6): seluruh identifier metrik mentah, nama
    skrip/berkas CSV/PNG, dan path dokumentasi internal diganti prosa/label tabel deskriptif.
    Seluruh nilai numerik, hash commit, dan catatan metodologis (termasuk penjelasan kaveat
    status penguncian *clock* yang tampak gagal padahal berhasil) diverifikasi tetap utuh.
  - `BAB-4-Kesimpulan-dan-Saran.md`: 14 edit — identifier metrik mentah, nama *plugin*/
    parameter konfigurasi, dan referensi silang antar-bab diganti prosa akademis; seluruh
    nilai numerik dan kesimpulan tetap utuh.
- Blockquote catatan provenans/status di bagian atas tiap berkas (BAB I, II, III) **tidak**
  disentuh — tetap dianggap metadata penulisan internal, bukan teks pembaca skripsi.
- Diverifikasi dengan sapuan grep di seluruh 4 berkas BAB: nol kemunculan tersisa untuk pola
  path relatif, nama berkas berformat kode, dan identifier metrik mentah di teks isi bab.

## 2026-08-22 13:32 WITA — Tulis-ulang gaya bahasa §2.6–§2.7 (BAB-2-Metode-Penelitian.md)

- Atas permintaan user ("jangan ai slop"), §2.6 Pengujian dan §2.7 Metrik Evaluasi
  ditulis ulang agar bergaya akademis murni: menghapus seluruh identifier kode mentah
  (mis. `Lat_Tracker_ms`, `Lat_Infer_ms`, `fps.csv`, `hardware_analysis.csv`) dan
  path relatif ke dokumentasi internal (`../../docs/...`) dari teks naratif — diganti
  dengan deskripsi deskriptif (mis. "latensi tahap pelacakan", "log utilisasi
  perangkat keras").
- Daftar *bullet* nama skrip di "Tooling otomasi pengujian" (`scripts/build.sh`,
  `scripts/run_benchmark.sh`, dst.) diringkas jadi paragraf naratif "Instrumentasi dan
  otomasi pengujian" yang menjelaskan fungsinya tanpa menyebut nama berkas.
- Referensi silang antarbab yang semula ditulis sebagai nama berkas markdown
  (`` `BAB-1-Pendahuluan.md` §1.6 ``) diubah ke gaya sitasi akademis biasa ("Bab I
  §1.6").
- Nama pustaka/perangkat lunak (tegrastats, pycocotools, Ultralytics) tetap disebut
  (perlu untuk reproduksibilitas metodologis) tetapi dideskripsikan secara naratif,
  bukan sebagai token kode.
- Tidak ada perubahan makna/isi metodologis — murni perubahan gaya penulisan agar
  sesuai konvensi akademis skripsi, bukan dokumentasi teknis/README.

## 2026-08-22 13:15 WITA — Pisahkan §2.6 Pengujian → §2.6 + §2.7 Metrik Evaluasi (BAB-2-Metode-Penelitian.md)

- Atas permintaan user, konten "Metrik yang diukur", paragraf metrik perangkat keras, dan
  "Kriteria evaluasi" (+ tabel) — semula teks *run-in* di dalam §2.6 Pengujian — dipindah
  dan direstrukturisasi menjadi subbab baru **§2.7 Metrik Evaluasi**, agar konsisten
  dengan pola penomoran subbab bernomor di tempat lain BAB II (§2.5.1–§2.5.5) dan
  memisahkan "prosedur pengujian" (§2.6: *tooling*, skenario) dari "metrik & kriteria
  penilaian" (§2.7).
- §2.7 dipecah jadi dua sub-kelompok: "Metrik performa *runtime* dan perangkat keras"
  (FPS, latensi per-komponen + rasionalisasi P95, metrik `tegrastats`) dan "Metrik
  kualitas deteksi" (precision/recall/mAP, rasionalisasi *sanity-check* FP16 vs. FP32).
- Menambahkan sitasi akademis (semua ≥2022, diambil dari daftar referensi yang sudah ada
  di `../journal/daftar-referensi.md` — tidak ada sitasi baru yang ditambahkan) untuk
  memperkuat beberapa klaim yang sebelumnya tidak bersitasi: Costa dkk. (2025) untuk
  ambang *real-time* ≥30 FPS, Nigade dkk. (2024) untuk rasionalisasi pelaporan latensi
  P95, Suder dkk. (2023) untuk preseden metrik daya `tegrastats`, Guerrouj dkk. (2025)
  untuk kekhawatiran *trade-off* presisi FP16, dan mengulang sitasi MLPerf Mobile
  (Janapa Reddi dkk., 2022) langsung di tabel kriteria RM3 (sebelumnya hanya disebut
  tanpa sitasi eksplisit di baris tabel tsb.).
- Memperbarui kolom Relevansi di `../journal/daftar-referensi.md` untuk kelima sitasi di
  atas agar mencantumkan pemakaian baru di BAB II §2.7.
- Menambahkan catatan provenance di header BAB-2-Metode-Penelitian.md yang
  mendokumentasikan bahwa pemecahan §2.6→§2.7 ini editorial (di luar naskah PDF sumber),
  tanpa penghapusan konten substantif.
- Tidak ada angka eksperimen baru yang diklaim; hanya reorganisasi teks metodologi dan
  penambahan sitasi pendukung.

## 2026-08-22 12:41 WITA — Ganti poin 7 §1.6 Ruang Lingkup Penelitian

- Atas permintaan user, poin 7 (batasan lokasi & waktu pengujian) di
  `draft/BAB-1-Pendahuluan.md` §1.6 dinilai tidak perlu dan diganti dengan poin batasan
  lain: kamera stereo (ZED) yang dipakai sebagai sumber video hanya dimanfaatkan sebagai
  sumber aliran 2D, kemampuan estimasi jarak berbasis stereo tidak dimanfaatkan/tidak
  menjadi bagian penelitian. Fakta ini digrounding ke
  `../docs/08_limitations_future_work.md` poin 1 (sudah menyebutkan hal yang sama sebagai
  limitasi terdokumentasi) — bukan angka/klaim baru, murni pemindahan poin batasan yang
  sudah ada di dokumentasi proyek ke §1.6.

## 2026-08-22 12:35 WITA — Tulis ulang §1.6 Ruang Lingkup Penelitian, gaya lebih formal & mandiri

- Menulis ulang `draft/BAB-1-Pendahuluan.md` §1.6 Ruang Lingkup Penelitian atas permintaan
  user, mengikuti instruksi ketat: bahasa Indonesia baku/formal, tiap poin diuraikan
  sebagai kalimat lengkap dan lugas (bukan daftar istilah singkat), tanpa rujukan
  eksplisit ke bagian dokumen lain (dihapus semua notasi silang seperti "(§1.3)" dan
  "lihat Bab II §2.5.5" yang ada di versi sebelumnya), dan tanpa singkatan pemberi contoh
  seperti "mis.", "dll.", "dsb.", atau "yaitu"/"yakni" (diganti jadi "berupa").
- User sempat menolak draf pertama (terlalu panjang, tiap poin jadi satu paragraf besar)
  dan memberi contoh gaya skripsi rekan (tiap poin 1–2 kalimat ringkas, format serupa
  daftar bernomor asli). Draf kedua ditulis mengikuti gaya itu: tujuh poin ringkas
  mencakup batasan perangkat (Jetson Orin Nano 4GB, bukan AGX Orin/DLA), variabel yang
  dibandingkan (optimasi NMS paralel EfficientNMS dan pemilihan tracker NvDCF/NvSORT),
  batasan objek (model pre-trained, tanpa pelatihan ulang), batasan cakupan sistem (hanya
  pipeline video, bukan integrasi kendaraan), batasan evaluasi tracking (efisiensi
  komputasi saja, bukan akurasi/ID switch/MOTA, karena tidak ada dataset tracking
  berlabel), batasan presisi numerik (FP16 saja), serta batasan lokasi & waktu (Departemen
  Teknik Informatika Unhas, Mei–Agustus 2026).
- Seluruh fakta yang dipakai (spesifikasi Jetson Orin Nano, daftar model YOLO, metrik
  efisiensi tracker, alasan ketiadaan dataset tracking berlabel, presisi FP16) digrounding
  ke isi `draft/BAB-2-Metode-Penelitian.md` §2.1/§2.2/§2.5.2/§2.5.5/§2.6 yang sudah
  tersinkronisasi dengan naskah PDF sebelumnya — tidak ada angka/fakta baru yang
  ditambahkan, murni penulisan ulang gaya dan struktur kalimat.

## 2026-08-21 16:29 WITA — Tambah diagram mermaid proses fusi graph EfficientNMS di BAB II §2.5.4

- Menambahkan diagram mermaid kedua di §2.5.4, kali ini fokus ke **proses fusi graph**
  (bukan posisi pipeline seperti diagram sebelumnya): dua fase build-time (parse ONNX →
  temukan tensor internal pre-NMS `[1,8400,7]` → pasang node `EfficientNMS_TRT` via
  TensorRT `INetworkDefinition` → build engine baru terpisah) dan runtime (engine hasil
  fusi dieksekusi sebagai satu graph GPU per frame). Detail teknis digrounding ke
  `../../utils/trt_efficientnms/README.md` (dibaca ulang penuh untuk verifikasi) — bentuk
  tensor internal `[1,8400,7]`, empat output plugin standar (`num_detections`,
  `detection_boxes`, `detection_scores`, `detection_classes`), dan penegasan bahwa ONNX +
  engine baseline tidak diubah/ditimpa.
- Nama method TensorRT Python API pada diagram (`network.add_plugin_v2`,
  `builder.build_serialized_network`) dicek langsung terhadap kode
  `../../utils/trt_efficientnms/build_efficientnms_engine.py` (baris 428, 465) — sesuai,
  bukan tebakan pola umum, sehingga catatan `[VERIFIKASI]` awal dihapus setelah
  dikonfirmasi.

## 2026-08-21 16:12 WITA — Tambah diagram mermaid before/after EfficientNMS di BAB II §2.5.4

- Menambahkan diagram mermaid *before/after* (baseline NMS sekuensial CPU vs.
  EfficientNMS_TRT sebagai *tail node* paralel GPU dalam satu *graph* TensorRT) di
  `draft/BAB-2-Metode-Penelitian.md` §2.5.4, atas permintaan user, mengikuti gaya diagram
  pipeline yang sudah ada di §2.5.3. Diagram menegaskan bahwa perubahan terjadi di dalam
  batas `nvinfer`, bukan menambah elemen *pipeline* GStreamer baru.

## 2026-08-21 13:09 WITA — Sinkronkan BAB I & BAB II dengan naskah PDF menuju semhas

Lanjutan sinkronisasi `draft/` ke `skripsi-sementara/SKRIPSI - PERDI MENUJU SEMHAS
YOO.pdf` (lihat entri 2026-08-21 12:30 WITA di bawah untuk konteks/klarifikasi awal).
Sumber acuan PDF: `extracted.txt` baris 380–784 (BAB I) dan 789–1033 (BAB II).

- **`draft/BAB-1-Pendahuluan.md` ditulis ulang penuh.** Perubahan struktural utama:
  (1) subbab baru §1.2 Landasan Teori disisipkan (9 sub-subbab: ADAS/*perception layer*,
  *edge computing*, DeepStream SDK, arsitektur YOLO, *bbox*/IoU/NMS, MOT, algoritma
  *tracking* DeepStream, TensorRT/kuantisasi, metrik evaluasi *real-time*) — disalin
  verbatim dari PDF, diformat ulang ke Markdown (notasi matematika Unicode → notasi
  teks/kode); (2) subbab lama digeser: Rumusan Masalah §1.2→§1.3, Tujuan §1.3→§1.4,
  Manfaat §1.4→§1.5; (3) §1.5 Batasan Masalah + §1.6 Sistematika Penulisan (draf lama)
  digabung jadi satu §1.6 Ruang Lingkup Penelitian (PDF tidak punya subbab Sistematika
  Penulisan terpisah di BAB I); (4) tiga paragraf SOTA tambahan di §1.1 yang tidak ada
  di PDF dihapus (sitasinya sudah relevan di §1.2.4/§1.2.5). Dua bug penomoran internal
  PDF diperbaiki dan didokumentasikan via `[VERIFIKASI]`: rujukan "(§1.2)" → "(§1.3)"
  untuk Rumusan Masalah, dan "Bab II §2.2.6" → "Bab II §2.5.5" untuk justifikasi
  *tracker* (dicek ulang setelah BAB II selesai ditulis).
- **`draft/BAB-2-Metode-Penelitian.md` ditulis ulang penuh**, mengikuti struktur PDF
  yang lebih ringkas (§2.1 Tempat dan Waktu; §2.2 Instrumen Penelitian sebagai daftar
  datar; §2.3 Tahapan Penelitian 8 langkah; §2.4 Teknik Pengambilan Dataset; §2.5
  Perancangan dan Implementasi Sistem §2.5.1–§2.5.5; §2.6 Pengujian) sambil
  mempertahankan detail teknis tergrounding dari draf restrukturisasi 2026-08-19
  sebelumnya (tabel spesifikasi Jetson Orin Nano, tabel model/GFLOPs, tabel konfigurasi
  *tracker*, sitasi dokumentasi resmi NVIDIA + preseden MLPerf Mobile) yang di PDF
  hanya ditulis ringkas atau berupa *placeholder* gambar `[Gambar N: ...]`.
  - Bug penomoran ganda "2.5.5" pada PDF (Implementasi MOT *dan* Pengujian sama-sama
    diberi nomor 2.5.5) diperbaiki: bagian "Pengujian" diberi nomor baru §2.6.
  - Paragraf pembuka §2.5 PDF ("...implementasi model terbaik ke dalam website
    sistem... jalur pelatihan... jalur pengujian...") **sengaja tidak disalin** —
    dinilai kontaminasi sisa skripsi lain (penelitian ini pakai model *pre-trained*,
    tidak melatih ulang, tidak ada *website* — lihat `BAB-1-Pendahuluan.md` §1.6),
    pola yang sama dengan kontaminasi Daftar Isi yang sudah dikonfirmasi sebelumnya.
  - *Placeholder* `[Gambar N: ...]` dari PDF tidak disalin literal, ditandai
    `[VERIFIKASI]` bila relevan (mengikuti pola `(Catatan Visual: ...)` di BAB III).
  - Tanggal spesifik eksekusi eksperimen (19 Agustus 2026, 11:33–12:46 WITA, dari
    `run_info.txt`) dipertahankan sebagai detail dalam rentang "Mei–Agustus 2026" versi
    PDF di §2.1 — tidak bertentangan, tidak dihapus.
- **Rujukan silang di `BAB-3-Hasil-dan-Pembahasan.md` dan `BAB-4-Kesimpulan-dan-Saran.md`
  diperbarui** menyesuaikan penomoran baru BAB I/II: §2.2.1→§2.2, §2.2.4→§2.5.2,
  §2.2.5→§2.5.5, §2.2.6→§2.6, §2.5.3(lama)→§2.5.5, §2.6.1/§2.6.2→§2.6 (dengan anotasi
  poin bila perlu), §1.5→§1.6, §1.2–§1.3→§1.3–§1.4. Dicek dengan `grep` bahwa seluruh
  rujukan `§2.x`/`§1.x` di keempat file BAB kini menunjuk ke subbab yang benar-benar
  ada di draf hasil restrukturisasi.
- *Front matter* (Ucapan Terima Kasih, Abstrak, Daftar Isi, Daftar Tabel/Gambar/
  Lampiran/Istilah, Daftar Pustaka, Lampiran) tetap **tidak disentuh** sesuai instruksi
  standing user — akan ditangani terpisah di Word.

## 2026-08-21 12:30 WITA — Sinkronkan BAB III & BAB IV dengan naskah PDF menuju semhas

Permintaan user: samakan isi `draft/` dengan
`skripsi-sementara/SKRIPSI - PERDI MENUJU SEMHAS YOO.pdf` (diperlakukan sebagai sumber
kebenaran terbaru untuk BAB I–IV; *front matter*, Daftar Pustaka, dan Lampiran di PDF
tersebut milik skripsi mahasiswa lain — sengaja **tidak disentuh**, akan ditangani user
sendiri secara terpisah di Word). Klarifikasi dikumpulkan lewat beberapa putaran
`AskUserQuestion`: PDF menang untuk struktur/wording BAB I–IV; untuk angka
`Lat_Tracker_ms` BAB III yang berbeda antara draf lama dan PDF, user mengonfirmasi angka
PDF yang benar dan diminta diterapkan ke markdown.

**Temuan kunci sebelum menulis apa pun** (dicek dulu, bukan dikarang): angka `Lat_Tracker_ms`
di PDF (mis. YOLOv8n NvDCF 10,981/NvSORT 0,363) **sudah ada persis** di
`eksperimen/runtime_summary.csv` kolom `avg_Lat_Tracker_ms` (rata-rata antar-5-repetisi per
skenario) — cocok hingga 3 desimal. Draf lama `BAB-3-Hasil-dan-Pembahasan.md` §3.3.1
sebelumnya memakai statistik berbeda (median dari seluruh *frame* gabungan, bukan rata-rata
antar-*run*), menghasilkan angka rasio "10×–62×" yang berbeda dari PDF. **Tidak perlu
regenerasi CSV/plot** — data sudah tersedia, hanya kolom agregasi yang perlu diselaraskan
dengan yang dipakai PDF. Rasio yang benar (dari `avg_Lat_Tracker_ms`, dihitung ulang):
YOLOv8n ~30×, YOLOv9t ~26×, YOLOv10n ~12×, YOLO26n ~32×, YOLOv8n+EfficientNMS ~29×,
YOLOv9t+EfficientNMS ~32× — rentang keseluruhan **~12×–32×**, bukan "10×–62×".

**File yang diubah:**

- **draft/BAB-3-Hasil-dan-Pembahasan.md**: ditulis ulang penuh.
  - Penomoran subbab digeser +1 mengikuti PDF (bagian pembuka "Kondisi Eksekusi..." yang
    tadinya tanpa nomor kini jadi §3.1 eksplisit; §3.1→§3.2, §3.2→§3.3, §3.3→§3.4, §3.4→§3.5,
    §3.5→§3.6), termasuk seluruh referensi silang `§3.x` di dalam teks.
  - Tabel §3.4.1 (dulu §3.3.1, `Lat_Tracker_ms` NvDCF vs NvSORT): angka & label kolom diganti
    dari "Median" (statistik pooled-frame lama) menjadi "rata-rata" (`avg_Lat_Tracker_ms`,
    konsisten dengan PDF dan dengan tabel dekomposisi latensi §3.2.3 yang **sebenarnya sudah**
    memakai angka rata-rata ini sejak draf lama — jadi ini juga memperbaiki inkonsistensi
    internal draf lama sendiri). Kolom p95 per-*tracker* dihapus karena tidak tersedia pada
    statistik rata-rata ini. Klaim rasio "10×–62×" → "~12×–32×".
  - Tabel §3.2.1–§3.2.2 (dulu §3.1.1–§3.1.2, FPS & latensi *baseline*): kolom latensi
    "median/p95/p99" (statistik pooled-frame) diganti "Average Latensi/Average P95" memakai
    `avg_latency_ms`/`avg_p95_latency_ms` dari CSV, konsisten dengan PDF §3.2.2 — nilai baru:
    YOLOv8n 273,35/329,10; YOLOv9t 457,67/536,32; YOLOv10n 247,48/299,46; YOLO26n 355,56/409,20
    (dihitung ulang dari CSV, sama persis dengan PDF). Kolom "Ambang" ditambah di tabel FPS
    (123%/72%/123%/119%) mengikuti format Tabel 3.2 PDF.
  - Prosa terkait (§3.1.2 lama, YOLOv9t sebagai *outlier*) disesuaikan ke angka rata-rata baru
    (selisih rata-rata↔P95 YOLOv9t ~78,7 ms vs ~52–56 ms model lain, dihitung ulang dari CSV).
  - Tidak menyalin *placeholder* PDF yang belum terisi (`(Catatan Visual: ...)`,
    `[Tabel 3.X: ...]`, teks literal "Kerjakan nanti di jetson") — tabel/prosa draf lokal yang
    sudah lengkap & tergrounding tetap dipertahankan, hanya angka `Lat_Tracker_ms` dan latensi
    *baseline* yang diselaraskan ke metode agregasi PDF.
- **draft/BAB-4-Kesimpulan-dan-Saran.md**: §4.1 poin 3 — klaim "10× hingga 62×" diganti
  "~12×–32×" mengikuti angka baru §3.4.1. Seluruh referensi silang `§3.x` ke BAB III
  diperbarui mengikuti pergeseran nomor subbab di atas.

**Belum dikerjakan** (lanjutan sesi ini): BAB I (`BAB-1-Pendahuluan.md`) dan BAB II
(`BAB-2-Metode-Penelitian.md`) belum disinkronkan ke struktur PDF (PDF punya §1.2 Landasan
Teori tersendiri dan §2.2 Instrumen Penelitian sebagai daftar datar, berbeda dari struktur
draf lokal saat ini). *Front matter*, Daftar Pustaka, dan Lampiran sengaja tidak disentuh
sesuai instruksi user.

---

## 2026-08-19 16:20 WITA — Gabungkan BAB I–IV (lengkap) menjadi satu naskah docx

Permintaan user: gabungkan skripsi yang sudah selesai (BAB I–IV, seluruhnya sudah terisi
penuh sejak entri 15:10 WITA) menjadi satu dokumen. `draft/_build/*.clean.md` dan
`draft/Skripsi-Gabungan-BAB-I-III.docx` sebelumnya masih merepresentasikan struktur BAB
I–III **lama** (sebelum restrukturisasi format Unhas 14:20 WITA) dan tidak menyertakan BAB
IV sama sekali — sesuai catatan "belum dilakukan" pada entri 14:20 WITA, keduanya dibangun
ulang di sesi ini.

**File yang diubah:**

- **draft/BAB-1-Pendahuluan.md** §1.6 Sistematika Penulisan: placeholder `TODO` diisi
  dengan ringkasan 1 paragraf per bab (BAB I–IV), karena struktur akhir seluruh bab kini
  sudah stabil dan lengkap — tidak ada data eksperimen baru yang dikarang, murni ringkasan
  struktural dari isi bab yang sudah ada.
- **draft/_build/clean_chapter.py** (BARU): skrip pembersih markdown yang menggantikan
  proses manual sebelumnya — menghapus blok blockquote status/`[VERIFIKASI]` di awal tiap
  bab, menghapus section `## Judul (revisi)` (khusus BAB I), dan melucuti tag inline
  `` `[VERIFIKASI]` `` di tengah paragraf/tabel tanpa menghapus kalimat substantifnya.
  Divalidasi dengan cara menjalankannya pada `BAB-1-Pendahuluan.md` dan membandingkan
  hasilnya terhadap `BAB-1.clean.md` versi lama — ditemukan bahwa versi lama tersebut sudah
  usang (belum memuat 3 paragraf *state of the art* yang dipindahkan dari
  `BAB-2-Tinjauan-Pustaka.md` pada restrukturisasi 14:20 WITA), mengonfirmasi perlunya
  regenerasi penuh.
- **draft/_build/BAB-1.clean.md, BAB-2.clean.md, BAB-3.clean.md**: dibangun ulang dari draf
  sumber terbaru (pasca-restrukturisasi format Unhas) via `clean_chapter.py`.
- **draft/_build/BAB-4.clean.md** (BARU): versi bersih `BAB-4-Kesimpulan-dan-Saran.md`,
  dibangun via `clean_chapter.py`.
- **draft/_build/generate_docx.py**: (a) `BUILD_DIR`/`OUT_PATH` diubah dari path absolut
  hardcoded milik mesin lain (`/home/perdidev/dev/final-project/...`) menjadi path relatif
  terhadap lokasi skrip, supaya portabel; (b) `CHAPTER_FILES` ditambah `BAB-4.clean.md`;
  (c) subjudul halaman judul diperbarui "Bab I–III" → "Bab I–IV"; (d) `OUT_PATH` diubah ke
  nama baru `Skripsi-Gabungan-BAB-I-IV.docx`; (e) `VERIFICATION_NOTES` dirombak: referensi
  nomor bagian usang diperbarui mengikuti struktur baru (mis. "Bab II §2.2.7" → "Bab II
  §2.2.1", "Bab III §3.2.2"/"§3.2.3"/"§3.6" → "Bab II §2.2.2"/"§2.2.3"/"§2.6.2", "Bab II
  §2.1.3" → "Bab I §1.1"); butir "jumlah repetisi belum dicatat" **dihapus** karena sudah
  resolved (60 run aktual, 5 repetisi per skenario, sudah dieksekusi dan dilaporkan di Bab
  III); butir baru ditambahkan untuk nama laboratorium spesifik yang belum dikonfirmasi (Bab
  II §2.1); butir verifikasi akurasi FP16 diperbarui untuk mencatat bahwa Bab IV §4.1 sudah
  mencantumkan keterbatasan ini secara eksplisit; (f) paragraf pembuka lampiran "Catatan
  Verifikasi" diperbarui — tidak lagi menyebut Bab IV/V sebagai "belum disertakan", karena
  kini keempat bab sudah tercakup penuh dalam naskah gabungan.
- **draft/Skripsi-Gabungan-BAB-I-IV.docx** (BARU, menggantikan
  `Skripsi-Gabungan-BAB-I-III.docx` yang dihapus): dibangun dari `clean.md` terbaru via
  `generate_docx.py` (dijalankan dengan python-docx 1.2.0 terpasang `pip install --user`).
  Diverifikasi terprogram: tag `[VERIFIKASI]` nihil di badan naskah (satu-satunya kemunculan
  string "VERIFIKASI" adalah judul lampiran itu sendiri), `NvDCF_perf` nihil, dan dua
  kemunculan `TODO` yang tersisa keduanya berasal dari BAB III §3.4 (verifikasi akurasi
  as-deployed FP16) yang memang secara jujur belum dieksekusi — sesuai kebijakan proyek
  untuk tidak mengarang data yang belum ada.
- **draft/Skripsi-Gabungan-BAB-I-III.docx**: dihapus (digantikan versi BAB I–IV di atas).

**Belum dilakukan (di luar scope sesi ini)**: gambar/plot (`../eksperimen/plots/*.png`) yang
dirujuk di Bab III (mis. `fps_boxplot_by_tracker.png`, `tradeoff_map_vs_fps.png`) tidak
diembed ke docx — `generate_docx.py` hanya merender teks/tabel, path gambar tetap disebut
apa adanya sebagai teks; penulis perlu menambahkannya manual di Word bila ingin
ditampilkan sebagai figur. Konfirmasi rumusan masalah #3 ke dosen pembimbing juga masih
belum dilakukan (lihat lampiran catatan verifikasi pada docx).

---

## 2026-08-19 15:10 WITA — Isi penuh BAB IV (Kesimpulan dan Saran)

- **skripsi/draft/BAB-4-Kesimpulan-dan-Saran.md**: skeleton TODO diisi penuh. §4.1 Kesimpulan
  menjawab ketiga rumusan masalah/tujuan penelitian (`BAB-1-Pendahuluan.md` §1.2–§1.3) satu per
  satu berdasarkan hasil `BAB-3-Hasil-dan-Pembahasan.md` (60 *run* — 6 model × 2 *tracker* × 5
  repetisi): (1) RM1 — keempat model *baseline* memenuhi ambang *real-time* 30 FPS dengan margin
  besar, YOLOv9t teridentifikasi sebagai *outlier* latensi akibat *backpressure* `Lat_PreMux_ms`;
  (2) RM2 — EfficientNMS tidak terbukti meningkatkan efisiensi *pipeline* (temuan negatif pada
  YOLOv9t, tidak signifikan pada YOLOv8n), model NMS-*free* juga tidak otomatis menjamin
  *throughput* tertinggi; (3) RM3 — NvSORT 10×–62× lebih murah dari NvDCF di level komponen
  (universal, tidak bergantung model), tetapi dampaknya ke *throughput* akhir kondisional (hanya
  signifikan besar pada YOLOv9t yang *headroom*-nya sudah tipis). Ditutup catatan bahwa
  kesimpulan terkait *trade-off* akurasi masih bersyarat pada *proxy* FP32 karena verifikasi
  akurasi *as-deployed* FP16 (§3.4.2 BAB III) belum dieksekusi — sengaja tidak diklaim pasti
  supaya tidak melebih-lebihkan kepastian hasil. §4.2 Saran berisi 8 poin diurutkan usaha
  kecil→besar: (1) verifikasi akurasi FP16 *as-deployed* (prioritas), (2) kanal pengukuran suhu
  SoC, (3) eksplorasi optimasi lanjutan di luar *plugin* EfficientNMS (mengutip urutan
  rekomendasi `../../utils/trt_efficientnms/README.md` §"Batas optimasi dan alternatif" —
  *score-threshold*, `max-output-boxes`, profil `trtexec`/Nsight Systems, model NMS-*free*,
  *custom plugin fused*), (4) eksperimen presisi INT8, (5) skenario pengujian tambahan
  (kepadatan lalu lintas/cahaya/cuaca/durasi klip), (6) pengukuran kualitas *tracking*
  (ID *switch*/MOTA/IDF1) di luar *scope* penelitian ini, (7) pengujian *deployment*
  multi-kamera/multi-model pada SKU 4GB/8GB, (8) pemanfaatan *depth-sensing* stereo kamera ZED
  untuk estimasi jarak (dari `../../docs/08_limitations_future_work.md` §8.1–8.2, belum
  dimanfaatkan pada penelitian ini). Tidak ada angka baru yang dikarang — seluruh klaim numerik
  merujuk balik ke tabel/bagian BAB III yang sudah ada.

---

## 2026-08-19 14:20 WITA — Restrukturisasi seluruh BAB II–V mengikuti format skripsi Unhas (BAB II wajib Metode Penelitian)

Atas instruksi eksplisit user (paste arahan/rubrik format skripsi Departemen Teknik Informatika
Unhas), seluruh struktur bab draf skripsi direstrukturisasi. Perubahan struktural: BAB II lama
("Tinjauan Pustaka") dan BAB III lama ("Metodologi Penelitian") digabung dan digeser menjadi
satu BAB II baru ("Metode Penelitian"); BAB IV lama ("Hasil dan Pembahasan") bergeser menjadi
BAB III baru; BAB V lama ("Kesimpulan dan Saran") bergeser menjadi BAB IV baru. Total bab
tetap 4 (I–IV), bukan 5 seperti draf sebelumnya.

**Alasan**: format skripsi Departemen Teknik Informatika Unhas mewajibkan BAB II berupa Metode
Penelitian (bukan Tinjauan Pustaka). *State of the art* ("Penelitian Terkait") harus diringkas
dan diintegrasikan ke BAB I (Latar Belakang) untuk memperkuat argumen *research gap*; Landasan
Teori tidak boleh berdiri sendiri sebagai bab, melainkan dipadatkan sebagai justifikasi ilmiah
langsung di dalam sub-bab metode (instrumen, perancangan sistem) di BAB II.

**File yang diubah:**

- **skripsi/draft/BAB-1-Pendahuluan.md**: §1.1 Latar Belakang ditambah 3 paragraf baru yang
  meringkas *state of the art* dari draf `BAB-2-Tinjauan-Pustaka.md` §2.1.1–§2.1.2 (versi lama,
  kini dihapus) — klaster deteksi objek YOLO pada perangkat *edge* (Ayachi, Dhatrika, Chaman,
  Guerrouj, Bouazizi, Xie, Tsai & Hsieh) dan klaster akselerasi NMS (Chen, Oro, Yang) — masing
  ditutup dengan kalimat yang mengaitkan celah literatur ke rumusan masalah #1/#2. Klaster
  efisiensi *real-time*/penjadwalan (§2.1.3 lama) dan konteks umum ADAS (§2.1.4 lama) tidak
  diduplikasi karena sitasinya (Choi, Nigade, Zhang, Ruiz-Barroso, Suder, Seyfipoor, Shah,
  Neumann, Costa) sudah ada di §1.1 sejak draf pertama. Rujukan silang "Bab II §2.2.6" (soal
  justifikasi *tracker*) diperbarui jadi "Bab II §2.5.3" mengikuti struktur BAB II baru.
  Ditambah catatan status di header.
- **skripsi/draft/BAB-2-Metode-Penelitian.md** (BARU, menggantikan `BAB-2-Tinjauan-Pustaka.md`
  dan `BAB-3-Metodologi-Penelitian.md` yang keduanya dihapus): §2.1 Tempat dan Waktu Penelitian
  (baru, ditandai `[VERIFIKASI]` untuk nama laboratorium spesifik — belum tercatat di dokumen
  proyek manapun); §2.2 Benda Uji dan Alat (dari §3.2 lama, sub-nomor 2.2.1–2.2.6 dipertahankan
  sama seperti sub-nomor lama 3.2.1–3.2.6 untuk minim disrupsi); §2.3 Tahapan Penelitian (dari
  §3.1 + §3.3 lama, digabung — deskripsi jenis penelitian eksperimental kuantitatif jadi
  paragraf pembuka); §2.4 Teknik Pengumpulan Data (baru, disintesis dari §2.2.8 lama "Metrik
  Evaluasi" + mekanisme pad probe/tegrastats yang sebelumnya tersebar di §3.4 lama); §2.5
  Perancangan dan Implementasi Sistem (dari §3.4 lama, digabung dengan landasan teori DeepStream
  SDK/NMS/tracker dari §2.2.2/§2.2.5/§2.2.6 lama — uraian NvDCF/NvSORT beserta sitasi NVIDIA dan
  MLPerf Mobile dipindah utuh ke §2.5.3); §2.6 Skenario Pengujian dan Kriteria Evaluasi (dari
  §3.5 + §3.6 lama). Seluruh angka/tabel teknis (spesifikasi Jetson Orin Nano, tabel model,
  tabel tracker) disalin apa adanya dari draf lama — tidak ada angka baru yang dikarang.
- **skripsi/draft/BAB-3-Hasil-dan-Pembahasan.md** (BARU, menggantikan
  `BAB-4-Hasil-dan-Pembahasan.md` yang dihapus — isi 60-run tetap sama, hanya restrukturisasi):
  §3.1 Baseline (RM1) dipecah jadi 3.1.1 Throughput (termasuk tabel kepatuhan real-time 60 run,
  dulu §4.4 terpisah), 3.1.2 Latensi E2E (p95/p99), 3.1.3 Latensi per-komponen; §3.2 NMS (RM2)
  dipecah jadi 3.2.1 dampak latensi inferensi, 3.2.2 dampak throughput (+ pembahasan hasil
  negatif dari §4.5.1 lama), 3.2.3 pembahasan model NMS-free (baru, mensintesis YOLOv10n/YOLO26n
  dari sudut RM2); §3.3 Tracking (RM3) dipecah jadi 3.3.1 latensi tracker, 3.3.2 dampak FPS (+
  pembahasan dari §4.5.2 lama), 3.3.3 penggunaan sumber daya; §3.4 Verifikasi akurasi FP16
  (gabungan §4.2.1+§4.2.2 lama, masih TODO — belum dieksekusi); §3.5 Pembahasan akhir (3.5.1
  trade-off dari §4.5.3 lama + perbandingan penelitian terkait dari §4.6 lama yang sebelumnya
  TODO kosong, kini diisi paragraf singkat; 3.5.2 keterbatasan dari §4.5.4 lama ditambah dua
  subbagian BARU yang diminta struktur Unhas — keterbatasan memori 4GB modul Jetson Orin Nano
  (dihitung dari RAM usage 1265–1449 MB di Tabel 3.3.3, bukan angka baru) dan pengaruh thermal
  throttling (ditulis jujur: dimitigasi prosedural via cooldown 60 detik, TAPI tidak diukur
  langsung karena tegrastats parser saat ini tidak mengekstrak suhu — bukan diklaim terverifikasi
  padahal tidak ada datanya).
- **skripsi/draft/BAB-4-Kesimpulan-dan-Saran.md** (BARU, menggantikan
  `BAB-5-Kesimpulan-dan-Saran.md` yang dihapus): isi tidak berubah (masih skeleton/TODO), hanya
  judul "BAB V" → "BAB IV" dan rujukan internal "BAB IV" → "BAB III" diperbarui.
- **skripsi/eksperimen/README.md**: rujukan "BAB IV (`BAB-4-Hasil-dan-Pembahasan.md`)" →
  "BAB III (`BAB-3-Hasil-dan-Pembahasan.md`)".
- **skripsi/journal/daftar-referensi.md**: catatan di bagian atas tabel diperbarui untuk
  menjelaskan pemindahan lokasi (BAB-2-Tinjauan-Pustaka.md dihapus) — kolom Relevansi per-baris
  **tidak** diubah satu per satu (masih menyebut nomor lama "BAB II §2.1.x"/"§2.2.x") karena
  disertai catatan penjelasan pemetaan di atas tabel, demi menghindari risiko salah pemetaan
  nomor per baris.

**Belum dilakukan (di luar scope sesi ini)**: regenerasi `draft/_build/*.clean.md` dan
`draft/Skripsi-Gabungan-BAB-I-III.docx` — keduanya kini merepresentasikan struktur BAB I–III
**lama** dan sudah usang setelah restrukturisasi ini; perlu dibangun ulang dari draf BAB I–III
baru sebelum diajukan ke dosen pembimbing lagi.

---

## 2026-08-19 13:50 WITA — Isi penuh BAB IV berdasarkan eksekusi 60 run (6 model × 2 tracker × 5 repetisi)

- **skripsi/draft/BAB-4-Hasil-dan-Pembahasan.md**: ditulis penuh (sebelumnya skeleton berisi
  TODO semua bagian). Sumber data: `skripsi/eksperimen/runtime_summary.csv`,
  `runtime_per_run.csv`, `tradeoff_summary.csv`, `plots/*.png` (hasil `scripts/run_all_benchmark.sh`
  yang dijalankan user pada 2026-08-19 11:33–12:46 WITA, 60 run sukses tanpa gagal) dan
  `../../docs/05_accuracy_results.md` (akurasi FP32).
- **Statistik tambahan yang dihitung khusus untuk bab ini** (belum ada di file CSV yang sudah
  ada, dihitung dari `data/benchmark/*/*/fps.csv` mentah via `utils/benchmark_analysis/common.py`
  + pandas, tidak disimpan sebagai file permanen — hanya dipakai sekali untuk mengisi tabel):
  median/p95/p99 `Lat_Tracker_ms` dan `Latency_ms` per skenario (pooled per-frame, bukan
  rata-rata dari rata-rata), estimasi energi per frame (`avg_VDD_IN_mW` ÷ `avg_fps`), dan FPS
  minimum/maksimum per skenario dari `runtime_per_run.csv`.
- **Uji signifikansi**: dijalankan `utils/benchmark_analysis/tradeoff_analysis.py --significance`
  (Welch's t-test) untuk seluruh pasangan NvDCF vs NvSORT (6 model) dan pasangan baseline vs
  EfficientNMS (YOLOv8n, YOLOv9t) — hasil p-value dicantumkan di tabel §4.3.2 dan §4.3.3.
- **Temuan utama yang masuk ke bab**: (a) `Lat_Tracker_ms` NvSORT konsisten 10–62× lebih rendah
  dari NvDCF di keenam model, tapi dampaknya ke FPS akhir hanya signifikan besar pada YOLOv9t
  (+30%/+33%) karena pipeline model ini sudah mendekati batas throughput; (b) EfficientNMS pada
  YOLOv9t secara statistik signifikan **lebih lambat** dari baseline (p=0,023) — hasil negatif
  yang dijelaskan lewat karakteristik tail-dependent plugin EfficientNMS_TRT
  (`utils/trt_efficientnms/README.md`); (c) estimasi energi/frame menunjukkan NvSORT lebih hemat
  di keenam model walau daya sesaat kadang lebih tinggi (YOLOv9t) — diselesaikan lewat normalisasi
  daya/FPS; (d) seluruh 60 run (termasuk FPS minimum per skenario) melampaui ambang real-time 30
  FPS dengan margin besar.
- **Bagian yang sengaja dibiarkan TODO** (tidak mengarang data): §4.2.2 (verifikasi akurasi
  as-deployed FP16 — infrastruktur siap tapi belum dieksekusi, sesuai status di
  `BAB-3-Metodologi-Penelitian.md` §3.5 poin 4) dan §4.6 (perbandingan dengan penelitian terkait
  di Bab II — perlu penulis merujuk ulang jurnal yang relevan).
- **Catatan transparansi ditambahkan di §4.1.1**: klarifikasi bahwa galat `jetson_clocks_status`
  di `run_info.txt` ("Run this script as a root user") berasal dari pemeriksaan status tanpa
  sudo di `run_benchmark.sh`, bukan indikasi clock gagal terkunci (langkah pengunci sesungguhnya
  pakai sudo di awal `run_all_benchmark.sh` dengan `set -e`); juga dicatat keterbatasan durasi
  klip video per run (~13–20 detik, lebih pendek dari rekomendasi 180 detik protokol) di §4.5.4.

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

# Panduan Kerja AI — Folder `skripsi/`

Catatan ini untuk Claude (atau AI lain) yang membantu menyusun skripsi di folder ini.
Baca file ini dulu sebelum mengerjakan apa pun di dalam `skripsi/`.

## Konteks singkat

- Judul: **Optimasi Realtime Pipeline Nvidia Deepstream pada Aplikasi ADAS Berbasis Edge Device**
- Sudah melewati **seminar proposal**. Rumusan masalah/tujuan penelitian versi proposal
  masih memakai **Jetson AGX Orin** (dengan target implementasi DLA/Deep Learning Accelerator).
- **Perubahan penting**: implementasi final memakai **Jetson Orin Nano**, bukan AGX Orin.
  Orin Nano tidak punya DLA yang relevan seperti AGX Orin, sehingga rumusan masalah/tujuan
  terkait DLA di proposal lama tidak bisa dipertahankan. **Keputusan sudah diambil**
  (2026-08-14, lihat `log/log-perubahan.md`): rumusan masalah #3 diganti dari "DLA vs GPU
  baseline" menjadi **"perbandingan efisiensi komputasi algoritma tracking (NvDCF vs
  NvSORT)"**, di-scope murni ke efisiensi komputasi (FPS, `Lat_Tracker_ms`,
  resource usage) — **tanpa** metrik kualitas/akurasi tracking (ID switch, MOTA), karena
  itu butuh dataset tracking berlabel terpisah yang belum tersedia. Detail lengkap
  perubahan rumusan masalah/tujuan/batasan ada di `draft/BAB-1-Pendahuluan.md` (lihat
  anotasi `[VERIFIKASI]` di setiap bagian yang berubah — penulis tetap perlu mengecek
  ulang sebelum final). Jangan buka ulang diskusi ini kecuali user yang memintanya.
- Kode pipeline DeepStream, dokumentasi teknis (`docs/01`..`docs/08`), dan hasil benchmark
  ada di root proyek ini (`../src`, `../docs`, `../data`, `../utils`). Ini adalah sumber
  kebenaran teknis untuk BAB III/IV — jangan mengarang angka atau detail arsitektur, selalu
  rujuk ke file aslinya.

## Struktur folder

```
skripsi/
├── PANDUAN-AI.md          <- file ini
├── Proposal/              <- proposal skripsi lama (user akan copy manual)
├── referensi-skripsi/     <- contoh skripsi lain untuk rujukan struktur & gaya penulisan
├── draft/                 <- draft per BAB skripsi (diisi/diupdate oleh AI)
├── journal/               <- jurnal referensi yang didownload AI + daftar-referensi.md
├── eksperimen/            <- hasil uji/eksperimen dari implementasi pipeline
└── log/
    └── log-perubahan.md   <- SATU file log, selalu diupdate tiap ada perubahan di skripsi/
```

## Aturan wajib untuk AI

1. **Setiap kali** membuat, mengubah, atau menghapus file di dalam `skripsi/` (termasuk
   subfoldernya), tambahkan entri baru di `skripsi/log/log-perubahan.md` — format dan aturan
   ada di file itu sendiri. Jangan lupakan ini walau perubahannya kecil.
2. Sebelum menulis konten BAB baru di `draft/`, cek dulu apakah `Proposal/` dan
   `referensi-skripsi/` sudah berisi file — kalau sudah, gunakan sebagai rujukan struktur/gaya
   supaya konsisten dengan apa yang sudah disetujui dosen pembimbing di seminar proposal.
3. Jangan mengarang data hasil eksperimen. Angka akurasi/FPS/latensi harus berasal dari
   `../docs/`, `../data/`, atau file di `eksperimen/` — kalau belum ada datanya, tulis
   placeholder yang jelas (mis. `TODO: isi setelah pengujian di Jetson Orin Nano`), jangan
   diisi dengan angka tebakan.
4. Ada skill Claude Code di `.claude/skills/skripsi-log/SKILL.md` yang merangkum aturan
   di atas — panggil/ikuti skill itu kalau tersedia saat mengerjakan sesuatu di folder ini.

Lihat `skripsi/log/log-perubahan.md` untuk riwayat perubahan terbaru sebelum melanjutkan
pekerjaan, supaya tidak mengulang atau menimpa pekerjaan sebelumnya.

# BAB IV — HASIL DAN PEMBAHASAN

> Status: skeleton awal. JANGAN isi dengan angka karangan — semua angka harus berasal dari
> `../eksperimen/` atau `../../docs/`. Kalau belum ada datanya, biarkan TODO.

## 4.1 Hasil Implementasi Pipeline

TODO

## 4.2 Hasil Pengujian Akurasi

- TODO: Tabel mAP/precision/recall FP32-proxy (lihat `../../docs/05_accuracy_results.md`)
- TODO: Tabel mAP as-deployed FP16 (setelah dijalankan di Jetson Orin Nano — lihat
  `../eksperimen/`)
- TODO: Perbandingan delta FP32 vs FP16

## 4.3 Hasil Pengujian Runtime/Hardware

- TODO: FPS & latensi baseline vs EfficientNMS (per model, per komponen pipeline)
- TODO: Analisis kenapa hasil EfficientNMS lebih lambat/tidak signifikan dibanding baseline
  (rujuk `../../utils/trt_efficientnms/README.md` §"Batas optimasi dan alternatif")

## 4.4 Hasil Pengujian Real-time

TODO — hasil pengujian live camera terhadap kriteria real-time yang didefinisikan di BAB III.

## 4.5 Analisis dan Pembahasan

TODO — termasuk pembahasan hasil negatif (EfficientNMS tidak signifikan) sebagai temuan
ilmiah yang sah, bukan kegagalan penelitian.

## 4.6 Perbandingan dengan Penelitian Terkait

TODO — bandingkan dengan jurnal di BAB II, khususnya jurnal yang menjadi dasar pemilihan
NMS sebagai fokus optimasi di proposal.

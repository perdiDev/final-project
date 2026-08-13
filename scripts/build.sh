#!/bin/bash

# 1. Hapus folder build jika sudah ada sebelumnya
rm -rf build

# 2. Konfigurasi build Release agar aplikasi dan custom parser teroptimasi.
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 3. Kompilasi menggunakan semua core CPU yang tersedia.
cmake --build build --parallel "$(nproc)"

# 4. Salin executable aplikasi ke root proyek.
cp build/app ./app

echo "Build berhasil diselesaikan!"

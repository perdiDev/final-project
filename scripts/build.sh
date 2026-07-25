#!/bin/bash

# 1. Hapus folder build jika sudah ada sebelumnya
rm -rf build

# 2. Buat folder build baru
mkdir -p build && cd build

# 3. Jalankan CMake
cmake ..

# 4. Jalankan kompilasi (Make) menggunakan semua core CPU yang tersedia
make -j$(nproc)

# 5. Pindahkan file hasil build (yang berawalan "Deep") ke direktori parent
mv app* ../

# 6. Kembali ke direktori sebelumnya
cd ..

echo "Build berhasil diselesaikan!"

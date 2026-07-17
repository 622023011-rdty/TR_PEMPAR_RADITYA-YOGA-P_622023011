**Laporan Tugas Rancang Pemrosesan Paralel**

**Aplikasi 2D Fire Particle System dengan OpenMP dan SFML**

1. Identitas Praktikan
* **Nama** : Raditya Yoga Pratama
* **NIM**  : 622023011
* **Kelas**: Pemrosesan Paralel
2. **Setup Development Environment**
2.1.**Pemasangan Dependensi**
Proyek ini menggunakan compiler GCC yang mendukung OpenMP dan library grafis SFML. Jalankan perintah berikut di terminal Ubuntu untuk memasang seluruh dependensi:
* sudo apt update
* sudo apt install build-essential libsfml-dev
2.2.**Langkah Kompilasi**
Gunakan perintah di bawah ini untuk mengompilasi kode program dengan optimasi level 3 (-O3) serta mengaktifkan pustaka paralel OpenMP:
g++ -O3 main.cpp -o simulasi\_partikel -fopenmp -lsfml-graphics -lsfml-window -lsfml-system
3. **Penjelasan Cara Kerja Program**
Program ini mensimulasikan pergerakan dinamis dari 2000 partikel yang membentuk kobaran api secara real-time dengan aturan logika fisik sebagai berikut:
* **Inisialisasi \& Batas Umur**:
Partikel lahir di bagian bawah tengah layar dengan koordinat X acak tipis dan koordinat Y di area dasar (550). Setiap partikel memiliki waktu hidup (lifetime) acak.
* **Pergerakan \& Turbulensi**:
Partikel melesat ke atas (nilai vy negatif). Supaya gerakan api tidak kaku atau lurus, sistem menambahkan gaya acak horizontal kecil (turbulensi udara) pada setiap frame sehingga api meliuk-liuk alami.
* **Dinamika Visual**:
Ukuran fisik dan warna partikel berubah secara linear mengikuti sisa umurnya. Partikel muda berukuran besar dan berwarna kuning-oranye terang, kemudian mengecil dan berubah memerah seiring bertambahnya umur hingga akhirnya mati (hangus).
* **Interaksi Hembusan Angin**:
Saat pengguna menekan klik kiri mouse, program menghitung jarak partikel ke posisi kursor. Jika berada dalam radius 250px, partikel diberi gaya dorong horizontal menjauh dari kursor layaknya tertiup angin kencang.
* **Sistem Respawn**:
Partikel yang mati atau keluar batas jendela akan langsung disetel ulang koordinat dan umurnya ke kondisi awal di dasar layar sehingga simulasi berjalan tanpa henti.
4. **Flowchart Program**
Berikut adalah alur logika eksekusi program dari awal hingga selesai:
```mermaid
graph TD
    A([Mulai Program]) --> B[Inisialisasi 2000 Partikel<br/>Posisi dasar, Kecepatan, Umur]
    B --> C[Tampilkan Window Grafis & Teks 'CLICK TO START']
    C --> D{Apakah User Klik Layar?}
    D -- Belum --> E[Tahan Layar / Diam]
    D -- Sudah --> F[Aktifkan State Simulasi]
    E --> G[Hitung & Update FPS]
    F --> G
    G --> H[=== MASUK LOOP UTAMA OPENMP ===<br/>Ambil Posisi Kursor Mouse]
    H --> I[Paralel Loop untuk Setiap Partikel<br/>i = 0 ke 1999]
    I --> J[Tambah Gaya Turbulensi Udara<br/>JIKA Klik Kiri: Tambah Gaya Hembusan Angin<br/>Update Posisi X, Y berdasarkan Kecepatan<br/>Kurangi Lifetime Partikel<br/>Hitung Gradasi Warna & Ukuran]
    J --> K{JIKA Lifetime <= 0}
    K -- Ya --> L[Respawn Partikel di dasar]
    K -- Tidak --> M[Bersihkan Layar / window.clear]
    L --> M
    M --> N[Gambar Semua Partikel & Teks FPS]
    N --> O[Tampilkan ke Layar / window.display]
    O --> P{Apakah Jendela Ditutup?}
    P -- Tidak --> D
    P -- Ya --> Q([Selesai / Keluar])

5. **Penjelasan Implementasi Paralel yang Digunakan**
* **Direktif OpenMP**:
Pemrosesan paralel diterapkan pada update loop utama partikel menggunakan konstruksi #pragma omp parallel yang dikombinasikan dengan #pragma omp for.
* **Pembagian Beban Kerja**:
Komputasi fisik (kalkulasi jarak angin mouse, penambahan turbulensi, pergerakan posisi, dan pengkondisian respawn) untuk 2000 partikel dibagi habis secara merata ke thread-thread CPU yang tersedia.
* **Thread-Safety (Keamanan Data)**:
Fungsi acak bawaan std::rand() tidak aman dijalankan secara paralel. Program ini menggunakan fungsi rand\_r(\&seed) di mana variabel seed dibuat unik untuk masing-masing thread dengan memanfaatkan ID thread (omp\_get\_thread\_num()). Hal ini mencegah terjadinya race condition atau tabrakan data antar core CPU.
6. **Pengujian performa diukur melalui indikator FPS meter biru yang berada di pojok kiri atas jendela simulasi**:
6.1. **Pengujian Sekuensial (1 Core / Thread)**:
Dijalankan menggunakan perintah OMP\_NUM\_THREADS=1 ./simulasi\_partikel. Aplikasi berjalan cukup lancar pada kondisi diam, namun nilai FPS mengalami penurunan signifikan (dropping) saat user melakukan klik kiri dan menggerakkan mouse secara agresif (karena CPU tunggal kewalahan menghitung rumus jarak matematika untuk 2000 partikel sekaligus).
6.2. **Pengujian Paralel (4 Cores / Threads)**:
Dijalankan menggunakan perintah OMP\_NUM\_THREADS=4 ./simulasi\_partikel. Distribusi beban kerja ke semua inti CPU membuat performa aplikasi sangat stabil tinggi di batas maksimal screen-refresh. Nilai FPS tetap bertahan mulus tanpa adanya gejala patah-patah meskipun efek hembusan angin mouse diaktifkan terus-menerus.
7. **Dokumentasi Penggunaan Program**
7.1. Memulai Aplikasi:
Saat jendela simulasi pertama kali terbuka, partikel akan membeku di dasar layar. Klik di area mana saja pada jendela window untuk memulai jalannya simulasi api.
7.2. Melihat Indikator Kinerja:
Setelah berjalan, teks indikator performa berwarna biru di pojok kiri atas akan aktif memantau nilai FPS aktual dan jumlah partikel secara real-time.
7.3. Interaksi Efek Angin:
Gerakkan kursor mouse ke arah tengah kobaran api, lalu klik kiri dan tahan. Gerakkan mouse ke kiri atau kanan; partikel api akan meliuk dan terhempas menjauh dari kursor meniru efek hembusan angin fisik.
7.4. Keluar dari Aplikasi:
Klik tombol silang (X) di pojok kanan atas jendela window untuk menutup jalannya simulasi secara aman.


MiniSplit 1.3
=============

Tentang Program Ini
-------------------
MiniSplit adalah sebuah program untuk memecah file menjadi bagian-bagian
yang lebih kecil.  MiniSplit didesain untuk memiliki ukuran yang kecil
tetapi tetap memiliki kemampuan yang memadai.  Program ini cocok digunakan
bila anda ingin memindahkan file berukuran besar dengan menggunakan disket
atau media lain yang terbatas kapasitasnya.

MiniSplit dapat disebarkan dan digunakan secara gratis (freeware) selama
tidak ada modifikasi yang dilakukan terhadap file program.  Tidak ada
garansi yang disertakan dalam program ini.

Cara Penggunaan
---------------
Untuk memecah file:
  o Pilih file yang akan dipecah dengan cara:
    (1) mengetikkan nama file pada kolom File Name;
    (2) klik Browse kemudian pilih file yang diinginkan;
    (3) menyeret file dari Windows Explorer ke dalam MiniSplit.
  o Bila perlu masukkan folder tujuan.
  o Tentukan ukuran maksimum file pada kolom Split Into, dan pilih besaran
    yang sesuai (byte, kbyte, mbyte).  Bila ingin memecah file menjadi
    n bagian ketikkan n pada kolom Split Into dan pilih 'parts' pada kolom
    besaran.
  o Bila perlu pilih Split pada kolom Operation.
  o Klik Do It, dan MiniSplit akan meminta konfirmasi untuk mulai memecah
    file.
  o File yang dihasilkan akan bernama namaasli.000, namaasli.001, ... dan
    juga namaasli.crc (berisi informasi pemeriksaan kesalahan pada file).

Untuk menggabungkan file:
  o Pastikan anda telah memiliki setiap bagian file pada sebuah folder.
  o Pilih salah satu bagian file. (lihat cara memilih file di atas)
  o Bila perlu, masukkan folder tujuan.
  o Klik Do It, dan MiniSplit akan meminta konfirmasi untuk mulai
    menggabungkan file.  Bila ada file yang rusak, MiniSplit akan
    menginformasikannya kepada anda.
  o Anda tetap dapat menggabungkan file tanpa file .crc, tetapi anda tidak
    dapat memeriksa keaslian file hasil gabungan.

.: Catatan :.
Ketika mem-Browse atau menyeret suatu file MiniSplit akan memilih operasi
yang sesuai dengan ekstensi file tersebut.  Misalnya anda memilih file
foo.000 maka MiniSplit akan memilih operasi Join secara otomatis.

Tips dan Trik
-------------
  o Anda bisa menyeret (drag) file dari Windows Explorer ke dalam MiniSplit.
  o MiniSplit akan mencatat kode untuk memeriksa kondisi setiap bagian file.
    Sehingga bila ada bagian file yang rusak, akan diketahui bagian mana
    yang rusak.
  o Anda dapat menggabungkan file hasil pemecahan dengan MiniSplit tanpa
    memerlukan MiniSplit yaitu dengan cara:

    Pada DOS:   copy /b foo.000+foo.001+... foo
                foo.000, foo.001, ... adalah nama setiap bagian file
                sedangkan foo adalah nama file tujuan.

    Pada Linux: cat foo.[0-9]* > foo

Informasi Teknis
----------------
File crc adalah file biner yang digunakan untuk memeriksa keaslian file.
Isi file ini yaitu:

  o 8 byte pertama berisi ukuran file asli.
  o n*(4 byte) berikutnya berisi CRC-32 dari n buah bagian file.

Penulisan file crc menggunakan urutan byte Intel (LSB terlebih dulu).

Kontak
------
Informasi bug atau komentar tentang program ini dapat dialamatkan pada
  mailto:arkusuma@lzesoftware.com

Anda dapat mengunjungi website pembuat MiniSplit pada
  http://www.lzesoftware.com/

==============================================
 MiniSplit Copyright © 2002-2004 LZE Software
==============================================
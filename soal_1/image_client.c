// image_client.c - RPC client untuk mengirim file teks dan menerima file gambar
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
    int soket = socket(AF_INET, SOCK_STREAM, 0);
    if (soket < 0) {
        return 1;
    }
    struct sockaddr_in alamat;
    memset(&alamat, 0, sizeof(alamat));
    alamat.sin_family = AF_INET;
    alamat.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &alamat.sin_addr) <= 0) {
        close(soket);
        return 1;
    }
    if (connect(soket, (struct sockaddr*)&alamat, sizeof(alamat)) < 0) {
        fprintf(stderr, "Gagal koneksi ke server.\n");
        close(soket);
        return 1;
    }
    printf("Terhubung ke %s:%d\n\n", SERVER_IP, SERVER_PORT);

    char pilihan_str[10];
    int pilihan;
    while (1) {
        int judul_len = strlen("Klien Pengubah Gambar");
        int garis_len = judul_len + 4;
        for (int i = 0; i < garis_len; ++i) printf("=");
        printf("\n| %s |\n", "Klien Pengubah Gambar");
        for (int i = 0; i < garis_len; ++i) printf("=");
        printf("\n");
        printf("1. Kirim file ke server\n");
        printf("2. Unduh file dari server\n");
        printf("3. Keluar\n");
        printf(">> ");
        fflush(stdout);

        if (!fgets(pilihan_str, sizeof(pilihan_str), stdin)) {
            break;
        }
        pilihan = atoi(pilihan_str);

        if (pilihan == 1) {
            char nama_file[256];
            printf("Masukkan nama file: ");
            fflush(stdout);
            if (!fgets(nama_file, sizeof(nama_file), stdin)) {
                continue;
            }
            size_t len = strlen(nama_file);
            if (len > 0 && nama_file[len-1] == '\n') {
                nama_file[len-1] = '\0';
            }
            if (strlen(nama_file) == 0) {
                continue;
            }
            FILE *berkas = fopen(nama_file, "rb");
            if (!berkas) {
                printf("Nama file teks tidak ditemukan.\n");
                continue;
            }
            fseek(berkas, 0, SEEK_END);
            long ukuran = ftell(berkas);
            fseek(berkas, 0, SEEK_SET);
            char *isi = malloc(ukuran);
            if (!isi) {
                fclose(berkas);
                continue;
            }
            fread(isi, 1, ukuran, berkas);
            fclose(berkas);

            char perintah = '1';
            if (send(soket, &perintah, 1, 0) < 0) {
                printf("Gagal mengirim ke server.\n");
                free(isi);
                continue;
            }
            uint32_t panjang_nama = htonl(strlen(nama_file));
            send(soket, &panjang_nama, sizeof(panjang_nama), 0);
            send(soket, nama_file, strlen(nama_file), 0);
            uint32_t net_ukuran = htonl(ukuran);
            send(soket, &net_ukuran, sizeof(net_ukuran), 0);
            send(soket, isi, ukuran, 0);

            uint32_t panjang_balasan_net;
            ssize_t n = recv(soket, &panjang_balasan_net, sizeof(panjang_balasan_net), 0);
            if (n < (ssize_t)sizeof(panjang_balasan_net)) {
                printf("Koneksi terputus.\n");
                free(isi);
                break;
            }
            uint32_t panjang_balasan = ntohl(panjang_balasan_net);
            char *pesan = malloc(panjang_balasan + 1);
            if (!pesan) {
                free(isi);
                continue;
            }
            n = recv(soket, pesan, panjang_balasan, 0);
            if (n < (ssize_t)panjang_balasan) {
                printf("Koneksi terputus.\n");
                free(isi);
                free(pesan);
                break;
            }
            pesan[panjang_balasan] = '\0';
            printf("Server: %s\n\n", pesan);
            free(isi);
            free(pesan);

        } else if (pilihan == 2) {
            char nama_gambar[256];
            printf("Masukkan nama file: ");
            fflush(stdout);
            if (!fgets(nama_gambar, sizeof(nama_gambar), stdin)) {
                continue;
            }
            size_t len = strlen(nama_gambar);
            if (len > 0 && nama_gambar[len-1] == '\n') {
                nama_gambar[len-1] = '\0';
            }
            if (strlen(nama_gambar) == 0) {
                continue;
            }
            char perintah = '2';
            if (send(soket, &perintah, 1, 0) < 0) {
                printf("Gagal mengirim ke server.\n");
                continue;
            }
            uint32_t panjang_nama = htonl(strlen(nama_gambar));
            send(soket, &panjang_nama, sizeof(panjang_nama), 0);
            send(soket, nama_gambar, strlen(nama_gambar), 0);
            uint32_t ukuran_net;
            ssize_t n = recv(soket, &ukuran_net, sizeof(ukuran_net), 0);
            if (n < (ssize_t)sizeof(ukuran_net)) {
                printf("Koneksi terputus.\n");
                break;
            }
            uint32_t ukuran = ntohl(ukuran_net);
            if (ukuran == 0) {
                uint32_t panjang_msg_net;
                n = recv(soket, &panjang_msg_net, sizeof(panjang_msg_net), 0);
                if (n < (ssize_t)sizeof(panjang_msg_net)) {
                    printf("Koneksi terputus.\n");
                    break;
                }
                uint32_t panjang_msg = ntohl(panjang_msg_net);
                char *pesan = malloc(panjang_msg + 1);
                if (!pesan) {
                    continue;
                }
                n = recv(soket, pesan, panjang_msg, 0);
                if (n < (ssize_t)panjang_msg) {
                    printf("Koneksi terputus.\n");
                    free(pesan);
                    break;
                }
                pesan[panjang_msg] = '\0';
                printf("%s.\n\n", pesan);
                free(pesan);
            } else {
                unsigned char *data = malloc(ukuran);
                if (!data) {
                    continue;
                }
                size_t diterima = 0;
                while (diterima < ukuran) {
                    n = recv(soket, data + diterima, ukuran - diterima, 0);
                    if (n <= 0) {
                        printf("Koneksi terputus.\n");
                        free(data);
                        break;
                    }
                    diterima += n;
                }
                if (diterima < ukuran) {
                    free(data);
                    break;
                }
                FILE *keluar = fopen(nama_gambar, "wb");
                if (!keluar) {
                    printf("Gagal menyimpan file.\n");
                    free(data);
                    continue;
                }
                fwrite(data, 1, ukuran, keluar);
                fclose(keluar);
                printf("Berhasil! Gambar disimpan sebagai %s\n\n", nama_gambar);
                free(data);
            }

        } else if (pilihan == 3) {
            char perintah = '3';
            send(soket, &perintah, 1, 0);
            printf("Keluar...\n");
            break;
        } else {
            printf("Pilihan tidak valid.\n");
        }
    }

    close(soket);
    return 0;
}

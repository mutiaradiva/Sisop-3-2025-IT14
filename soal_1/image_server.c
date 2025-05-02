// image_server.c - Server daemon pengolah file terenkripsi menjadi gambar
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080
#define BACKLOG 5
#define BERKAS_LOG "server.log"

void balikkan_string(char *teks) {
    if (teks == NULL) return;
    size_t panjang = strlen(teks);
    for (size_t i = 0; i < panjang / 2; ++i) {
        char sementara = teks[i];
        teks[i] = teks[panjang - 1 - i];
        teks[panjang - 1 - i] = sementara;
    }
}

void catat_log(const char *sumber, const char *aksi, const char *info) {
    FILE *f = fopen(BERKAS_LOG, "a");
    if (!f) return;
    time_t waktu = time(NULL);
    struct tm info_tm;
    localtime_r(&waktu, &info_tm);
    char teks_waktu[20];
    strftime(teks_waktu, sizeof(teks_waktu), "%Y-%m-%d %H:%M:%S", &info_tm);
    fprintf(f, "[%s][%s]: %s %s\n", sumber, teks_waktu, aksi, info ? info : "");
    fclose(f);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    if (setsid() < 0) exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    mkdir("server", 0755);
    mkdir("server/database", 0755);

    int soket_peladen = socket(AF_INET, SOCK_STREAM, 0);
    if (soket_peladen < 0) exit(EXIT_FAILURE);

    int opsi = 1;
    setsockopt(soket_peladen, SOL_SOCKET, SO_REUSEADDR, &opsi, sizeof(opsi));

    struct sockaddr_in alamat_peladen;
    memset(&alamat_peladen, 0, sizeof(alamat_peladen));
    alamat_peladen.sin_family = AF_INET;
    alamat_peladen.sin_addr.s_addr = htonl(INADDR_ANY);
    alamat_peladen.sin_port = htons(PORT);

    if (bind(soket_peladen, (struct sockaddr*)&alamat_peladen, sizeof(alamat_peladen)) < 0) exit(EXIT_FAILURE);
    if (listen(soket_peladen, BACKLOG) < 0) exit(EXIT_FAILURE);

    struct sockaddr_in alamat_klien;
    socklen_t panjang_klien = sizeof(alamat_klien);

    while (1) {
        int soket_klien = accept(soket_peladen, (struct sockaddr*)&alamat_klien, &panjang_klien);
        if (soket_klien < 0) continue;

        char ip_klien[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &alamat_klien.sin_addr, ip_klien, INET_ADDRSTRLEN);
        char info_log[100];
        snprintf(info_log, sizeof(info_log), "Client connected from %s:%d", ip_klien, ntohs(alamat_klien.sin_port));
        catat_log("Client", "CONNECT", info_log);

        while (1) {
            char perintah;
            ssize_t n = recv(soket_klien, &perintah, 1, 0);
            if (n <= 0) {
                catat_log("Client", "DISCONNECT", ip_klien);
                close(soket_klien);
                break;
            }

            if (perintah == '1') {
                uint32_t pjg_nama_net;
                if (recv(soket_klien, &pjg_nama_net, sizeof(pjg_nama_net), 0) != sizeof(pjg_nama_net)) continue;
                uint32_t pjg_nama = ntohl(pjg_nama_net);
                char *namafile = malloc(pjg_nama + 1);
                if (!namafile) continue;
                if (recv(soket_klien, namafile, pjg_nama, 0) != (ssize_t)pjg_nama) {
                    free(namafile);
                    continue;
                }
                namafile[pjg_nama] = '\0';
                catat_log("Client", "UPLOAD", namafile);

                uint32_t ukuran_net;
                if (recv(soket_klien, &ukuran_net, sizeof(ukuran_net), 0) != sizeof(ukuran_net)) {
                    free(namafile);
                    continue;
                }
                uint32_t ukuran = ntohl(ukuran_net);
                char *buffer = malloc(ukuran + 1);
                if (!buffer) {
                    free(namafile);
                    continue;
                }
                size_t diterima = 0;
                while (diterima < ukuran) {
                    n = recv(soket_klien, buffer + diterima, ukuran - diterima, 0);
                    if (n <= 0) break;
                    diterima += n;
                }
                buffer[ukuran] = '\0';

                char *bersih = malloc(ukuran + 1);
                if (!bersih) {
                    free(namafile);
                    free(buffer);
                    continue;
                }
                size_t j = 0;
                for (size_t i = 0; i < ukuran; ++i) {
                    char c = buffer[i];
                    if (c != '\n' && c != '\r' && c != ' ' && c != '\t') {
                        bersih[j++] = c;
                    }
                }
                bersih[j] = '\0';
                balikkan_string(bersih);

                size_t panjang_hex = strlen(bersih);
                size_t panjang_data = panjang_hex / 2;
                unsigned char *data_gambar = malloc(panjang_data);
                if (!data_gambar) {
                    free(namafile);
                    free(buffer);
                    free(bersih);
                    continue;
                }
                for (size_t i = 0; i < panjang_data; ++i) {
                    char dua[3] = { bersih[2*i], bersih[2*i + 1], '\0' };
                    data_gambar[i] = (unsigned char) strtol(dua, NULL, 16);
                }

                time_t sekarang = time(NULL);
                char nama_keluaran[64];
                snprintf(nama_keluaran, sizeof(nama_keluaran), "%ld.jpeg", sekarang);
                char path_keluaran[128];
                snprintf(path_keluaran, sizeof(path_keluaran), "server/database/%s", nama_keluaran);
                FILE *keluar = fopen(path_keluaran, "wb");
                if (!keluar) {
                    const char *pesan = "Error saving file on server";
                    uint32_t panjang = htonl(strlen(pesan));
                    send(soket_klien, &panjang, sizeof(panjang), 0);
                    send(soket_klien, pesan, strlen(pesan), 0);
                    free(namafile); free(buffer); free(bersih); free(data_gambar);
                    continue;
                }
                fwrite(data_gambar, 1, panjang_data, keluar);
                fclose(keluar);
                catat_log("Server", "SAVE", nama_keluaran);

                char balasan[128];
                snprintf(balasan, sizeof(balasan), "Text decrypted and saved as %s", nama_keluaran);
                uint32_t pjg_balasan = htonl(strlen(balasan));
                send(soket_klien, &pjg_balasan, sizeof(pjg_balasan), 0);
                send(soket_klien, balasan, strlen(balasan), 0);
                free(namafile); free(buffer); free(bersih); free(data_gambar);

            } else if (perintah == '2') {
                uint32_t pjg_nama_net;
                if (recv(soket_klien, &pjg_nama_net, sizeof(pjg_nama_net), 0) != sizeof(pjg_nama_net)) continue;
                uint32_t pjg_nama = ntohl(pjg_nama_net);
                char *namafile = malloc(pjg_nama + 1);
                if (!namafile) continue;
                if (recv(soket_klien, namafile, pjg_nama, 0) != (ssize_t)pjg_nama) {
                    free(namafile);
                    continue;
                }
                namafile[pjg_nama] = '\0';
                catat_log("Client", "DOWNLOAD", namafile);
                char path_gambar[128];
                snprintf(path_gambar, sizeof(path_gambar), "server/database/%s", namafile);
                FILE *f = fopen(path_gambar, "rb");
                if (!f) {
                    catat_log("Server", "ERROR", "Requested image not found");
                    uint32_t nol = htonl(0);
                    send(soket_klien, &nol, sizeof(nol), 0);
                    const char *pesan = "File JPEG tidak ditemukan";
                    uint32_t panjang = htonl(strlen(pesan));
                    send(soket_klien, &panjang, sizeof(panjang), 0);
                    send(soket_klien, pesan, strlen(pesan), 0);
                    free(namafile);
                    continue;
                }
                fseek(f, 0, SEEK_END);
                long ukuran = ftell(f);
                fseek(f, 0, SEEK_SET);
                unsigned char *data = malloc(ukuran);
                if (!data) {
                    fclose(f);
                    free(namafile);
                    continue;
                }
                fread(data, 1, ukuran, f);
                fclose(f);
                uint32_t net_ukuran = htonl(ukuran);
                send(soket_klien, &net_ukuran, sizeof(net_ukuran), 0);
                send(soket_klien, data, ukuran, 0);
                catat_log("Server", "SEND", namafile);
                free(data);
                free(namafile);

            } else if (perintah == '3') {
                catat_log("Client", "DISCONNECT", "Client requested exit");
                close(soket_klien);
                break;
            } else {
                catat_log("Server", "ERROR", "Unknown command received");
                close(soket_klien);
                break;
            }
        }
    }

    close(soket_peladen);
    return 0;
}

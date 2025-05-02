#include "order.h"
#include <string.h>
#include <unistd.h>

void muat_csv_ke_memori(Order* data) {
    FILE* file = fopen("delivery_order.csv", "r");
    if (!file) return;
    char baris[256];
    int i = 0;
    while (fgets(baris, sizeof(baris), file) && i < MAX_ORDERS) {
        char* nama = strtok(baris, ",\n");
        char* alamat = strtok(NULL, ",\n");
        char* jenis = strtok(NULL, ",\n");
        if (nama && alamat && jenis) {
            strncpy(data[i].nama, nama, MAX_NAME_LEN);
            strncpy(data[i].alamat, alamat, MAX_ADDRESS_LEN);
            strncpy(data[i].jenis, jenis, 10);
            data[i].status = PENDING;
            strcpy(data[i].delivered_by, "-");
            i++;
        }
    }
    fclose(file);
}

void perintah_kirim(char* target, char* pengirim, Order* data) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(data[i].nama, target) == 0 && strcmp(data[i].jenis, "Reguler") == 0 && data[i].status == PENDING) {
            data[i].status = DELIVERED;
            strncpy(data[i].delivered_by, pengirim, MAX_NAME_LEN);

            FILE* log = fopen("delivery.log", "a");
            if (log) {
                time_t t = time(NULL);
                struct tm* tm_info = localtime(&t);
                char waktu[64];
                strftime(waktu, sizeof(waktu), "%d/%m/%Y %H:%M:%S", tm_info);
                fprintf(log, "[%s] [AGENT %s] Reguler package delivered to %s in %s\n",
                        waktu, pengirim, data[i].nama, data[i].alamat);
                fclose(log);
            }
            break;
        }
    }
}

void perintah_status(char* nama, Order* data) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(data[i].nama, nama) == 0) {
            if (data[i].status == DELIVERED)
                printf("Status for %s: Delivered by %s\n", nama, data[i].delivered_by);
            else
                printf("Status for %s: Pending\n", nama);
            return;
        }
    }
    printf("Status for %s: Not found\n", nama);
}

void perintah_list(Order* data) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strlen(data[i].nama) > 0) {
            if (data[i].status == DELIVERED) {
                printf("Status for %s: Delivered by %s\n", data[i].nama, data[i].delivered_by);
            } else {
                printf("Status for %s: Pending\n", data[i].nama);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int id;
    Order* data = attach_shared_memory(&id);

    int kosong = 1;
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strlen(data[i].nama) > 0) {
            kosong = 0;
            break;
        }
    }
    if (kosong) {
        muat_csv_ke_memori(data);
    }

    if (argc == 3 && strcmp(argv[1], "-deliver") == 0) {
        char* user = getenv("USER");
        if (!user) user = "USER";
        perintah_kirim(argv[2], user, data);
    } else if (argc == 3 && strcmp(argv[1], "-status") == 0) {
        perintah_status(argv[2], data);
    } else if (argc == 2 && strcmp(argv[1], "-list") == 0) {
        perintah_list(data);
    } else {
        printf("Penggunaan:\n");
        printf("  ./dispatcher -deliver [Nama]\n");
        printf("  ./dispatcher -status [Nama]\n");
        printf("  ./dispatcher -list\n");
    }

    detach_shared_memory(data);
    return 0;
}

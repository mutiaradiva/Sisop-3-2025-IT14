#include "order.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void* jalankan_agen(void* arg) {
    char* agen = (char*) arg;
    int id;
    Order* daftar = attach_shared_memory(&id);

    while (1) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(daftar[i].jenis, "Express") == 0 && daftar[i].status == PENDING) {
                daftar[i].status = DELIVERED;
                strncpy(daftar[i].delivered_by, agen, MAX_NAME_LEN);

                FILE* log = fopen("delivery.log", "a");
                if (log) {
                    time_t t = time(NULL);
                    struct tm* tm_info = localtime(&t);
                    char waktu[64];
                    strftime(waktu, sizeof(waktu), "%d/%m/%Y %H:%M:%S", tm_info);

                    fprintf(log, "[%s] [%s] Express package delivered to %s in %s\n",
                            waktu, agen, daftar[i].nama, daftar[i].alamat);
                    fclose(log);
                }

                printf("[%s] mengantar paket Express ke %s\n", agen, daftar[i].nama);
                sleep(1); // simulasikan waktu antar
            }
        }
        sleep(2);
    }

    detach_shared_memory(daftar);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, jalankan_agen, "AGENT A");
    pthread_create(&t2, NULL, jalankan_agen, "AGENT B");
    pthread_create(&t3, NULL, jalankan_agen, "AGENT C");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}

# LAPRES Praktikum Sistem Operasi Modul 3 - IT14

## Anggota
1. Muhammad Fatihul Qolbi Ash Shiddiqi (5027241023)
2. Mutiara Diva Jaladitha (5027241083)
3. M. Faqih Ridho (5027241123)

## DAFTAR ISI
- [Soal 1](#soal-1)
- [Soal 2](#soal-2)
- [Soal 3](#soal-3)
- [Soal 4](#soal-4)

# soal-1
**Dikerjakan oleh M. Faqih Ridho (5027241123)**

##  Penjelasan Soal Nomor 1 (a–g)
### A. Struktur Direktori
File teks rahasia yang harus dikonversi berada dalam file .zip yang dapat diekstrak secara manual. Setelah proses unzip dan kompilasi.


### B.  image_server.c sebagai Daemon
Program image_server.c berjalan secara daemon di background menggunakan fork() dan setsid(). Program ini:

Menunggu koneksi client di port 8080.

Tidak menampilkan output terminal.

Siap menerima perintah RPC untuk pemrosesan file.

### C.  Fungsi image_client.c
Program client (image_client.c) memungkinkan pengguna untuk:

Mengkirim file terenkripsi

Klien mengirim file .txt yang berisi hex dibalik (reverse string) ke server.

Server melakukan:

Membalik teks (reverse).

Dekode hex → binary → simpan sebagai .jpeg berdasarkan timestamp.

Meminta file JPEG

Klien meminta nama file JPEG (misal 1744401282.jpeg).

Server mengirim file tersebut ke klien.

File hasil diunduh ke folder client/.

 File tidak pernah dicopy atau dipindah — semua data dikirim melalui socket RPC.

### D.  Menu Interaktif image_client.c
Klien menyediakan menu interaktif seperti:
```
==============================
| Klien Pengubah Gambar      |
==============================
1. Kirim file ke server
2. Unduh file dari server
3. Keluar
>>
```

Pengguna dapat memasukkan perintah berkali-kali tanpa harus menjalankan ulang program.

### E.  Output JPEG yang Valid
Setelah klien mengirim file teks, server akan menyimpan hasil dekripsi ke:

server/database/1744401282.jpeg
Klien dapat mengunduh file ini dan membuka hasilnya sebagai file gambar JPEG yang valid.

### F.  Penanganan Error
Dari Klien:

Gagal koneksi ke server. → jika server tidak aktif.

Nama file teks tidak ditemukan. → jika file input salah/tidak ada.

Dari Server:

File JPEG tidak ditemukan. → dikirim kembali ke klien jika file tidak tersedia.

Server tidak pernah crash, dan selalu mengirim pesan respons atau mencatat error ke log.

### G  server.log: Format Pencatatan Aktivitas
Semua interaksi client-server dicatat ke:

server/server.log
Dengan format:

[Client][YYYY-MM-DD hh:mm:ss]: [DECRYPT] [Text data]
[Server][YYYY-MM-DD hh:mm:ss]: [SAVE] [1744401282.jpeg]
[Client][YYYY-MM-DD hh:mm:ss]: [DOWNLOAD] [1744401282.jpeg]
[Server][YYYY-MM-DD hh:mm:ss]: [SEND] [1744401282.jpeg]
Log mencakup semua:
Koneksi,Upload,Download,Error,Exit.



# soal-2
**Dikerjakan oleh  M. Faqih Ridho (5027241123)**

### A. Mengunduh dan unzip

```
wget --content-disposition "https://drive.google.com/uc?export=download&id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9"
```

### B. Pengiriman Bertipe Express
Kode berikut ditulis dalam file `delivery_agent.c`. Tiga thread dibuat dengan `pthread_create`, lalu masing-masing thread menjalankan fungsi `jalankan_agen()` untuk memeriksa shared memory dan memproses order Express yang pending.

```
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
                sleep(1);
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
```
- Fungsi jalankan_agen() digunakan oleh semua agen.

- Setiap agen memeriksa array Order dalam shared memory.

- Jika menemukan order bertipe "Express" dan status PENDING, maka:

- Status diubah menjadi DELIVERED,

- Nama agen dicatat di field delivered_by,

- Log dicetak ke file delivery.log.

- Log ini sesuai dengan format yang ditentukan oleh soal. Proses berjalan terus-menerus dalam loop while(1) sehingga agen akan terus mengecek order baru.


### C.Pengiriman Bertipe Reguler
```
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
                strftime(waktu, sizeof(waktu), "%d/%m/%Y %H:%M:%S\", tm_info);
                fprintf(log, "[%s] [AGENT %s] Reguler package delivered to %s in %s\\n",
                        waktu, pengirim, data[i].nama, data[i].alamat);
                fclose(log);
            }
            break;
        }
    }
}
```
- Fungsi ini dipanggil ketika user memberikan perintah ./dispatcher -deliver [Nama].

- Program akan mencari nama target di array shared memory.

- Jika ditemukan dan jenisnya "Reguler" serta statusnya masih PENDING, maka:

- Status diubah menjadi DELIVERED,

- Nama user disimpan sebagai delivered_by,

- Catatan pengiriman ditulis ke file delivery.log sesuai format soal.

  **Pemanggilan fungsi**
  
  ```
  if (argc == 3 && strcmp(argv[1], "-deliver") == 0) {
    char* user = getenv("USER");
    if (!user) user = "USER";
    perintah_kirim(argv[2], user, data);
} 


- Bagian ini ada di main() dispatcher.

- Mendapatkan nama user dari environment variable USER.

- Lalu memanggil perintah_kirim.

### D. Mengecek Status Pesanan

```
void perintah_status(char* nama, Order* data) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(data[i].nama, nama) == 0) {
            if (data[i].status == DELIVERED)
                printf("Status for %s: Delivered by %s\\n", nama, data[i].delivered_by);
            else
                printf("Status for %s: Pending\\n", nama);
            return;
        }
    }
    printf("Status for %s: Not found\\n", nama);
}
```

- Fungsi ini mencari nama dalam array order.

Jika ditemukan:

- Jika statusnya DELIVERED, maka ditampilkan Delivered by [agent].

- Jika belum dikirim, maka ditampilkan Pending.

- Jika nama tidak ditemukan, tampilkan Not found.

    ** Panggilan fungsi **
```
else if (argc == 3 && strcmp(argv[1], "-status") == 0) {
    perintah_status(argv[2], data);
}
```

### E. Melihat Daftar Semua Pesanan

```
void perintah_list(Order* data) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strlen(data[i].nama) > 0) {
            if (data[i].status == DELIVERED) {
                printf("Status for %s: Delivered by %s\\n", data[i].nama, data[i].delivered_by);
            } else {
                printf("Status for %s: Pending\\n", data[i].nama);
            }
        }
    }
}

```
- Fungsi ini menampilkan semua order yang ada di shared memory.

Untuk setiap order:

- Jika statusnya DELIVERED, ditampilkan nama dan agen pengantar.

- Jika PENDING, hanya ditampilkan statusnya.

Cocok untuk perintah -list agar user tahu semua progress pengiriman.

      ** Pemanggilan Fungsi **
```
else if (argc == 2 && strcmp(argv[1], "-list") == 0) {
    perintah_list(data);
}
```


# soal-3
**Dikerjakan oleh Mutiara Diva Jaladitha (5027241083)**

## a. dungeon.c akan bekerja sebagai server yang dimana client (player.c) dapat terhubung melalui RPC. dungeon.c akan memproses segala perintah yang dikirim oleh player.c. Lebih dari 1 client dapat mengakses server.

### 1. Inisialisasi Socket Server
```
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
...
bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
listen(server_fd, MAX_CLIENTS);
printf("Server listening on port %d...\n", PORT);
```
- Membuka socket TCP (`SOCK_STREAM`).
- Mengikat ke alamat lokal (`INADDR_ANY`) dan port `8080`.
- Mulai mendengarkan koneksi masuk hingga `MAX_CLIENTS`.

### 2. Menerima Banyak Client
```
*client_sock = accept(server_fd, NULL, NULL);
...
pthread_create(&tid, NULL, handle_client, client_sock);
pthread_detach(tid);
```
- `accept()` menerima koneksi dari client `player.c`.
- Untuk setiap client baru, dibuat thread baru `handle_client`.
- Ini memungkinkan banyak player terhubung secara paralel.

### 3. Semua state dan logika game ada di server (dungeon.c)
```
PlayerState ps = { ... };
...
switch(choice) {
    case 1: show_stats(client_sock, &ps); break;
    case 2: show_shop(client_sock, &ps); break;
    case 3: show_inventory(client_sock, &ps); break;
    case 4: battle_mode(client_sock, &ps); break;
    case 5: 
        send_str(client_sock, "Goodbye!\n");
        close(client_sock);
        return NULL;
}
```
- Semua logika seperti statistik, belanja senjata, inventory, equip, hingga battle mode ada di server.
- `player.c` hanya mengirim angka/menu, server yang proses dan merespons.

## b. Ketika player.c dijalankan, ia akan terhubung ke dungeon.c dan menampilkan sebuah main menu

### 1. Terhubung ke dungeon.c (server)
```
int sock = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in serv_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(PORT),
    .sin_addr.s_addr = inet_addr("127.0.0.1")
};

connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
```
- `socket()` membuat koneksi TCP.
- `connect()` menyambungkan client ke server di `127.0.0.1:8080`, yaitu `dungeon.c`.

### 2. Menerima dan Menampilkan Main Menu
```
char buffer[1024];
int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
...
buffer[bytes] = '\0';
printf("%s", buffer);
```
- `recv()` menunggu pesan dari `dungeon.c`.
- Jika `dungeon.c` mengirim main menu, maka akan ditampilkan lewat `printf("%s", buffer)`;.

### 3. Menanggapi Main Menu (Input User)
```
if (strstr(buffer, "Choose an option:") || strstr(buffer, "Enter")) {
    int choice;
    scanf("%d", &choice);
    send(sock, &choice, sizeof(choice), 0);
}
```
- Ketika isi buffer memuat string seperti `"Choose an option:"`, artinya server meminta input menu.
- Pemain memasukkan pilihan → dikirim ke server.

### Output
<img width="277" alt="image" src="https://github.com/user-attachments/assets/8c7c8133-a9f7-4559-b09a-82ca27d63283" />

## c. Jika opsi Show Player Stats dipilih, maka program akan menunjukan Uang yang dimiliki (Jumlah dibebaskan), senjata yang sedang digunakan, Base Damage, dan jumlah musuh yang telah dimusnahkan.
```
void show_stats(int sock, PlayerState* ps) {
    char buffer[1024];
    int base_damage = (ps->equipped_weapon.damage == 0) ?
        ps->base_damage :
        ps->equipped_weapon.damage;

    snprintf(buffer, sizeof(buffer),
        GREEN "\n=== PLAYER STATS ===\n" RESET
        "Gold: %d | Equipped Weapon: %s | Base Damage: %d",
        ps->gold, 
        ps->equipped_weapon.name,
        base_damage);

    if(ps->equipped_weapon.passive != NONE) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
            " | Passive: %s",
            get_passive_name(ps->equipped_weapon.passive));
    }

    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
        " | Kills: %d\n",
        ps->kills);

    send_str(sock, buffer);
}
```
- `ps->gold`: Jumlah uang pemain.
- `ps->equipped_weapon.name`: Nama senjata yang sedang digunakan.
- `base_damage`: Damage dari senjata yang digunakan (default ke `base_damage` jika pakai Fists).
- `ps->kills`: Jumlah musuh yang dimusnahkan.
- Semua ini disusun dalam `buffer` dan dikirim ke klien melalui `send_str(sock, buffer);`.

### Output
<img width="516" alt="image" src="https://github.com/user-attachments/assets/bf6eade8-0130-4306-a0fa-9fb9ec07af16" />

## d. Disaat opsi Shop dipilih, program akan menunjukan senjata apa saja yang dapat dibeli beserta harga, damage, dan juga passive (jika ada)

### Fungsi show_shop_weapons() di shop.c
```
void show_shop_weapons(int sock) {
    char buffer[256];
    send_str(sock, YELLOW "\n=== WEAPON SHOP ===\n" RESET);
    for (int i = 1; i < shop_size; i++) {
        Weapon w = shop[i];
        snprintf(buffer, sizeof(buffer), "[%d] %s - %d Gold - %d Damage", 
            i, w.name, w.price, w.damage);

        if (w.passive != NONE) {
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
                     " (Passive: %s)", get_passive_name(w.passive));
        }

        strcat(buffer, "\n");
        send_str(sock, buffer);
    }
    send_str(sock, "\nEnter weapon number to buy or 0 to cancel: ");
}
```

### Fungsi show_shop() di dungeon.c
```
void show_shop(int sock, PlayerState* ps) {
    show_shop_weapons(sock);  // <--- ini bagian penting yang menampilkan senjata
    int choice = receive_choice(sock);

    if(choice > 0 && choice < shop_size) {
        buy_weapon(sock, choice, &ps->gold, ps->inventory, &ps->inventory_count);
    } else if(choice != 0) {
        send_str(sock, RED "Invalid choice!\n" RESET);
    }
    clear_input_buffer(sock);
}
```
- show_shop_weapons(sock) memunculkan semua senjata di toko.
- receive_choice() menerima nomor senjata dari player.
- buy_weapon() digunakan jika player memilih membeli.

### Output
<img width="554" alt="image" src="https://github.com/user-attachments/assets/8d7bbe86-fcdd-4c15-952c-07dc2a06cc36" />

## e. Jika opsi View Inventory dipilih, program akan menunjukan senjata apa saja yang dimiliki dan dapat dipakai (jika senjata memiliki passive, tunjukan juga passive tersebut). Apabila opsi Show Player Stats dipilih saat menggunakan weapon maka Base Damage player akan berubah dan jika memiliki passive, maka akan ada status tambahan yaitu Passive.

### View Inventory – Fungsi show_inventory()
```
void show_inventory(int sock, PlayerState* ps) {
    char buffer[1024];

    send_str(sock, YELLOW "\n=== INVENTORY ===\n" RESET);

    snprintf(buffer, sizeof(buffer), "[0] Fists%s\n", 
        (ps->equipped_weapon.damage == 0) ? " (EQUIPPED)" : "");
    send_str(sock, buffer);

    for(int i = 0; i < ps->inventory_count; i++) {
        Weapon w = ps->inventory[i];
        snprintf(buffer, sizeof(buffer), "[%d] %s%s%s)%s\n",
            i+1,
            w.name,
            (w.passive != NONE) ? " (Passive: " : "",
            (w.passive != NONE) ? get_passive_name(w.passive) : "",
            (strcmp(w.name, ps->equipped_weapon.name) == 0) ? " (EQUIPPED)" : ""
        );
        send_str(sock, buffer);
    }

    send_str(sock, "\nEnter item number to equip or 0 to cancel: ");

    int choice;
    recv(sock, &choice, sizeof(choice), 0);

    if(choice != 0) {
        if(choice > 0 && choice <= ps->inventory_count) {
            ps->equipped_weapon = ps->inventory[choice-1];
            send_str(sock, GREEN "\nEquipment updated!\n" RESET);
        } else {
            send_str(sock, RED "\nInvalid selection!\n" RESET);
        }
    }
}
```
- Semua senjata yang dimiliki pemain ditampilkan.
- Jika senjata memiliki passive, akan muncul (Passive: NamaPassive).
- Senjata yang sedang digunakan akan muncul dengan label (EQUIPPED).

### Output
<img width="454" alt="image" src="https://github.com/user-attachments/assets/0cda3024-e66e-4fe2-be56-4facf27b2b55" />

### Show Player Stats – Fungsi show_stats()
```
void show_stats(int sock, PlayerState* ps) {
    char buffer[1024];
    int base_damage = (ps->equipped_weapon.damage == 0) ?
                      ps->base_damage :
                      ps->equipped_weapon.damage;

    snprintf(buffer, sizeof(buffer),
        GREEN "\n=== PLAYER STATS ===\n" RESET
        "Gold: %d | Equipped Weapon: %s | Base Damage: %d",
        ps->gold, 
        ps->equipped_weapon.name,
        base_damage);

    if(ps->equipped_weapon.passive != NONE) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
            " | Passive: %s",
            get_passive_name(ps->equipped_weapon.passive));
    }

    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
        " | Kills: %d\n",
        ps->kills);

    send_str(sock, buffer);
}
```
Menampilkan:
- Gold (uang),
- Nama senjata yang digunakan,
- Base Damage berdasarkan damage senjata yang digunakan,
- Passive jika senjata punya passive,
- Jumlah kill.

### Output
<img width="764" alt="image" src="https://github.com/user-attachments/assets/757a96bb-27e4-4122-98f5-429b40d0ac63" />

## f. Opsi Battle Mode

### Menampilkan Health Bar Musuh
```
snprintf(buffer, sizeof(buffer),
    RED "\n[ENEMY]: %s\nHP: [%s%s] (%d/%d)\n" RESET,
    enemy.name,
    get_hp_bar(enemy.hp, enemy.max_hp),
    get_hp_bar_blank(enemy.hp, enemy.max_hp),
    enemy.hp, enemy.max_hp
);
send_str(sock, buffer);
```
- Menampilkan nama musuh.
- Health bar visual: gabungan fungsi get_hp_bar() dan get_hp_bar_blank() yang menggambarkan darah musuh secara visual seperti ini: [██████ ] (60/100).

### Input Pemain: attack atau exit
```
recv(sock, input, sizeof(input), 0);

if(strncmp(input, "exit", 4) == 0) {
    send_str(sock, YELLOW "\nYou fled the battle.\n" RESET);
    break;
}
if(strncmp(input, "attack", 6) == 0) {
    int damage = (ps->equipped_weapon.damage == 0) ?
                 ps->base_damage :
                 ps->equipped_weapon.damage;

    enemy.hp -= damage;
    snprintf(buffer, sizeof(buffer),
        GREEN "\nYou dealt %d damage to %s!\n" RESET,
        damage, enemy.name);
    send_str(sock, buffer);
```
- Jika player mengetik exit → keluar dari Battle Mode.
- Damage disesuaikan dengan senjata yang digunakan.
- HP musuh dikurangi, dan pesan damage dikirim ke client.

### Output
<img width="392" alt="image" src="https://github.com/user-attachments/assets/77ada146-87fe-4075-9ea0-aae306806350" />

### Jika musuh mati: Reward + Musuh Baru
```
if(enemy.hp <= 0) {
    ps->kills++;
    int reward = enemy.max_hp * 10;
    ps->gold += reward;

    snprintf(buffer, sizeof(buffer),
        GREEN "\nYou defeated %s!\nRewards: %d Gold\n" RESET,
        enemy.name, reward);
    send_str(sock, buffer);

    enemy = generate_enemy(); // Spawn new enemy
}
```
Jika HP musuh 0 atau kurang:
- Jumlah kills pemain bertambah.
- Pemain mendapatkan reward berdasarkan enemy.max_hp * 10.
- Musuh baru otomatis muncul.

### Output
<img width="400" alt="image" src="https://github.com/user-attachments/assets/f202d738-19fa-4d28-a2bd-af1d10d64243" />

## g. Other Battle Logic

### Darah Musuh Acak: generate_enemy()
```
Enemy generate_enemy() {
    Enemy e;
    strcpy(e.name, "Goblin");

    e.max_hp = 50 + rand() % 151; // HP antara 50–200
    e.hp = e.max_hp;

    return e;
}
```
- 50 + rand() % 151 artinya akan menghasilkan nilai 50 sampai 200.
- HP maksimum dan current HP di-set ke nilai ini.

### Damage Acak + Critical Hit + Passive Weapon: Di dalam battle_mode()
```
int base = (ps->equipped_weapon.damage == 0) ?
           ps->base_damage :
           ps->equipped_weapon.damage;

// Damage acak: base + [0 - 4]
int damage = base + (rand() % 5);

// Critical chance: 20%
if (rand() % 100 < 20) {
    damage *= 2;
    send_str(sock, MAGENTA "[CRITICAL HIT!] " RESET);
}

// Passive weapon effect
if (strlen(ps->equipped_weapon.passive) > 0) {
    if (rand() % 100 < 30) { // 30% chance passive aktif
        send_str(sock, GREEN "[PASSIVE ACTIVATED!] " RESET);
        // contoh efek tambahan: +5 damage
        damage += 5;
    }
}
```
- Base damage diambil dari weapon yang sedang dipakai. Jika tidak memakai senjata, maka dari base_damage.
- Damage acak: nilai damage ditambah angka acak dari 0–4.
- Critical hit: ada peluang 20% (rand() % 100 < 20) untuk melipatgandakan damage.
- Passive effect: jika senjata punya passive (dicek via strlen(passive) > 0), maka ada 30% chance untuk aktif. Efek bisa berupa damage tambahan.

### Reward Acak Saat Musuh Kalah
```
int reward = (rand() % 21 + 10) * (enemy.max_hp / 50); // 10-30 gold dikali skala HP
ps->gold += reward;

snprintf(buffer, sizeof(buffer),
    GREEN "\nYou defeated %s!\nRewards: %d Gold\n" RESET,
    enemy.name, reward);
send_str(sock, buffer);
```
- Gold yang diperoleh bersifat dinamis, tergantung HP musuh.
- Misalnya jika musuh punya 100 HP → (10–30)*2 = 20–60 gold.

## h. Error Handling
### Error Handling di Weapon Shop
```
if(choice > 0 && choice < shop_size) {
        buy_weapon(sock, choice, &ps->gold, ps->inventory, &ps->inventory_count);
    } else if(choice != 0) {
        send_str(sock, RED "Invalid choice!\n" RESET);
    }
```
### Output
<img width="545" alt="image" src="https://github.com/user-attachments/assets/c5907f3e-ea41-43aa-8176-f15adf4351f0" />

### Error Handling di Inventory
```
 if(choice != 0) {
        if(choice > 0 && choice <= ps->inventory_count) {
            ps->equipped_weapon = ps->inventory[choice-1];
            send_str(sock, GREEN "\nEquipment updated!\n" RESET);
        } else {
            send_str(sock, RED "\nInvalid selection!\n" RESET);
        }
    }
```
### Output
<img width="452" alt="image" src="https://github.com/user-attachments/assets/784499f6-0713-4660-9a76-806d3d10ca21" />

### Error Handling di Battle Mode
```
        else {
            send_str(sock, RED "\nInvalid command! Use 'attack' or 'exit'.\n" RESET);
        }
```
### Output
<img width="388" alt="image" src="https://github.com/user-attachments/assets/ed787a07-3feb-408f-921e-e2f3e688be2a" />

### Error Handling di Main Menu
```
if(choice < 1 || choice > 5) {
            send_str(client_sock, RED "\nInvalid choice!\n" RESET);
            continue;
        }
```
### Output
<img width="273" alt="image" src="https://github.com/user-attachments/assets/c4e08f69-69d4-45d6-b72b-774ae439f0a7" />

# soal-4
**Dikerjakan oleh Muhammad Fatihul Qolbi Ash Shiddiqi (5027241023)**

## Deskripsi Soal 

Sung Jin-Woo, seorang hunter ternama, tengah mengembangkan sebuah sistem manajemen hunter berbasis terminal menggunakan bahasa C. Sistem ini bertujuan untuk mengatur interaksi antar hunter dan dungeon secara real-time dengan memanfaatkan shared memory (IPC). Proyek ini dibagi menjadi dua bagian utama:

- `system.c` : Program utama yang mengatur shared memory, dungeon, dan administrasi hunter.
- `hunter.c` : Program untuk hunter yang digunakan untuk registrasi, login, melihat dungeon, menyerang dungeon, dan bertarung antar hunter.
 
#### A. Membuat System.c dan Hunter.c dengan catatan hunter.c bisa dijalankan ketika sistem sudah dijalankan.

#### hunter.c 
```C
key_t key_hunter = 1234;
shmid_hunter = shmget(key_hunter, sizeof(struct SystemData), IPC_CREAT | 0666);
data = (struct SystemData*) shmat(shmid_hunter, NULL, 0);
if (data->num_hunters < 0 || data->num_hunters > MAX_HUNTERS) {
    data->num_hunters = 0;
}

key_t key_dungeon = 5678;
shmid_dungeon = shmget(key_dungeon, sizeof(struct DungeonData), IPC_CREAT | 0666);
d_data = (struct DungeonData*) shmat(shmid_dungeon, NULL, 0);
if (d_data->num_dungeons < 0 || d_data->num_dungeons > MAX_DUNGEONS) {
    d_data->num_dungeons = 0;
}
```
- `key_t key_hunter = 1234;` → Membuat key unik untuk shared memory hunter. Nilai 1234 akan digunakan kembali di file lain untuk mengakses memori yang sama.
- `shmid_hunter = shmget(...);` → Mengalokasikan shared memory untuk SystemData. Flag IPC_CREAT | 0666 artinya: buat jika belum ada dan beri permission rw-rw-rw-.
- `data = (struct SystemData*) shmat(...);` → Menempelkan (attach) shared memory ke proses dan mengembalikannya dalam pointer data.
- `if (data->num_hunters < 0 ...` → Validasi data: jika nilai awal num_hunters tidak wajar (misal karena memori baru atau rusak), set ulang ke 0.
- `key_t key_dungeon = 5678;` → Key unik untuk shared memory dungeon, serupa dengan hunter.
- `shmid_dungeon = shmget(...);` → Alokasi memori untuk dungeon.
- `d_data = (struct DungeonData*) shmat(...)`; → Attach shared memory dungeon ke pointer d_data.
- `if (d_data->num_dungeons < 0 ...` → Validasi awal jumlah dungeon untuk mencegah pembacaan data korup.

#### system.c 
```C
key_t key_hunter = 1234;
int shmid_hunter = shmget(key_hunter, sizeof(struct SystemData), 0666);
if (shmid_hunter == -1) {
    perror("Shared memory for hunter not found. Please run system first.");
    exit(1);
}
struct SystemData* data = (struct SystemData*) shmat(shmid_hunter, NULL, 0);

key_t key_dungeon = 5678;
int shmid_dungeon = shmget(key_dungeon, sizeof(struct DungeonData), 0666);
if (shmid_dungeon == -1) {
    perror("Shared memory for dungeon not found.");
    exit(1);
}
struct DungeonData* d_data = (struct DungeonData*) shmat(shmid_dungeon, NULL, 0);
```
- `key_t key_hunter = 1234;` → Gunakan key yang sama dengan system.c untuk mengakses shared memory hunter.
- `shmid_hunter = shmget(..., 0666);` → Akses shared memory hunter. Tanpa IPC_CREAT, artinya hunter hanya bisa berjalan jika system sudah aktif.
- `if (shmid_hunter == -1) { ... }` → Jika tidak ditemukan, tampilkan error dan keluar. Mencegah hunter dijalankan lebih dulu.
- `shmat(...);` → Attach pointer shared memory hunter ke data.
- `key_t key_dungeon = 5678;` → Key dungeon harus sama dengan di system.c agar sinkron.
- `shmid_dungeon = shmget(..., 0666);` → Akses memori dungeon tanpa membuat baru.
- `if (shmid_dungeon == -1)` { ... } → Cek apakah dungeon memory tersedia. Jika tidak, keluar dengan pesan error.
- `shmat(...);` → Attach pointer shared memory dungeon ke d_data.

#### Output 

#### system.c 
![Screenshot 2025-05-08 221151](https://github.com/user-attachments/assets/8d979568-5df2-4a18-97ba-d0ca156c8ad9)

#### hunter.c
![Screenshot 2025-05-08 221320](https://github.com/user-attachments/assets/0c9e4580-7a08-463e-9474-8a5421bd35d9)

#### hunter.c jika belum menjalankan system.c
![Screenshot 2025-05-08 221220](https://github.com/user-attachments/assets/6ea483a8-55ff-459f-a77a-0862b77c5c47)



#### B. Registrasi dan login serta stats awal 

#### hunter.c 

#### Register

```C
if (choice == 1) {
    printf("Enter username to register: ");
    scanf("%s", username);
    getchar();

    int exists = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, username) == 0) {
            exists = 1;
            break;
        }
    }

    if (exists) {
        printf("Username already exists.\n");
    } else if (data->num_hunters >= MAX_HUNTERS) {
        printf("Max hunter limit reached.\n");
    } else {
        struct Hunter* h = &data->hunters[data->num_hunters++];
        strcpy(h->username, username);
        h->level = 1;
        h->exp = 0;
        h->atk = 10;
        h->hp = 100;
        h->def = 5;
        h->banned = 0;
        printf("Registration successful.\n");
    }
}
```

- Program meminta username dari user.
- Mengecek apakah username sudah ada di array data->hunters.
- Jika tidak ada, dan kapasitas belum penuh, maka hunter baru ditambahkan ke sistem dengan level dan stats default.

#### Login

```C
if (choice == 2) {
    printf("Enter username to login: ");
    scanf("%s", username);
    getchar();

    int found = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, username) == 0) {
            if (data->hunters[i].banned) {
                printf("This account has been banned.\n");
                found = 1;
                break;
            }
            found = 1;
            hunter_session(&data->hunters[i], data, d_data);
            break;
        }
    }

    if (!found) {
        printf("Hunter not found.\n");
    }
}

```

- Program menerima username dari user.
- Melakukan pencarian pada array data->hunters.
- Jika hunter ditemukan dan tidak dibanned, maka hunter_session() dipanggil untuk masuk ke sesi hunter.
- Jika tidak ditemukan, akan muncul pesan error.

#### Atribut awal 

```C
struct Hunter* h = &data->hunters[data->num_hunters++];
strcpy(h->username, username);
h->level = 1;
h->exp = 0;
h->atk = 10;
h->hp = 100;
h->def = 5;
h->banned = 0;
```

- `h->level = 1;`: Menetapkan level awal pemain ke level 1 saat pertama kali mendaftar.
- `h->exp = 0;`: Menetapkan pengalaman awal pemain (EXP) ke 0, karena pemain baru belum memiliki EXP.
- `h->atk = 10;`: Menetapkan stat serangan (attack) awal pemain ke 10.
- `h->hp = 100;`: Menetapkan stat kesehatan (hit points) awal pemain ke 100.
- `h->def = 5;`: Menetapkan stat pertahanan (defense) awal pemain ke 5.
- `h->banned = 0;` : Menetapkan status pemblokiran (banned) pemain ke 0 (tidak diblokir) saat registrasi

#### Output 

#### registrasi
![image](https://github.com/user-attachments/assets/25cdcde3-73cd-46d0-89d7-028cda21e073)

#### Login 
![image](https://github.com/user-attachments/assets/97566c1a-c632-45f2-b1fe-c81d8341fb1d)

#### Atribut awal 
![image](https://github.com/user-attachments/assets/536be240-b94e-4319-8538-7240bc6a1f06)

#### C. Menampilkan Informasi semua hunter

```C
void display_hunter_info(struct SystemData* data) {
    printf("\n=== HUNTER INFO ===\n");
    for (int i = 0; i < data->num_hunters; i++) {
        struct Hunter* h = &data->hunters[i];
        printf("Name: %s\tLevel: %d\tEXP: %d\tATK: %d\tHP: %d\tDEF: %d\tStatus: %s\n",
               h->username, h->level, h->exp, h->atk, h->hp, h->def,
               h->banned ? "BANNED" : "ACTIVE");
    }
}
```

- Fungsi ini bertugas untuk menampilkan informasi tentang hunter yang ada dalam sistem.
- Fungsi ini menerima parameter data, yang merupakan pointer ke struktur SystemData yang menyimpan informasi tentang hunter.
Di dalam fungsi ini, dilakukan iterasi pada array hunters yang ada dalam data. Setiap hunter akan ditampilkan dengan informasi:
- Name: Nama hunter.
- Level: Level hunter.
- EXP: EXP hunter.
- ATK: Attack (serangan) hunter.
- HP: Health points (kesehatan) hunter.
- DEF: Defense (pertahanan) hunter.
- Status: Status hunter, apakah BANNED atau ACTIVE (tergantung apakah field banned bernilai 1 atau 0).

#### Output 
![image](https://github.com/user-attachments/assets/7db9f122-6fcf-416c-a13f-85c044afa63a)


#### D. Membuat Dungeon (Generate Dungeon) dengan atribut acak sesuai dengan rentang yang ditentukan 

```C
const char *dungeon_names[] = {
    "Double Dungeon", "Demon Castle", "Pyramid Dungeon", "Red Gate Dungeon",
    "Hunters Guild Dungeon", "Busan A-Rank Dungeon", "Insects Dungeon",
    "Goblins Dungeon", "D-Rank Dungeon", "Gwanak Mountain Dungeon",
    "Hapjeong Subway Station Dungeon"
};
void generate_dungeon(struct DungeonData* d_data) {
    if (d_data->num_dungeons >= MAX_DUNGEONS) {
        printf("Dungeon limit reached.\n");
        return;
    }

    struct Dungeon* d = &d_data->dungeons[d_data->num_dungeons];

    strcpy(d->name, dungeon_names[rand() % (sizeof(dungeon_names) / sizeof(dungeon_names[0]))]);
    d->min_level = rand() % 5 + 1;
    d->atk_reward = rand() % 51 + 100;
    d->hp_reward = rand() % 51 + 50;
    d->def_reward = rand() % 26 + 25;
    d->exp_reward = rand() % 151 + 150;
    d->key = generate_key();

    d_data->num_dungeons++;

    printf("\nDungeon generated!\n");
    printf("Name: %s\n", d->name);
    printf("Minimum Level: %d\n", d->min_level);
    printf("ATK: %d\nHP: %d\nDEF: %d\nEXP: %d\nKEY: %lu\n",
           d->atk_reward, d->hp_reward, d->def_reward, d->exp_reward, d->key);
}
```
- `const char *dungeon_names[]` : Mendeklarasikan semua nama dungeon 
- Fungsi pertama-tama memeriksa apakah jumlah dungeon yang ada (d_data->num_dungeons) sudah mencapai batas maksimum (MAX_DUNGEONS).
- Jika sudah, fungsi akan mencetak pesan "Dungeon limit reached." dan keluar tanpa menambah dungeon baru.
- Menentukan dungeon baru: Jika jumlah dungeon belum mencapai batas, fungsi memilih slot dungeon yang kosong pada array dungeons menggunakan d_data->dungeons[d_data->num_dungeons].
- Menentukan nama dungeon secara acak: Nama dungeon dipilih secara acak dari array dungeon_names[] menggunakan rand() % (sizeof(dungeon_names) / sizeof(dungeon_names[0])) untuk memilih indeks secara acak.

- Menentukan atribut dungeon secara acak: Level minimal (min_level) untuk dungeon dipilih secara acak antara 1 hingga 5 (rand() % 5 + 1).

Hadiah untuk dungeon ditentukan dengan nilai acak:
- ATK reward: antara 100 hingga 150 (rand() % 51 + 100).
- HP reward: antara 50 hingga 100 (rand() % 51 + 50).
- DEF reward: antara 25 hingga 50 (rand() % 26 + 25).
- EXP reward: antara 150 hingga 300 (rand() % 151 + 150).

- Menghasilkan kunci unik untuk dungeon: Kunci dungeon `(key)` dihasilkan menggunakan fungsi `generate_key()`, yang menggabungkan waktu saat ini dan nilai acak untuk memastikan kunci unik.
- Update jumlah dungeon: Setelah dungeon baru dibuat, jumlah dungeon dalam `d_data->num_dungeons` diperbarui dengan menambah 1.
- Menampilkan informasi dungeon: Fungsi mencetak informasi tentang dungeon yang baru dibuat, termasuk nama dungeon, level minimum, hadiah (ATK, HP, DEF, EXP), dan kunci dungeon (key).

#### Output

![image](https://github.com/user-attachments/assets/ae03036e-3797-4600-bbdf-011d4fbac827)

#### E. Menampilkan seluruh informasi Dungeon

```C
void display_dungeon_info(struct DungeonData* d_data) {
    printf("\n=== DUNGEON INFO ===\n");
    if (d_data->num_dungeons == 0) {
        printf("No dungeons available.\n");
        return;
    }

    for (int i = 0; i < d_data->num_dungeons; i++) {
        struct Dungeon* d = &d_data->dungeons[i];
        printf("[Dungeon %d]\n", i + 1);
        printf("Name: %s\n", d->name);
        printf("Minimum Level: %d\n", d->min_level);
        printf("EXP Reward: %d\n", d->exp_reward);
        printf("ATK: %d\nHP: %d\nDEF: %d\nKEY: %lu\n\n",
               d->atk_reward, d->hp_reward, d->def_reward, d->key);
    }
}
```

- `printf("\n=== DUNGEON INFO ===\n");` Menampilkan judul untuk informasi dungeon.
- `if (d_data->num_dungeons == 0)` Memeriksa apakah jumlah dungeon dalam d_data adalah 0. Jika tidak ada dungeon yang tersedia:
- `printf("No dungeons available.\n");` Menampilkan pesan bahwa tidak ada dungeon yang tersedia.
- `return;` Menghentikan eksekusi fungsi jika tidak ada dungeon.
- `for (int i = 0; i < d_data->num_dungeons; i++)` Melakukan iterasi untuk setiap dungeon yang ada dalam d_data->dungeons[] sebanyak jumlah dungeon yang ada (d_data->num_dungeons).
- `struct Dungeon* d = &d_data->dungeons[i];` Menyimpan pointer ke dungeon yang sedang diproses dalam variabel d.
- `printf("[Dungeon %d]\n", i + 1);` Menampilkan nomor dungeon yang sedang diproses (dimulai dari 1, bukan 0).
- `printf("Name: %s\n", d->name);` Menampilkan nama dungeon (d->name).
- `printf("Minimum Level: %d\n", d->min_level);` Menampilkan level minimum yang dibutuhkan untuk memasuki dungeon (d->min_level).
- `printf("EXP Reward: %d\n", d->exp_reward);` Menampilkan jumlah EXP yang didapatkan dari dungeon (d->exp_reward).
- `printf("ATK: %d\nHP: %d\nDEF: %d\nKEY: %lu\n\n", d->atk_reward, d->hp_reward, d->def_reward, d->key);` Menampilkan hadiah dungeon berupa nilai ATK, HP, DEF, dan kunci unik dungeon (d->atk_reward, d->hp_reward, d->def_reward, d->key).

#### Output 
![image](https://github.com/user-attachments/assets/1ecb65c3-e8df-424b-a93e-13cd6f8b9446)


#### F. Hanya dapat menampilkan level minimum sesuai level hunter

```C
void view_available_dungeons(struct Dungeon* dungeons, int total, int level) {
    printf("\n=== AVAILABLE DUNGEONS ===\n");
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (strlen(dungeons[i].name) > 0 && dungeons[i].min_level <= level) {
            printf("%d. %s\t(Level %d+)\n", ++count, dungeons[i].name, dungeons[i].min_level);
        }
    }
    if (count == 0) {
        printf("No available dungeons for your level.\n");
    }
    printf("\nPress enter to continue...");
    getchar();
}

```

- `if (strlen(dungeons[i].name) > 0 && dungeons[i].min_level <= level)` Baris ini memastikan bahwa hanya dungeon yang memiliki name yang valid (tidak kosong) dan memiliki min_level yang lebih kecil atau sama dengan level hunter yang ditampilkan.
- `dungeons[i].min_level <=` level mengecek apakah dungeon tersebut dapat diakses oleh hunter berdasarkan level mereka.
- `printf("%d. %s\t(Level %d+)\n", ++count, dungeons[i].name, dungeons[i].min_level);`  Jika kondisi di atas terpenuhi, informasi dungeon akan ditampilkan dengan nomor urut, nama dungeon, dan level minimum yang diperlukan untuk memasuki dungeon tersebut.

#### Output 

![image](https://github.com/user-attachments/assets/a83a00e6-5d1a-47f1-be1d-cf0d50be5e01)


#### G. Jika exp sudah mencapai 500 , maka naik level dan exp kembali lagi dari 0 

```C
if (hunter->exp >= 500) {
    hunter->level++;
    hunter->exp = 0;
}
```

- Kode ini dijalankan setelah hunter berhasil melakukan raid dungeon dan mendapatkan reward EXP dari dungeon.
- `hunter->exp` adalah jumlah EXP hunter setelah ditambahkan reward dari dungeon yang dipilih.

Jika EXP hunter sekarang sudah >= 500, maka:
- `hunter->level++` akan menaikkan level hunter sebanyak 1.
- `hunter->exp = 0` akan mengatur ulang EXP menjadi 0 setelah naik level.


#### Output 
![image](https://github.com/user-attachments/assets/51324ee4-d7d0-4d24-8ed0-528c5523c0a8)

#### H. Mode battle dengan Player Lain PVP dan merampas stats pemain lawan jika kita menang

```C
void battle_hunter(struct Hunter* self, struct SystemData* data) {
    printf("\n=== PVP LIST ===\n");
    for (int i = 0; i < data->num_hunters; i++) {
        struct Hunter* h = &data->hunters[i];
        if (strcmp(h->username, self->username) != 0 && !h->banned) {
            printf("%s - Power: %d\n", h->username, h->atk + h->hp + h->def);
        }
    }

    char target_name[50];
    printf("Target: ");
    scanf("%s", target_name);
    getchar();

    struct Hunter* target = NULL;
    int index = -1;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, target_name) == 0) {
            target = &data->hunters[i];
            index = i;
            break;
        }
    }

    if (!target || target->banned || strcmp(self->username, target->username) == 0) {
        printf("Invalid target.\n");
        return;
    }

    int power_self = self->atk + self->hp + self->def;
    int power_target = target->atk + target->hp + target->def;

    if (power_self >= power_target) {
        // Pemenang menyerap stats musuh
        self->atk += target->atk;
        self->hp += target->hp;
        self->def += target->def;
        for (int i = index; i < data->num_hunters - 1; i++) {
            data->hunters[i] = data->hunters[i + 1];
        }
        data->num_hunters--;
        printf("You won and absorbed %s's stats!\n", target->username);
    } else {
        // Kalah: musuh menyerap stats kita, kita dihapus
        target->atk += self->atk;
        target->hp += self->hp;
        target->def += self->def;
        for (int i = 0; i < data->num_hunters; i++) {
            if (strcmp(data->hunters[i].username, self->username) == 0) {
                for (int j = i; j < data->num_hunters - 1; j++) {
                    data->hunters[j] = data->hunters[j + 1];
                }
                data->num_hunters--;
                break;
            }
        }
        printf("You lost. You are removed from the system.\n");
        exit(0);
    }

    printf("\nPress enter to continue...");
    getchar();
}

```

- Loop semua hunter kecuali diri sendiri dan yang tidak di-ban, lalu tampilkan power total mereka (atk + hp + def).
- User diminta memasukkan nama hunter yang ingin ditantang.
- Memastikan target valid, bukan diri sendiri, dan tidak di-ban.

Perhitungan Power dan Hasil Pertarungan:
- Jika power kita ≥ power lawan: Kita menang → stats lawan diserap dan Lawan dihapus dari sistem (hunter list di-shift).
- Jika power kita < power lawan: Kita kalah → stats kita diserap lawan dan Kita dihapus dari sistem dan program keluar (exit(0)).


#### Output Battle 
![image](https://github.com/user-attachments/assets/321a7c6a-0f25-4c36-b06c-6bc11a12d493)

#### Output stats yang dirampas 
![image](https://github.com/user-attachments/assets/a3e9df9c-a33c-4e97-bb28-889e17a2187c)

#### I. Ban dan Unban ( Tidak bisa masuk ke menu System Hunter ) 

#### Ban Hunter
```C
void ban_hunter(struct SystemData* data) {
    char name[50];
    printf("Enter hunter name to ban: ");
    scanf("%s", name);
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = 1;
            printf("Hunter '%s' has been banned.\n", name);
            return;
        }
    }
    printf("Hunter not found.\n");
}
```

- Program meminta user untuk memasukkan nama hunter.
- Dilakukan pencarian pada `array data->hunters` menggunakan `strcmp`.
- Jika ditemukan, nilai `banned` pada hunter tersebut di-set ke 1, menandakan hunter tersebut dibanned.
- Jika tidak ditemukan, akan ditampilkan pesan "Hunter not found.".

#### Unban Hunter

```C
void unban_hunter(struct SystemData* data) {
    char name[50];
    printf("Enter hunter name to unban: ");
    scanf("%s", name);
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = 0;
            printf("Hunter '%s' has been unbanned.\n", name);
            return;
        }
    }
    printf("Hunter not found.\n");
}
```

- Mirip dengan ban_hunter, tetapi justru mengubah banned menjadi 0 (aktif kembali).
- Berguna untuk membatalkan status banned pada hunter yang sebelumnya diblokir.

#### Output Ban Hunter
![image](https://github.com/user-attachments/assets/e40740cb-1ce8-410b-b0ea-82a88fc0f4a0)


#### Pemain tidak bisa login ke menu hunter 
![image](https://github.com/user-attachments/assets/e1234c0f-bcc7-4360-8e21-f761b2e324da)


#### Output Unban Hunter
![image](https://github.com/user-attachments/assets/4adb06f1-d705-4c99-90eb-e70611f5522a)

#### J. Membuat menu reset untuk hunter bertobat ( Mengembalikan ke level 1 dan stats awal ) 

```C
void reset_hunter(struct SystemData* data) {
    char name[50];
    printf("Enter hunter name to reset: ");
    scanf("%s", name);
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].level = 1;
            data->hunters[i].exp = 0;
            data->hunters[i].atk = 10;
            data->hunters[i].hp = 100;
            data->hunters[i].def = 5;
            data->hunters[i].banned = 0;
            printf("Hunter '%s' has been reset.\n", name);
            return;
        }
    }
    printf("Hunter not found.\n");
}
```

- `scanf("%s", name);` Meminta pengguna memasukkan nama hunter yang ingin di-reset.
- `for (int i = 0; i < data->num_hunters; i++) {
    if (strcmp(data->hunters[i].username, name) == 0) {`
    Melakukan iterasi seluruh data hunter dan Membandingkan username hunter satu per satu dengan input name.
-
`data->hunters[i].level = 1;
data->hunters[i].exp = 0;
data->hunters[i].atk = 10;
data->hunters[i].hp = 100;
data->hunters[i].def = 5;
data->hunters[i].banned = 0;` 

Mengatur kembali status hunter ke kondisi awal:
level = 1
exp = 0
atk = 10
hp = 100
def = 5
banned = 0 (hunter otomatis diaktifkan kembali jika sebelumnya dibanned)

#### Output 
![image](https://github.com/user-attachments/assets/f5ae418a-354a-4ff3-a59d-6cd46c8e1196)


#### K. Fitur Notifikasi dungeon yang berubah setiap 3 detik 

```C
int running_notification = 0;
pthread_t notification_thread;
struct DungeonData* global_d_data = NULL;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void* notification_loop(void* arg) {
    int index = 0;
    while (running_notification) {
        pthread_mutex_lock(&print_lock);
        printf("\033[s"); // Simpan posisi kursor
        printf("\033[2;1H%-60s", " "); // Bersihkan baris 2
        if (global_d_data == NULL || global_d_data->num_dungeons == 0) {
            printf("\033[2;1HNo active dungeons.");
        } else {
            struct Dungeon* d = &global_d_data->dungeons[index % global_d_data->num_dungeons];
            printf("\033[2;1H%s (Min. Lv %d)", d->name, d->min_level);
            index++;
        }
        printf("\033[u"); // Kembalikan posisi kursor
        fflush(stdout);
        pthread_mutex_unlock(&print_lock);
        sleep(3); // Ganti notifikasi setiap 3 detik
    }
    return NULL;
}
void start_notification(struct DungeonData* d_data) {
    if (!running_notification) {
        running_notification = 1;
        global_d_data = d_data;
        pthread_create(&notification_thread, NULL, notification_loop, NULL);
    }
}

```

- `running_notification` → Menandai apakah thread notifikasi sedang berjalan.
- `notification_thread` → ID thread yang dijalankan untuk notifikasi.
- `global_d_data` → Referensi ke data dungeon dari shared memory.
- `print_lock` → Menghindari konflik cetak antara notifikasi dan menu utama.

- Loop akan terus berjalan selama running_notification aktif.
- Menggunakan `ANSI` escape `(\033[2;1H)` untuk menulis di baris ke-2 kolom pertama.
- Setiap 3 detik, dungeon berikutnya akan ditampilkan (berdasarkan indeks).
- Menggunakan `mutex` agar tidak tumpang tindih dengan tampilan menu utama.

- Mengecek apakah notifikasi sudah aktif. Jika belum:
- `Set running_notification = 1`
- Simpan referensi dungeon data `(global_d_data)`
- Buat thread baru yang menjalankan `notification_loop`

#### Output 
![image](https://github.com/user-attachments/assets/8b27168d-d78d-46a9-a03a-2483580ea580)

#### L. Clean Shared Memory

```C
void cleanup(int signal) {
    printf("\nCleaning up shared memory...\n");
    shmdt(data);
    shmdt(d_data);
    shmctl(shmid_hunter, IPC_RMID, NULL);
    shmctl(shmid_dungeon, IPC_RMID, NULL);
    exit(0);
}
```

- `void cleanup(int signal)` Mendefinisikan fungsi cleanup yang menerima satu argumen bertipe int (nomor sinyal).
- `printf("\nCleaning up shared memory...\n");` Menampilkan pesan ke terminal bahwa sistem sedang membersihkan shared memory.
- `shmdt(data);` Detach (melepaskan) pointer data dari segmen shared memory hunter (SystemData), agar tidak lagi diakses oleh proses ini.
- `shmdt(d_data);` Detach pointer d_data dari segmen shared memory dungeon (DungeonData).
- `shmctl(shmid_hunter, IPC_RMID, NULL);` Menghapus segmen shared memory hunter dari sistem menggunakan IPC_RMID (hapus permanent).
- `shmctl(shmid_dungeon, IPC_RMID, NULL);` Menghapus segmen shared memory dungeon dari sistem.
- `exit(0);`  Mengakhiri proses program dengan status sukses (kode keluar 0).

#### Output sebelum diexit
![image](https://github.com/user-attachments/assets/fe42a3a8-473b-4b1d-980f-6b1092ca90d8)

#### Setelah diexit , program akan otomatis menghapus shared memory 
![image](https://github.com/user-attachments/assets/74a06c8f-6600-43ba-9907-850c407094ce)

![image](https://github.com/user-attachments/assets/7579bd28-7c4e-4548-a471-05c7ed0c664c)



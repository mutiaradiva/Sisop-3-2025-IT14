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

📖 Penjelasan Soal Nomor 1 (a–g)
🅰️ a. Struktur Direktori
File teks rahasia yang harus dikonversi berada dalam file .zip yang dapat diekstrak secara manual. Setelah proses unzip dan kompilasi, struktur direktori proyek menjadi:

css
Salin
Edit
.
├── image_client.c
├── image_server.c
├── client
│   ├── image_client
│   ├── secrets
│   │   ├── input_1.txt
│   │   ├── input_2.txt
│   │   └── ...
│   └── [hasil_unduhan].jpeg
└── server
    ├── image_server
    ├── server.log
    └── database
        └── [hasil_dekripsi].jpeg
🅱️ b. image_server.c sebagai Daemon
Program image_server.c berjalan secara daemon di background menggunakan fork() dan setsid(). Program ini:

Menunggu koneksi client di port 8080.

Tidak menampilkan output terminal.

Siap menerima perintah RPC untuk pemrosesan file.

🅲 c. Fungsi image_client.c
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

📌 File tidak pernah dicopy atau dipindah — semua data dikirim melalui socket RPC.

🅳 d. Menu Interaktif image_client.c
Klien menyediakan menu interaktif seperti:

markdown
Salin
Edit
==============================
| Klien Pengubah Gambar      |
==============================
1. Kirim file ke server
2. Unduh file dari server
3. Keluar
>> 
Pengguna dapat memasukkan perintah berkali-kali tanpa harus menjalankan ulang program.

🅴 e. Output JPEG yang Valid
Setelah klien mengirim file teks, server akan menyimpan hasil dekripsi ke:

pgsql
Salin
Edit
server/database/1744401282.jpeg
Klien dapat mengunduh file ini dan membuka hasilnya sebagai file gambar JPEG yang valid.

🅵 f. Penanganan Error
Dari Klien:

Gagal koneksi ke server. → jika server tidak aktif.

Nama file teks tidak ditemukan. → jika file input salah/tidak ada.

Dari Server:

File JPEG tidak ditemukan. → dikirim kembali ke klien jika file tidak tersedia.

Server tidak pernah crash, dan selalu mengirim pesan respons atau mencatat error ke log.

🅶 g. server.log: Format Pencatatan Aktivitas
Semua interaksi client-server dicatat ke:

pgsql
Salin
Edit
server/server.log
Dengan format:

less
Salin
Edit
[Client][YYYY-MM-DD hh:mm:ss]: [DECRYPT] [Text data]
[Server][YYYY-MM-DD hh:mm:ss]: [SAVE] [1744401282.jpeg]
[Client][YYYY-MM-DD hh:mm:ss]: [DOWNLOAD] [1744401282.jpeg]
[Server][YYYY-MM-DD hh:mm:ss]: [SEND] [1744401282.jpeg]
Log mencakup semua:

Koneksi,

Upload,

Download,

Error,

Exit.



# soal-2
**Dikerjakan oleh  M. Faqih Ridho (5027241123)**

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
### Error handling di Weapon Shop
```
if(choice > 0 && choice < shop_size) {
        buy_weapon(sock, choice, &ps->gold, ps->inventory, &ps->inventory_count);
    } else if(choice != 0) {
        send_str(sock, RED "Invalid choice!\n" RESET);
    }
```
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
### Error Handling di Battle Mode
```
        else {
            send_str(sock, RED "\nInvalid command! Use 'attack' or 'exit'.\n" RESET);
        }
```
### Error Handling di Main Menu
```
if(choice < 1 || choice > 5) {
            send_str(client_sock, RED "\nInvalid choice!\n" RESET);
            continue;
        }
```

# soal-4
**Dikerjakan oleh Muhammad Fatihul Qolbi Ash Shiddiqi (5027241023)**

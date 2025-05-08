# Soal 3
a. Membuat dungeon.c sebagai server untuk client.c yang terhubung melalui RPC.

dungeon.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

#define PORT 12345

void* handle_client(void* socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int read_size = recv(sock, buffer, sizeof(buffer), 0);
        if (read_size <= 0) break;

        printf("Received command: %s\n", buffer);

        char* response = "Dungeon accepted your command.\n";
        send(sock, response, strlen(response), 0);
    }

    close(sock);
    free(socket_desc);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    listen(server_fd, 5);

    printf("Dungeon server listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr*)&client, &client_len);
        printf("New player connected.\n");

        pthread_t thread_id;
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        pthread_create(&thread_id, NULL, handle_client, (void*)new_sock);
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}

```
Penjelasan kode:

Fungsi main pada kode dungeon.c memulai server yang menerima koneksi dari client menggunakan socket TCP. Pertama, server membuka socket dan mengonfigurasi alamat serta port untuk memulai koneksi. Setelah itu, server masuk ke dalam loop utama di mana ia menerima koneksi baru dari client menggunakan `accept()`. Setiap koneksi client ditangani secara terpisah dengan membuat thread baru menggunakan `pthread_create`, yang menjalankan fungsi `handle_client` untuk berkomunikasi dengan client tersebut. Setelah selesai menangani client, thread akan dibersihkan dengan `pthread_detach`, dan server tetap mendengarkan koneksi baru. Proses ini memungkinkan server untuk melayani banyak client secara bersamaan.

player.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[1024], input[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));
    printf("Connected to dungeon server.\n");

    while (1) {
        printf("Enter command: ");
        fgets(input, sizeof(input), stdin);
        send(sock, input, strlen(input), 0);

        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("Dungeon: %s\n", buffer);
    }

    close(sock);
    return 0;
}
```
Penjelasan kode:

Fungsi main di dalam kode player.c ini bertugas untuk menghubungkan client ke server dungeon menggunakan socket TCP. Pertama, program membuat socket dan mengonfigurasi alamat serta port server yang akan dihubungkan. Di sini, alamat server adalah `127.0.0.1` yang merujuk pada localhost (server yang berjalan di mesin yang sama). Setelah socket berhasil terhubung ke server menggunakan `connect()`, client siap untuk berkomunikasi. Dalam loop utama, client meminta input dari pengguna (command), mengirimnya ke server menggunakan `send()`, lalu menunggu respons dari server dengan `recv()`. Setelah menerima respons, client mencetak pesan dari server di layar. Proses ini terus berulang selama koneksi tetap aktif, dan setelah selesai, socket ditutup dengan `close()`.

![1](https://github.com/user-attachments/assets/c941160f-632c-4a43-9680-90df75c3bf53)
![1_1](https://github.com/user-attachments/assets/a0283393-0f4b-46e4-9801-07a41a3c6e7a)

b. Player terhubung dengan dungeon dan menampilkan menu
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345

void show_menu() {
    printf("\n===== Main Menu =====\n");
    printf("1. Show Player Stats\n");
    printf("2. Shop (Buy Weapons)\n");
    printf("3. View Inventory & Equip Weapons\n");
    printf("4. Battle Mode\n");
    printf("5. Exit game\n");
    printf("Choose an option: ");
}

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[1024], input[10];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to dungeon server.\n");

    int running = 1;
    while (running) {
        show_menu();
        fgets(input, sizeof(input), stdin);

        switch (input[0]) {
            case '1':
                printf("[Player Stats]\nHP: 100\nAttack: 10\nDefense: 5\n");
                break;
            case '2':
                printf("[Shop] You entered the weapon shop (not implemented yet).\n");
                break;
            case '3':
                printf("[Inventory] You have a rusty sword equipped.\n");
                break;
            case '4':
                printf("[Battle Mode] Searching for enemy...\n");
                send(sock, "battle", strlen("battle"), 0);
                memset(buffer, 0, sizeof(buffer));
                recv(sock, buffer, sizeof(buffer), 0);
                printf("Dungeon: %s\n", buffer);
                break;
            case '5':
                printf("Exiting game...\n");
                running = 0;
                break;
            default:
                printf("Invalid option. Please choose 1-5.\n");
        }
    }

    close(sock);
    return 0;
}
```
Penjelasan kode:

Setelah berhasil terhubung ke server, program memasuki loop utama yang menampilkan menu dengan pilihan-pilihan yang dapat dipilih oleh pemain. Pemain dapat memilih opsi dengan memasukkan angka yang akan mengirimkan perintah ke server untuk diproses dengan menggunakan switch-case. Respons dari server akan diterima dan ditampilkan kepada pemain. Program terus berjalan dalam loop ini hingga pemain memilih untuk keluar, yang kemudian menutup koneksi socket dan menghentikan program.

![2](https://github.com/user-attachments/assets/169ffa50-3b22-4e05-b38f-80a88cc3c098)

c. Menambahkan opsi untuk melihat statistik player

Update player.c dengan struct Player
```c
typedef struct {
    int money;
    char equipped_weapon[50];
    int base_damage;
    int enemies_defeated;
} Player;

Player player = {
    .money = 100,
    .equipped_weapon = "Rusty Sword",
    .base_damage = 10,
    .enemies_defeated = 0
};
```
Perbarui switch case menu opsi 1
```c
case '1':
    printf("\n[Player Stats]\n");
    printf("Uang             : %d gold\n", player.money);
    printf("Senjata dipakai  : %s\n", player.equipped_weapon);
    printf("Base Damage      : %d\n", player.base_damage);
    printf("Musuh dikalahkan : %d\n", player.enemies_defeated);
    break;
```

![3](https://github.com/user-attachments/assets/5bbaa938-e9b1-435d-b85a-bbfd2f80cfc8)

d. Membuat shop.c dan menambahkan opsi kedua (shop)

shop.c
```c
#include <stdio.h>
#include <string.h>

typedef struct {
    char name[50];
    int price;
    int damage;
    char passive[100];
} Weapon;

Weapon weapon_list[] = {
    {"Rusty Sword", 0, 10, "No passive"},
    {"Iron Sword", 50, 15, "No passive"},
    {"Fire Dagger", 80, 12, "Burn: Chance to burn enemy"},
    {"Thunder Axe", 120, 18, "Shock: Chance to stun enemy"},
    {"Holy Blade", 150, 20, "Heal: Small chance to heal player"}
};

int weapon_count = sizeof(weapon_list) / sizeof(weapon_list[0]);

void list_weapons(char* out) {
    strcpy(out, "\n=== Weapon Shop ===\n");
    char line[200];
    for (int i = 0; i < weapon_count; i++) {
        sprintf(line, "%d. %s - %d gold - Damage: %d - Passive: %s\n",
                i + 1,
                weapon_list[i].name,
                weapon_list[i].price,
                weapon_list[i].damage,
                weapon_list[i].passive);
        strcat(out, line);
    }
    strcat(out, "Choose a weapon number to buy:\n");
}

int buy_weapon(int id, int* damage, char* name_out, char* passive_out) {
    if (id < 1 || id > weapon_count) return -1;

    Weapon w = weapon_list[id - 1];
    *damage = w.damage;
    strcpy(name_out, w.name);
    strcpy(passive_out, w.passive);

    return w.price;
}
```
Penjelasan kode:

Pada kode shop.c, terdapat struktur data Weapon yang menyimpan informasi tentang senjata, meliputi nama, harga, damage, dan kemampuan pasif. Array `weapon_list[]` berisi berbagai senjata dengan atribut yang berbeda. Fungsi `list_weapons()` digunakan untuk menampilkan daftar senjata yang tersedia di toko beserta harga, damage, dan kemampuan pasifnya, yang kemudian dikirimkan ke client. Sementara itu, fungsi `buy_weapon()` memungkinkan pemain untuk membeli senjata dengan ID yang diberikan. Fungsi ini mengecek validitas ID senjata, dan jika valid, mengembalikan informasi senjata seperti damage, nama, dan pasif serta harga senjata. Jika ID tidak valid, fungsi akan mengembalikan nilai -1. Mekanisme ini memungkinkan pemain untuk melihat dan membeli senjata dari toko dalam permainan.

tambahkan `#include "shop.c"` dan handle_client di dungeon.c
```c
if (strncmp(buffer, "shop", 4) == 0) {
    char response[1024];
    list_weapons(response);
    send(sock, response, strlen(response), 0);
}
else if (strncmp(buffer, "buy", 3) == 0) {
    int id, uang;
    sscanf(buffer + 4, "%d %d", &id, &uang);

    int dmg;
    char wname[50], passive[100];
    int price = buy_weapon(id, &dmg, wname, passive);

    char response[256];
    if (price == -1) {
        sprintf(response, "Invalid weapon ID.");
    } else if (uang < price) {
        sprintf(response, "Not enough gold. Price: %d", price);
    } else {
        sprintf(response, "Bought %s! New damage: %d. Passive: %s\n", wname, dmg, passive);
    }
    send(sock, response, strlen(response), 0);
}
```
Penjelasan kode:

Ketika perintah "shop" diterima, server mengirimkan daftar senjata yang tersedia menggunakan fungsi `list_weapons()`. Jika perintah "buy" diterima dengan ID senjata dan jumlah uang, server memproses pembelian dengan mengonversi input menggunakan `sscanf()` untuk mendapatkan ID dan uang yang tersedia, kemudian memanggil fungsi `buy_weapon()` untuk memeriksa validitas ID dan mengembalikan informasi tentang senjata, termasuk harga, damage, dan kemampuan pasif. Jika ID senjata tidak valid, server mengirimkan pesan "Invalid weapon ID", jika uang tidak cukup, server memberikan pesan "Not enough gold", dan jika pembelian berhasil, server mengonfirmasi pembelian dengan menyertakan informasi tentang senjata yang dibeli.

Update player.c untuk switch case opsi 2
```c
case '2': {
    send(sock, "shop", strlen("shop"), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    printf("Buy which weapon (number)? ");
    char choice[10];
    fgets(choice, sizeof(choice), stdin);
    int buy_id = atoi(choice);

    char buy_cmd[50];
    sprintf(buy_cmd, "buy %d %d", buy_id, player.money);
    send(sock, buy_cmd, strlen(buy_cmd), 0);

    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    if (strstr(buffer, "Bought")) {
        char* dmg_str = strstr(buffer, "New damage: ");
        if (dmg_str) {
            int dmg;
            sscanf(dmg_str, "New damage: %d", &dmg);
            player.base_damage = dmg;
        }

        char* start = strstr(buffer, "Bought ");
        if (start) {
            char wname[50];
            sscanf(start, "Bought %[^\n!]", wname);
            strcpy(player.equipped_weapon, wname);
        }
        player.money -= 150;
    }

    break;
}
```
Penjelasan kode:

Saat pemain memilih opsi '2' untuk membuka toko, program mengirimkan perintah "shop" ke server untuk meminta daftar senjata yang tersedia, kemudian menerima dan menampilkan daftar tersebut ke pemain. Setelah itu, pemain diminta untuk memilih senjata yang ingin dibeli dengan memasukkan nomor pilihan. Pilihan ini dikirimkan ke server bersama dengan jumlah uang yang dimiliki pemain dalam format perintah "buy". Server kemudian memproses pembelian dan mengirimkan respons yang berisi informasi tentang pembelian senjata, termasuk nama senjata dan damage baru. Jika pembelian berhasil, kode ini mengekstrak informasi tentang damage baru dan nama senjata dari respons, memperbarui atribut `base_damage` dan `equipped_weapon` pemain, serta mengurangi uang pemain. 

![4](https://github.com/user-attachments/assets/f53bad61-cc4f-4d31-ad04-95d23f30e5fb)

e. Membuat inventory untuk player
Tambahkan struct untuk weapon dan inventory di player.c
```c
#define MAX_INVENTORY 10

typedef struct {
    char name[50];
    int damage;
    char passive[100];
} Weapon;

Weapon inventory[MAX_INVENTORY];
int inventory_count = 0;
int equipped_index = -1;
```
Update opsi pembelian senjata untuk menambahkan ke inventory
```c
if (inventory_count < MAX_INVENTORY) {
    strcpy(inventory[inventory_count].name, player.equipped_weapon);
    inventory[inventory_count].damage = player.base_damage;
    strcpy(inventory[inventory_count].passive, passive);
    equipped_index = inventory_count;
    inventory_count++;
}
```
Pada case 3 tambahkan untuk melihat inventory dan menggunakan senjata
```c
case '3': {
    printf("=== YOUR INVENTORY ===\n");
    for (int i = 0; i < inventory_count; i++) {
        printf("[%d] %s - Damage: %d - Passive: %s%s\n",
               i,
               inventory[i].name,
               inventory[i].damage,
               strlen(inventory[i].passive) > 0 ? inventory[i].passive : "None",
               (i == equipped_index) ? " (Equipped)" : "");
    }

    printf("Equip which weapon? (index, or -1 to cancel): ");
    char choice[10];
    fgets(choice, sizeof(choice), stdin);
    int idx = atoi(choice);
    if (idx >= 0 && idx < inventory_count) {
        equipped_index = idx;
        strcpy(player.equipped_weapon, inventory[idx].name);
        player.base_damage = inventory[idx].damage;
        printf("Equipped %s!\n", player.equipped_weapon);
    } else {
        printf("Cancelled.\n");
    }
    break;
}
```

![5](https://github.com/user-attachments/assets/022371b0-9224-4a02-902a-bde6a0d7cf41)

f. Membuat program untuk opsi battle mode

Tambahkan struct musuh pada dungeon.c
```c
typedef struct {
    int hp;
    int max_hp;
    int reward;
    int in_battle;
} Enemy;
```
Di dalam `handle_client`, tambahkan variabel sebelum perulangan while
```c
Enemy enemy = {.in_battle = 0}; // status musuh untuk client ini
```
Update `handle_client` untuk parsing battle command
```c
else if (strncmp(buffer, "battle start", 12) == 0) {
    enemy.max_hp = 100 + rand() % 151;
    enemy.hp = enemy.max_hp;
    enemy.reward = 100 + rand() % 101;
    enemy.in_battle = 1;

    char res[128];
    sprintf(res, "A wild enemy appears! HP: %d\n", enemy.hp);
    send(sock, res, strlen(res), 0);
}
else if (strncmp(buffer, "battle attack", 13) == 0 && enemy.in_battle) {
    int dmg = player_dmg;
    enemy.hp -= dmg;

    char res[256];
    if (enemy.hp <= 0) {
        enemy.in_battle = 0;
        sprintf(res,
                "You defeated the enemy! You gained %d gold.\n",
                enemy.reward);
    } else {
        sprintf(res,
                "You dealt %d damage! Enemy HP: %d/%d\n",
                dmg,
                enemy.hp,
                enemy.max_hp);
    }
    send(sock, res, strlen(res), 0);
}
else if (strncmp(buffer, "battle exit", 11) == 0) {
    enemy.in_battle = 0;
    send(sock, "You fled the battle.\n", 22, 0);
}
```
Update menu battle di player.c
```c
case '4':
    send(sock, "battle start", strlen("battle start"), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    while (1) {
        printf("Enter command (attack/exit): ");
        char cmd[20];
        fgets(cmd, sizeof(cmd), stdin);

        if (strncmp(cmd, "attack", 6) == 0) {
            char attack_cmd[64];
            sprintf(attack_cmd, "battle attack %d", player.base_damage);
            send(sock, attack_cmd, strlen(attack_cmd), 0);
        } else if (strncmp(cmd, "exit", 4) == 0) {
            send(sock, "battle exit", strlen("battle exit"), 0);
        } else {
            printf("Unknown command.\n");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);

        if (strstr(buffer, "defeated") || strstr(buffer, "fled")) {
            break;
        }
    }
    break;
```
Penjelasan kode:
perintah "battle start" dikirim ke server untuk memulai pertempuran. Server lalu mengirimkan informasi musuh yang muncul, yang kemudian ditampilkan ke pemain. Setelah itu, program masuk ke loop yang meminta pemain memasukkan perintah selama pertempuran, yaitu "attack" untuk menyerang atau "exit" untuk kabur. Jika pemain memilih "attack", program mengirimkan perintah "battle attack" beserta damage dasar pemain ke server. Jika pemain memilih "exit", maka dikirimkan perintah "battle exit" untuk mengakhiri pertarungan. Setelah setiap aksi, program menerima dan menampilkan respons dari server. Loop pertempuran akan berhenti jika dalam respons ditemukan kata "defeated" (musuh kalah) atau "fled" (pemain kabur), yang menandakan akhir dari sesi pertarungan.

![6](https://github.com/user-attachments/assets/0b7e23e0-791d-42e2-99b6-f6d44be7ceb0)
![6_1](https://github.com/user-attachments/assets/19ecd867-e9d7-448b-b0ce-39be433ffca4)

**Revisi**
Setelah musuh dikalahkan seharusnya musuh baru akan muncul tetapi di dalam kode belum terimplementasi jadi saya meng-update kode agar musuh baru muncul.

![revisi](https://github.com/user-attachments/assets/e84048fe-c199-43f0-81f6-be86a055fe78)

g. Update kode pada dungeon.c untuk menambahkan battle logic
```c
else if (strncmp(buffer, "battle attack", 13) == 0 && enemy.in_battle) {
    int base_dmg = 0;
    char passive[100] = "";
    int chance = rand() % 101;
    char extra_info[256] = "";

    // Parsing format: "battle attack <base_dmg>|<passive>"
    char* dmg_str = strtok(buffer + 14, "|");
    char* passive_str = strtok(NULL, "\n");

    if (dmg_str) base_dmg = atoi(dmg_str);
    if (passive_str) strncpy(passive, passive_str, sizeof(passive) - 1);

    // === Damage Variation (misalnya ±20%) ===
    int variation = base_dmg * (rand() % 21 - 10) / 100; // -10% to +10%
    int final_dmg = base_dmg + variation;

    // === Critical Hit Chance ===
    int crit_chance = rand() % 100;
    int is_crit = 0;
    if (crit_chance < 25) { // 25% chance
        final_dmg *= 2;
        is_crit = 1;
        strcat(extra_info, "💥 Critical Hit!\n");
    }

    // === Passive Effects ===
    if (strcmp(passive, "(Burn: 60% chance to burn enemy)") == 0) {
        if (chance < 60) {
            final_dmg += 2;
            strcat(extra_info, "🔥 Burn damage activated!\n");
        }
    }
    // (Bisa tambah passive lainnya di sini)

    // Apply damage
    enemy.hp -= final_dmg;
    if (enemy.hp < 0) enemy.hp = 0;

    // Response to player
    char res[512];
    if (enemy.hp <= 0) {
        enemy.in_battle = 0;
        sprintf(res, "%s\nYou dealt %d damage! Enemy defeated!\nYou gained %d gold.\n",
                extra_info, final_dmg, enemy.reward);
    } else {
        sprintf(res, "%s\nYou dealt %d damage! Enemy HP: %d/%d\n",
                extra_info, final_dmg, enemy.hp, enemy.max_hp);
    }

    send(sock, res, strlen(res), 0);
}
```
Penjelasan kode:

Memisahkan nilai damage dasar (base_dmg) dan efek pasif dari senjata, jika ada, dari perintah yang dikirim klien. Damage akan mengalami variasi acak sebesar ±10%, lalu dihitung peluang serangan kritikal (25% kemungkinan) yang menggandakan damage dan menambahkan pesan efek kritikal. Jika senjata memiliki efek pasif seperti Burn, server memeriksa peluang efek tersebut terjadi (contohnya 60% untuk Burn), menambahkan damage ekstra jika berhasil, serta menyisipkan info tambahan. Setelah total damage dihitung, HP musuh dikurangi, dan jika HP-nya habis, musuh dianggap kalah dan pemain diberi hadiah gold. Server lalu merespons kembali ke pemain dengan laporan damage, efek spesial yang terjadi, dan status HP musuh, atau jika musuh telah dikalahkan.

![Capture7](https://github.com/user-attachments/assets/5255210d-a109-445d-ad92-7a740bc771ef)
![Capture7_1](https://github.com/user-attachments/assets/64d0aca0-68fa-4e23-bc5e-710e404b2581)

h. Error handling

Menambahkan `default` pada setiap switch case dan `else` untuk setiap if-statement.

![Capture8](https://github.com/user-attachments/assets/f87e51c5-12e0-40b6-8af8-0d0f72a5869f)

# Soal 4
a. Membuat shared memory utama untuk system.c yang mengelola shared memory hunter.c

system.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define SHM_KEY 1234
#define MAX_HUNTERS 10

typedef struct {
    int id;
    char status[50];
} Hunter;

int main() {
    int shmid = shmget(SHM_KEY, sizeof(Hunter) * MAX_HUNTERS, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    Hunter *hunters = (Hunter *)shmat(shmid, NULL, 0);
    if (hunters == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Inisialisasi hunter
    for (int i = 0; i < MAX_HUNTERS; i++) {
        hunters[i].id = i;
        strcpy(hunters[i].status, "inactive");
    }

    printf("System aktif. Shared memory siap digunakan.\n");
    printf("Tekan CTRL+C untuk keluar dan menghapus shared memory.\n");

    while (1) {
        sleep(5);
    }

    return 0;
}
```
Penjelasan kode:

Kode pada system.c bertujuan untuk menginisialisasi shared memory yang menyimpan data 10 hunter. Program dimulai dengan mendefinisikan konstanta `SHM_KEY` sebagai kunci unik shared memory dan `MAX_HUNTERS` sebagai jumlah maksimum hunter yang akan disimpan. Struktur Hunter berisi dua field: id sebagai identitas unik dan status sebagai status hunter. Kemudian, program membuat shared memory menggunakan shmget dan melampirkannya ke alamat proses dengan `shmat`. Jika berhasil, program menginisialisasi setiap elemen dalam shared memory dengan id dari 0 sampai 9 dan status "inactive". Setelah itu, program mencetak informasi bahwa sistem aktif dan akan terus berjalan dalam loop `while(1)` agar shared memory tetap tersedia sampai dihentikan secara manual dengan CTRL+C. Program ini menjadi pondasi utama agar proses lain, seperti hunter client, dapat mengakses dan memodifikasi data hunter yang tersimpan di shared memory.

File: hunter.c – Mengakses dan mengubah data di shared memory
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define SHM_KEY 1234
#define MAX_HUNTERS 10

typedef struct {
    int id;
    char status[50];
} Hunter;

int main() {
    int shmid = shmget(SHM_KEY, sizeof(Hunter) * MAX_HUNTERS, 0666);
    if (shmid == -1) {
        perror("shmget failed (pastikan system.c sudah jalan)");
        exit(1);
    }

    Hunter *hunters = (Hunter *)shmat(shmid, NULL, 0);
    if (hunters == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    int myId = getpid() % MAX_HUNTERS;
    sprintf(hunters[myId].status, "Hunter aktif (PID: %d)", getpid());

    printf("Hunter %d menjalankan status: %s\n", myId, hunters[myId].status);

    sleep(10);

    strcpy(hunters[myId].status, "inactive");

    printf("Hunter %d keluar.\n", myId);
    shmdt(hunters);

    return 0;
}
```
Penjelasan kode:

Kode pada hunter.c berfungsi sebagai representasi dari proses seorang hunter yang akan mengakses dan memodifikasi data miliknya di shared memory yang sebelumnya telah dibuat oleh system.c. Program ini pertama-tama mencoba mengakses shared memory dengan `shmget`, menggunakan `SHM_KEY` yang sama dengan di system.c. Jika berhasil, program melampirkan shared memory ke proses menggunakan shmat. Hunter kemudian menentukan id-nya berdasarkan hasil `getpid() % MAX_HUNTERS`, sehingga setiap proses akan memiliki indeks berbeda (meskipun tidak dijamin unik). Status hunter pada indeks tersebut diubah menjadi aktif dengan mencantumkan PID-nya. Program kemudian menampilkan status hunter, menunggu selama 10 detik (mensimulasikan aktivitas), lalu mengubah status kembali menjadi "inactive" dan melepaskan akses ke shared memory. Program ini menunjukkan bagaimana proses lain dapat membaca dan menulis data pada shared memory secara bersamaan, mencerminkan interaksi antar proses dalam sistem hunter.

![1](https://github.com/user-attachments/assets/6f63551d-1e49-4155-874a-3c2a566f364b)

b. Membuat fitur registrasi dan login di program hunter.

Update file hunter.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define MAX_HUNTERS 10

typedef struct {
    int id;
    char username[20];
    int isRegistered;
    int isLoggedIn;
    int level;
    int exp;
    int atk;
    int hp;
    int def;
} Hunter;

void displayHunterMenu();
void displayHunterSystemMenu();
void registerHunter(Hunter *hunters);
void loginHunter(Hunter *hunters);
void listDungeon();
void raid();
void battle();
void toggleNotification();

int main() {
    // Menyambungkan ke shared memory
    int shmid = shmget(SHM_KEY, sizeof(Hunter) * MAX_HUNTERS, 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    Hunter *hunters = (Hunter *)shmat(shmid, NULL, 0);
    if (hunters == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    int choice;
    while (1) {
        displayHunterMenu();
        printf("Choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                registerHunter(hunters);
                break;
            case 2:
                loginHunter(hunters);
                break;
            case 3:
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice, try again.\n");
        }
    }

    return 0;
}

void displayHunterMenu() {
    printf("\n=== HUNTER MENU ===\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. Exit\n");
}

void displayHunterSystemMenu(const char *username) {
    printf("\n=== HUNTER SYSTEM ===\n");
    printf("=== %s's MENU ===\n", username);
    printf("1. List Dungeon\n");
    printf("2. Raid\n");
    printf("3. Battle\n");
    printf("4. Toggle Notification\n");
    printf("5. Exit\n");
}

void registerHunter(Hunter *hunters) {
    char inputUsername[20];
    printf("Username: ");
    scanf("%s", inputUsername);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, inputUsername) == 0) {
            printf("Username already taken!\n");
            return;
        }
    }

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (!hunters[i].isRegistered) {
            hunters[i].isRegistered = 1;
            strcpy(hunters[i].username, inputUsername);
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;
            printf("Registration success!\n");
            return;
        }
    }

    printf("Registration failed: No space for new hunters.\n");
}

void loginHunter(Hunter *hunters) {
    char inputUsername[20];
    printf("Username: ");
    scanf("%s", inputUsername);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, inputUsername) == 0) {
            if (hunters[i].isLoggedIn) {
                printf("Hunter already logged in!\n");
                return;
            }
            hunters[i].isLoggedIn = 1;
            printf("Login success! Welcome, %s.\n", hunters[i].username);
            displayHunterSystemMenu(hunters[i].username);

            // Tampilkan menu setelah login berhasil
            int choice;
            while (1) {
                printf("Choice: ");
                scanf("%d", &choice);
                switch (choice) {
                    case 1:
                        listDungeon();
                        break;
                    case 2:
                        raid();
                        break;
                    case 3:
                        battle();
                        break;
                    case 4:
                        toggleNotification();
                        break;
                    case 5:
                        hunters[i].isLoggedIn = 0;
                        printf("Logging out...\n");
                        return;
                    default:
                        printf("Invalid choice, try again.\n");
                }
            }
        }
    }

    printf("Login failed: Username not found.\n");
}

void listDungeon() {
    printf("Listing dungeons...\n");
}

void raid() {
    printf("Starting raid...\n");
}

void battle() {
    printf("Starting battle...\n");
}

void toggleNotification() {
    printf("Toggling notifications...\n");
}
```
Penjelasan kode:

mengakses shared memory yang telah dibuat oleh proses system.c menggunakan `shmget` dan `shmat`. Setelah terhubung, pemain diberikan pilihan untuk mendaftar atau masuk. Fungsi `registerHunter` memeriksa apakah username sudah digunakan, lalu menyimpan data hunter baru ke slot kosong di shared memory. Fungsi `loginHunter` memverifikasi username yang sudah terdaftar dan belum login, kemudian menampilkan menu sistem hunter. Di dalam menu ini, pengguna dapat memilih aksi seperti `listDungeon`, `raid`, `battle`, dan `toggleNotification` meskipun semua fitur ini masih berupa placeholder dengan output sederhana. Hunter dapat keluar dari sistem melalui opsi logout, yang akan mengubah status `isLoggedIn` menjadi 0. Program ini mencerminkan bagaimana proses hunter berinteraksi dengan sistem pusat menggunakan shared memory secara konkuren.

Tambahkan struct pada system.c
```c
typedef struct {
    int id;
    char key[20];
    int isRegistered;
    int isLoggedIn;
    int level;
    int exp;
    int atk;
    int hp;
    int def;
} Hunter;

for (int i = 0; i < MAX_HUNTERS; i++) {
    hunters[i].id = i;
    hunters[i].isRegistered = 0;
    hunters[i].isLoggedIn = 0;
    strcpy(hunters[i].key, "");
    hunters[i].level = 0;
    hunters[i].exp = 0;
    hunters[i].atk = 0;
    hunters[i].hp = 0;
    hunters[i].def = 0;
}
```

![2](https://github.com/user-attachments/assets/24b57a3a-3440-42da-b9c4-89b6dffe01b2)

c. Menambahkan opsi untuk melihat informasi hunter di system.c

Update system.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define SHM_KEY 1234
#define MAX_HUNTERS 10

typedef struct {
    int id;
    char username[20];
    int isRegistered;
    int isLoggedIn;
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int isBanned;  // Menambahkan status banned
} Hunter;

int shmid = -1;  // Declare global variable for shared memory

// Fungsi untuk menghapus shared memory ketika program berhenti
void cleanup(int signum) {
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);  // Hapus shared memory
        printf("Shared memory telah dihapus.\n");
    }
    exit(0);
}

void displaySystemMenu();
void displayHunterInfo(Hunter *hunters);
void banHunter(Hunter *hunters);
void resetHunter(Hunter *hunters);

int main() {
    // Menangani SIGINT (Ctrl+C) untuk cleanup shared memory
    signal(SIGINT, cleanup);

    // Membuat shared memory
    shmid = shmget(SHM_KEY, sizeof(Hunter) * MAX_HUNTERS, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory ke proses
    Hunter *hunters = (Hunter *)shmat(shmid, NULL, 0);
    if (hunters == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Menampilkan menu utama sistem
    int choice;
    while (1) {
        displaySystemMenu();
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayHunterInfo(hunters);
                break;
            case 2:
                // Implementasi Dungeon Info bisa ditambahkan nanti
                printf("Dungeon Info (belum tersedia)\n");
                break;
            case 3:
                // Implementasi Generate Dungeon bisa ditambahkan nanti
                printf("Generate Dungeon (belum tersedia)\n");
                break;
            case 4:
                banHunter(hunters);
                break;
            case 5:
                resetHunter(hunters);
                break;
            case 6:
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice, try again.\n");
        }
    }

    return 0;
}

void displaySystemMenu() {
    printf("\n=== SYSTEM MENU ===\n");
    printf("1. Hunter Info\n");
    printf("2. Dungeon Info\n");
    printf("3. Generate Dungeon\n");
    printf("4. Ban Hunter\n");
    printf("5. Reset Hunter\n");
    printf("6. Exit\n");
}

void displayHunterInfo(Hunter *hunters) {
    printf("\n=== HUNTER INFO ===\n");
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered) {
            printf("Name: %-20s Level: %-2d EXP: %-3d ATK: %-3d HP: %-3d DEF: %-3d Status: %-6s\n",
                   hunters[i].username, hunters[i].level, hunters[i].exp,
                   hunters[i].atk, hunters[i].hp, hunters[i].def,
                   hunters[i].isBanned ? "Banned" : "Active");
        }
    }
}

void banHunter(Hunter *hunters) {
    char username[20];
    printf("Enter username to ban: ");
    scanf("%s", username);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, username) == 0) {
            if (hunters[i].isBanned) {
                printf("Hunter %s is already banned.\n", hunters[i].username);
            } else {
                hunters[i].isBanned = 1;
                printf("Hunter %s has been banned.\n", hunters[i].username);
            }
            return;
        }
    }
    printf("Hunter not found.\n");
}

void resetHunter(Hunter *hunters) {
    char username[20];
    printf("Enter username to reset: ");
    scanf("%s", username);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, username) == 0) {
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;
            hunters[i].isBanned = 0;  // Reset banned status
            printf("Hunter %s has been reset.\n", hunters[i].username);
            return;
        }
    }
    printf("Hunter not found.\n");
}
```
Penjelasan kode:

Kode terbaru dari file system.c telah diperluas fungsionalitasnya untuk memungkinkan pengelolaan informasi hunter secara lebih lengkap melalui menu sistem. Program ini tetap menggunakan shared memory (`shmget` dan `shmat`) untuk menyimpan dan berbagi data hunter antar proses. Selain menangani sinyal `SIGINT` agar dapat membersihkan shared memory saat keluar, program ini sekarang memiliki beberapa fitur tambahan seperti menampilkan informasi hunter (displayHunterInfo), mem-ban hunter (banHunter), serta mereset status hunter ke kondisi awal (resetHunter). Masing-masing hunter yang terdaftar akan ditampilkan dengan atribut seperti nama, level, exp, atk, hp, def, dan status (active/banned). Fungsi ban memungkinkan admin melarang hunter tertentu, sementara fungsi reset akan mengembalikan atribut hunter ke nilai default dan mencabut status banned jika ada. Dengan demikian, kode ini memberikan kontrol penuh kepada administrator sistem dalam mengelola para hunter melalui antarmuka berbasis terminal.

![3](https://github.com/user-attachments/assets/92f06154-d219-41dc-9657-0c9ca78c3248)

d. Membuat program untuk men-generate dungeon

Buat struct baru di system.c
```c
#define MAX_DUNGEONS 10

typedef struct {
    char name[50];
    int minLevel;
    int rewardATK;
    int rewardHP;
    int rewardDEF;
    int rewardEXP;
    int isGenerated;
    int key;
} Dungeon;
```
Alokasi shared memory dungeon di `main()`
```c
#define SHM_KEY_DUNGEON 5678
int shmid_dungeon = shmget(SHM_KEY_DUNGEON, sizeof(Dungeon) * MAX_DUNGEONS, IPC_CREAT | 0666);
if (shmid_dungeon == -1) {
    perror("shmget dungeon failed");
    exit(1);
}

Dungeon *dungeons = (Dungeon *)shmat(shmid_dungeon, NULL, 0);
if (dungeons == (void *) -1) {
    perror("shmat dungeon failed");
    exit(1);
}

// Inisialisasi dungeon list
for (int i = 0; i < MAX_DUNGEONS; i++) {
    dungeons[i].isGenerated = 0;
}
```
Buat fungsi untuk generate dungeon
```c
#include <time.h>

char *dungeonNames[] = {
    "Gate of Zulqarnain", "Peak of Vindagnyr",
    "Annapausis",
    "Vourukasha Oasis",
    "The Secret of Al-Ahmar",
    "Stormterror's Lair"
};

void generateDungeon(Dungeon *dungeons) {
    srand(time(NULL));

    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (!dungeons[i].isGenerated) {
            strcpy(dungeons[i].name, dungeonNames[rand() % 6]);
            dungeons[i].minLevel = rand() % 5 + 1;
            dungeons[i].rewardATK = rand() % 51 + 100;
            dungeons[i].rewardHP = rand() % 51 + 50;
            dungeons[i].rewardDEF = rand() % 26 + 25;
            dungeons[i].rewardEXP = rand() % 151 + 150;
            dungeons[i].isGenerated = 1;

            printf("\nDungeon generated!\n");
            printf("Name: %s\n", dungeons[i].name);
            printf("Minimum Level: %d\n", dungeons[i].minLevel);
            return;
        }
    }

    printf("All dungeon slots are full!\n");
}
```


![4](https://github.com/user-attachments/assets/0c5f62a1-47dc-472f-a4b8-354368bd429b)

e. Menampilkan informasi lengkap semua dungeon

Menambahkan di fungsi `generateDungeon`
```c
dungeons[i].key = rand();
```
Dan menampilkan dungeon info
```c
void showDungeonInfo(Dungeon *dungeons) {
    printf("\n=== DUNGEON INFO ===\n");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (dungeons[i].isGenerated) {
            printf("[Dungeon %d]\n", i + 1);
            printf("Name: %s\n", dungeons[i].name);
            printf("Minimum Level: %d\n", dungeons[i].minLevel);
            printf("EXP Reward: %d\n", dungeons[i].rewardEXP);
            printf("ATK: %d\n", dungeons[i].rewardATK);
            printf("HP: %d\n", dungeons[i].rewardHP);
            printf("DEF: %d\n", dungeons[i].rewardDEF);
            printf("Key: %d\n\n", dungeons[i].key);
        }
    }
}
```

![5](https://github.com/user-attachments/assets/01aeaf2b-8188-4fcc-adc4-192966c04396)

f. Menambahkan fitur untuk melihat dungeon pada hunter

Tambahkan struct dungeon pada hunter.c
```c
#define DUNGEON_SHM_KEY 5678
#define MAX_DUNGEONS 10

typedef struct {
    char name[50];
    int minLevel;
    int rewardATK;
    int rewardHP;
    int rewardDEF;
    int rewardEXP;
    int isGenerated;
    int key;
} Dungeon;
```

Lalu membuat fungsi untuk menampilkan dungeon
```c
void listAvailableDungeons(int hunterLevel) {
    int shmid = shmget(DUNGEON_SHM_KEY, sizeof(Dungeon) * MAX_DUNGEONS, 0666);
    if (shmid == -1) {
        perror("shmget dungeon failed");
        return;
    }

    Dungeon *dungeons = (Dungeon *)shmat(shmid, NULL, 0);
    if (dungeons == (void *)-1) {
        perror("shmat dungeon failed");
        return;
    }

    printf("\n=== AVAILABLE DUNGEONS ===\n");
    int found = 0;
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (dungeons[i].isGenerated && dungeons[i].minLevel <= hunterLevel) {
            printf("%d. %s (Level %d+)\n", i + 1, dungeons[i].name, dungeons[i].minLevel);
            found = 1;
        }
    }

    if (!found) {
        printf("No available dungeons for your level.\n");
    }

    shmdt(dungeons);
}
```


![6](https://github.com/user-attachments/assets/ecd64331-f4dc-4e1e-bb76-f7c314829a58)

g. Menambahkan fitur untuk raid dungeon pada hunter

Tambahkan fungsi `raidDungeon()` di hunter.c
```c
void raidDungeon(Hunter *hunter) {
    int shmid = shmget(DUNGEON_SHM_KEY, sizeof(Dungeon) * MAX_DUNGEONS, 0666);
    if (shmid == -1) {
        perror("shmget dungeon failed");
        return;
    }

    Dungeon *dungeons = (Dungeon *)shmat(shmid, NULL, 0);
    if (dungeons == (void *)-1) {
        perror("shmat dungeon failed");
        return;
    }

    printf("\n=== RAIDABLE DUNGEONS ===\n");
    int indices[MAX_DUNGEONS];
    int count = 0;

    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (dungeons[i].isGenerated && dungeons[i].minLevel <= hunter->level) {
            printf("%d. %s (Level %d+)\n", count + 1, dungeons[i].name, dungeons[i].minLevel);
            indices[count++] = i;
        }
    }

    if (count == 0) {
        printf("No dungeons available for your level.\n");
        shmdt(dungeons);
        return;
    }

    int choice;
    printf("Choose Dungeon: ");
    scanf("%d", &choice);
    getchar(); // consume newline

    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        shmdt(dungeons);
        return;
    }

    int index = indices[choice - 1];
    Dungeon selected = dungeons[index];

    // Tambahkan reward ke hunter
    hunter->atk += selected.rewardATK;
    hunter->hp += selected.rewardHP;
    hunter->def += selected.rewardDEF;
    hunter->exp += selected.rewardEXP;

    printf("\nRaid success! Gained:\n");
    printf("ATK: %d\n", selected.rewardATK);
    printf("HP: %d\n", selected.rewardHP);
    printf("DEF: %d\n", selected.rewardDEF);
    printf("EXP: %d\n", selected.rewardEXP);

    // Level up check
    if (hunter->exp >= 500) {
        hunter->level += 1;
        hunter->exp = 0;
        printf("Congratulations! You've leveled up to Level %d!\n", hunter->level);
    }

    // Hapus dungeon
    dungeons[index].isGenerated = 0;

    // pause
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    
    shmdt(dungeons);
}
```
Penjelasan kode:

Fungsi raidDungeon memungkinkan seorang hunter untuk melakukan serangan (raid) ke dungeon yang tersedia berdasarkan levelnya. Pertama, fungsi mengakses shared memory yang menyimpan data dungeon menggunakan `shmget` dan `shmat`, kemudian menampilkan daftar dungeon yang sudah dihasilkan (isGenerated) dan memenuhi syarat level minimum (`minLevel <= hunter->level`). Hunter diminta memilih salah satu dungeon, dan jika pilihan valid, atribut hunter seperti atk, hp, def, dan exp akan ditambah sesuai reward dungeon tersebut. Jika setelah raid exp hunter mencapai 500 atau lebih, hunter akan naik level dan exp-nya di-reset ke 0. Setelah raid berhasil, dungeon yang telah diserang akan dihapus (ditandai dengan `isGenerated = 0`) agar tidak bisa diserang kembali. Fungsi ini juga menangani input user dengan validasi sederhana dan menyediakan jeda sebelum melanjutkan, serta memastikan shared memory dilepas dari proses dengan shmdt setelah selesai.

![7](https://github.com/user-attachments/assets/e3c860c1-fab7-46d5-9ca0-246c482409fe)

h. Menambahkan fitur untuk battle dengan hunter lain

Tambahkan fungsi `hunterBattle`
```c
void hunterBattle(Hunter *self, Hunter *hunters) {
    printf("\n=== PVP LIST ===\n");
    int indices[MAX_HUNTERS];
    int count = 0;

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, self->username) != 0) {
            int power = hunters[i].atk + hunters[i].hp + hunters[i].def;
            printf("%d. %s - Total Power: %d\n", count + 1, hunters[i].username, power);
            indices[count++] = i;
        }
    }

    if (count == 0) {
        printf("No available opponents.\n");
        pauseConsole();
        return;
    }

    int choice;
    printf("Target: ");
    scanf("%d", &choice);
    getchar(); // flush newline

    if (choice < 1 || choice > count) {
        printf("Invalid target.\n");
        pauseConsole();
        return;
    }

    int targetIdx = indices[choice - 1];
    Hunter *opponent = &hunters[targetIdx];

    printf("You chose to battle %s\n", opponent->username);

    int yourPower = self->atk + self->hp + self->def;
    int oppPower = opponent->atk + opponent->hp + opponent->def;

    printf("Your Power: %d\n", yourPower);
    printf("Opponent's Power: %d\n", oppPower);

    if (yourPower >= oppPower) {
        // Menang: Tambah stats dan hapus opponent
        self->atk += opponent->atk;
        self->hp += opponent->hp;
        self->def += opponent->def;

        printf("Battle won! You acquired %s's stats\n", opponent->username);

        // Reset opponent
        memset(opponent, 0, sizeof(Hunter));
    } else {
        // Kalah: Transfer stats ke opponent dan hapus self
        opponent->atk += self->atk;
        opponent->hp += self->hp;
        opponent->def += self->def;

        printf("You lost the battle... Your stats have been given to %s\n", opponent->username);
        memset(self, 0, sizeof(Hunter));

        pauseConsole();
        // Keluar karena hunter dihapus
        exit(0);
    }

    pauseConsole();
}
```
Penjelasan kode:

Fungsi hunterBattle memungkinkan seorang hunter untuk melakukan pertarungan PvP melawan hunter lain yang terdaftar di sistem. Fungsi ini terlebih dahulu menampilkan daftar semua hunter lain beserta total kekuatannya (jumlah dari atk, hp, dan def), kemudian meminta pengguna memilih target dari daftar tersebut. Jika target valid, sistem membandingkan total kekuatan antara hunter pengguna (self) dan lawannya (opponent). Jika pengguna menang, semua atribut lawan akan ditambahkan ke atribut pengguna dan data lawan akan di-reset menggunakan `memset`, menandakan lawan kalah. Sebaliknya, jika pengguna kalah, maka atribut pengguna ditransfer ke lawan dan data pengguna akan dihapus, diikuti dengan `exit(0)` untuk keluar dari program karena hunter tersebut dianggap gugur. Fungsi ini juga menggunakan pauseConsole di beberapa titik untuk menunggu input sebelum melanjutkan, memberikan waktu bagi pengguna membaca hasil pertarungan.

![8](https://github.com/user-attachments/assets/42b012f6-7c9b-4c3e-a81c-e8568660d83f)
![8_1](https://github.com/user-attachments/assets/29256d19-0851-4205-831b-b5fba5934d06)

i. Menambahkan opsi untuk ban/unban hunter pada system

Tambahkan fungsi `banOrUnbanHunter()` di system.c
```c
void banOrUnbanHunter(Hunter *hunters) {
    char target[20];
    int action;

    printf("Enter hunter's username to modify ban status: ");
    scanf("%s", target);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, target) == 0) {
            printf("Current status: %s\n", hunters[i].isBanned ? "Banned" : "Active");
            printf("1. Ban\n2. Unban\nChoice: ");
            scanf("%d", &action);

            if (action == 1) {
                hunters[i].isBanned = 1;
                printf("Hunter %s has been banned from raid and battle.\n", target);
            } else if (action == 2) {
                hunters[i].isBanned = 0;
                printf("Hunter %s has been unbanned.\n", target);
            } else {
                printf("Invalid action.\n");
            }
            return;
        }
    }
    printf("Hunter not found.\n");
}
```
Penjelasan kode:

Fungsi banOrUnbanHunter digunakan untuk memodifikasi status banned dari seorang hunter berdasarkan username-nya. Fungsi ini menerima array hunters dari shared memory, lalu meminta pengguna untuk memasukkan username hunter yang ingin dicari. Jika hunter ditemukan dan sudah terdaftar, sistem akan menampilkan status saat ini (banned atau aktif) dan memberikan dua pilihan: 1 untuk melakukan ban dan 2 untuk melakukan unban. Bila pengguna memilih salah satu opsi dengan benar, status isBanned pada struct hunter akan diperbarui sesuai pilihan, disertai pesan konfirmasi. Jika input pilihan tidak valid atau username tidak ditemukan, maka akan muncul pesan kesalahan yang sesuai.

![9](https://github.com/user-attachments/assets/196264fc-19d3-46c7-a3dd-f09d95f9bc47)
![9_1](https://github.com/user-attachments/assets/98f196d3-3c68-4bf3-9eb6-f80a58f74544)

j. Menambahkan opsi untuk mengulang stat hunter pada system

Tambahkan fungsi `resetHunter` di system.c
```c
void resetHunter(Hunter *hunters) {
    char target[20];
    printf("Enter hunter's username to reset: ");
    scanf("%s", target);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].isRegistered && strcmp(hunters[i].username, target) == 0) {
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;
            hunters[i].isLoggedIn = 0;

            printf("Hunter %s has been reset to default stats.\n", target);
            return;
        }
    }

    printf("Hunter not found.\n");
}
```

Penjelasan kode:

Fungsi resetHunter digunakan untuk mengatur ulang status dan atribut dasar seorang hunter yang telah terdaftar berdasarkan username yang dimasukkan oleh pengguna. Setelah menerima input username, fungsi akan mencari hunter yang cocok di array hunters. Jika ditemukan, hunter tersebut akan di-reset ke kondisi awal: level dikembalikan ke 1, exp menjadi 0, serta atribut dasar seperti atk, hp, dan def disetel ke 10, 100, dan 5. Selain itu, status login (`isLoggedIn`) juga di-set ke 0 agar hunter dianggap telah logout. Jika hunter tidak ditemukan, sistem akan menampilkan pesan bahwa hunter tidak ditemukan. Fungsi ini bermanfaat untuk admin yang ingin mengatur ulang progress seorang hunter tanpa menghapus akunnya.

![10](https://github.com/user-attachments/assets/14c4f086-14b3-48a4-87fc-29f19a17b965)
![10_1](https://github.com/user-attachments/assets/1c3be144-90d3-40c4-ad67-1dd3f6397358)

k. Menambahkan fitur untuk notifikasi dungeon yang tersedia pada hunter

buat fungsi `dungeonNotification`
```c
void *dungeonNotification(void *arg) {
    while (running) {
        if (notificationOn && !pauseNotification) {
            int shmid = shmget(DUNGEON_SHM_KEY, sizeof(Dungeon) * MAX_DUNGEONS, 0666);
            if (shmid == -1) {
                perror("shmget failed (notif)");
                sleep(3);
                continue;
            }

            Dungeon *dungeons = (Dungeon *)shmat(shmid, NULL, 0);
            if (dungeons == (void *)-1) {
                perror("shmat failed (notif)");
                sleep(3);
                continue;
            }

            int validIndex[MAX_DUNGEONS];
            int count = 0;
            for (int i = 0; i < MAX_DUNGEONS; i++) {
                if (dungeons[i].isGenerated && dungeons[i].minLevel <= currentHunterLevel) {
                    validIndex[count++] = i;
                }
            }

            if (count > 0) {
                int randIndex = validIndex[rand() % count];
                printf("\n=== HUNTER SYSTEM ===\n");
                printf("Dungeon \"%s\" for minimum level %d opened!\n", dungeons[randIndex].name, dungeons[randIndex].minLevel);
                printf("=== %s's MENU ===\n", currentHunterUsername);
            }

            shmdt(dungeons);
        }

        sleep(3);
    }

    return NULL;
}
```
Penjelasan kode:

Fungsi dungeonNotification berjalan sebagai thread terpisah yang secara berkala (setiap 3 detik) memantau dungeon yang tersedia dan sesuai level hunter saat ini, selama notifikasi (notificationOn) aktif dan tidak sedang dijeda (pauseNotification). Fungsi ini mengakses shared memory yang menyimpan data dungeon, lalu mencari dungeon yang telah digenerate (isGenerated) dan dapat diakses oleh hunter berdasarkan level (`minLevel <= currentHunterLevel`). Jika ada dungeon yang memenuhi syarat, maka salah satu dari mereka dipilih secara acak dan informasi pembukaannya ditampilkan di layar dengan mencantumkan nama dungeon dan level minimum. Tujuan dari fungsi ini adalah memberikan notifikasi real-time kepada hunter agar mereka mengetahui dungeon baru yang tersedia dan dapat segera mengambil tindakan, seperti melakukan raid.

![11](https://github.com/user-attachments/assets/a0a99ebd-a13c-481b-bd7e-f234df52d22d)

l. Menghapus semua shared memory ketika sistem dimatikan

Tambahkan fungsi `cleanup()` di system.c
```c
void cleanup(int signum) {
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);  // Hapus shared memory hunter
        printf("Shared memory hunter telah dihapus.\n");
    }

    int shmid_dungeon = shmget(SHM_KEY_DUNGEON, sizeof(Dungeon) * MAX_DUNGEONS, 0666);
    if (shmid_dungeon != -1) {
        shmctl(shmid_dungeon, IPC_RMID, NULL);  // Hapus shared memory dungeon
        printf("Shared memory dungeon telah dihapus.\n");
    }

    exit(0);
}
```
Penjelasan kode:

Fungsi cleanup ini bertugas untuk membersihkan resource shared memory ketika program menerima sinyal tertentu (misalnya `SIGINT` dari Ctrl+C). Jika shared memory untuk hunter (`shmid`) masih terhubung, maka akan dihapus menggunakan `shmctl` dengan flag IPC_RMID, lalu mencetak pesan bahwa shared memory hunter telah dihapus. Setelah itu, fungsi mencoba mendapatkan ID dari shared memory dungeon (shmid_dungeon) menggunakan `shmget`. Jika berhasil, shared memory dungeon juga dihapus dengan cara yang sama dan disusul dengan pesan konfirmasi. Fungsi ini penting untuk mencegah kebocoran resource di sistem dan memastikan semua segment shared memory yang digunakan program dibersihkan dengan benar saat program berhenti.

![12](https://github.com/user-attachments/assets/4f1ad7b0-5fe6-4bc4-a7b5-872545e9501f)

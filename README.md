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

        // Simulasi respon
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
                // contoh command dikirim ke dungeon server
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
        // Ekstrak damage & nama senjata
        char* dmg_str = strstr(buffer, "New damage: ");
        if (dmg_str) {
            int dmg;
            sscanf(dmg_str, "New damage: %d", &dmg);
            player.base_damage = dmg;
        }

        // Ekstrak nama senjata
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

f.

![6](https://github.com/user-attachments/assets/0b7e23e0-791d-42e2-99b6-f6d44be7ceb0)
![6_1](https://github.com/user-attachments/assets/19ecd867-e9d7-448b-b0ce-39be433ffca4)

**Revisi**

![revisi](https://github.com/user-attachments/assets/e84048fe-c199-43f0-81f6-be86a055fe78)

g.

![Capture7](https://github.com/user-attachments/assets/5255210d-a109-445d-ad92-7a740bc771ef)
![Capture7_1](https://github.com/user-attachments/assets/64d0aca0-68fa-4e23-bc5e-710e404b2581)

h. 

![Capture8](https://github.com/user-attachments/assets/f87e51c5-12e0-40b6-8af8-0d0f72a5869f)

# Soal 4
a.

![1](https://github.com/user-attachments/assets/6f63551d-1e49-4155-874a-3c2a566f364b)

b.

![2](https://github.com/user-attachments/assets/24b57a3a-3440-42da-b9c4-89b6dffe01b2)

c.

![3](https://github.com/user-attachments/assets/92f06154-d219-41dc-9657-0c9ca78c3248)

d.

![4](https://github.com/user-attachments/assets/0c5f62a1-47dc-472f-a4b8-354368bd429b)

e.

![5](https://github.com/user-attachments/assets/01aeaf2b-8188-4fcc-adc4-192966c04396)

f. 

![6](https://github.com/user-attachments/assets/ecd64331-f4dc-4e1e-bb76-f7c314829a58)

g.

![7](https://github.com/user-attachments/assets/e3c860c1-fab7-46d5-9ca0-246c482409fe)

h. 

![8](https://github.com/user-attachments/assets/42b012f6-7c9b-4c3e-a81c-e8568660d83f)
![8_1](https://github.com/user-attachments/assets/29256d19-0851-4205-831b-b5fba5934d06)

i.

![9](https://github.com/user-attachments/assets/196264fc-19d3-46c7-a3dd-f09d95f9bc47)
![9_1](https://github.com/user-attachments/assets/98f196d3-3c68-4bf3-9eb6-f80a58f74544)

j.

![10](https://github.com/user-attachments/assets/14c4f086-14b3-48a4-87fc-29f19a17b965)
![10_1](https://github.com/user-attachments/assets/1c3be144-90d3-40c4-ad67-1dd3f6397358)


k. 

![11](https://github.com/user-attachments/assets/a0a99ebd-a13c-481b-bd7e-f234df52d22d)

l.

![12](https://github.com/user-attachments/assets/4f1ad7b0-5fe6-4bc4-a7b5-872545e9501f)

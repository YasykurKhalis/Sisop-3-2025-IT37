// player.c (RPC Client)

// soal a
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345

// soal c
typedef struct {
    int money;
    char equipped_weapon[50];
    char weapon_passive[100];
    int base_damage;
    int enemies_defeated;
} Player;

Player player = {
    .money = 100,
    .equipped_weapon = "Rusty Sword",
    .weapon_passive = "",
    .base_damage = 10,
    .enemies_defeated = 0
}; // c

// soal e
typedef struct {
    char name[50];
    int damage;
    char passive[100];
} Weapon;

Weapon inventory[100];
int inventory_count = 0;
int equipped_index = -1; // e

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[1024], input[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    // connect(sock, (struct sockaddr*)&server, sizeof(server));
    // printf("Connected to dungeon server.\n");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to dungeon server.\n");
    
    // soal b
    int running = 1;
    while (running) {

        printf("\n===== Main Menu =====\n");
        printf("1. Show Player Stats\n");
        printf("2. Shop (Buy Weapons)\n");
        printf("3. View Inventory & Equip Weapons\n");
        printf("4. Battle Mode\n");
        printf("5. Exit game\n");
        printf("Choose an option: ");

        fgets(input, sizeof(input), stdin);

        switch (input[0]) {
            case '1':
                // soal c
                printf("\n[Player Stats]\n");
                printf("Gold             : %d gold\n", player.money);
                printf("Equipped Weapon  : %s\n", player.equipped_weapon);
                printf("Base Damage      : %d\n", player.base_damage);
                printf("Kills            : %d\n", player.enemies_defeated); 
                // soal e
                if (equipped_index >= 0 && strlen(inventory[equipped_index].passive) > 0) {
                    printf("Passive: %s\n", inventory[equipped_index].passive);
                } // e 
                // c
                break;
            case '2':
                // soal d
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
            
                // Parsial parsing respon (jika berhasil beli, update data)
                if (strstr(buffer, "Bought")) {
                    // Ekstrak damage & nama senjata
                    int dmg = 0;
                    char wname[50] = "";
                    char passive_buf[100] = "";

                    char* dmg_str = strstr(buffer, "New damage: ");
                    if (dmg_str) {
                        sscanf(dmg_str, "New damage: %d", &dmg);
                    }
            
                    // Ekstrak nama senjata
                    char* start = strstr(buffer, "Bought ");
                    if (start) {
                        sscanf(start, "Bought %[^\n!]", wname);
                    }

                    char* money_str = strstr(buffer, "Remaining gold: ");
                    if (money_str) {
                        int new_money;
                        sscanf(money_str, "Remaining gold: %d", &new_money);
                        player.money = new_money;
                    }

                    // soal e
                    char* p_start = strstr(buffer, "Passive: ");
                    if (p_start) {
                        sscanf(p_start, "Passive: %[^\n\r]", passive_buf);
                    }

                    if (inventory_count < 100 && strlen(wname) > 0) {
                        strcpy(inventory[inventory_count].name, wname);
                        inventory[inventory_count].damage = dmg;
                        strcpy(inventory[inventory_count].passive, passive_buf); // dari parsing, bisa simpan sebelumnya
                        inventory_count++;
                        printf("Weapon '%s' added to inventory.\n", wname);
                    } // e

                } // d
                break;
            case '3':
                // soal e
                if (inventory_count == 0) {
                    printf("Inventory is empty!\n");
                    break;
                }

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
                // char choice[10];
                memset(choice, 0, sizeof(choice));

                fgets(choice, sizeof(choice), stdin);
                int idx = atoi(choice);
                if (idx >= 0 && idx < inventory_count) {
                    equipped_index = idx;
                    strcpy(player.equipped_weapon, inventory[idx].name);
                    player.base_damage = inventory[idx].damage;
                    strcpy(player.weapon_passive, inventory[idx].passive);
                    printf("Equipped %s!\n", player.equipped_weapon);
                } else {
                    printf("Cancelled.\n");
                } // e
                break;
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
                        char attack_cmd[128];
                        sprintf(attack_cmd, "battle attack %d|%s", player.base_damage, player.weapon_passive);
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

                    // reward parsing
                    char* reward_str = strstr(buffer, "[REWARD:");
                    if (reward_str) {
                        int reward;
                        sscanf(reward_str, "[REWARD:%d]", &reward);
                        player.money += reward;
                        printf("[INFO] You now have %d gold.\n", player.money);
                    }

                    // cek apakah musuh dikalahkan
                    if (strstr(buffer, "You defeated the enemy!")) {
                        player.enemies_defeated++;
                    }
            
                    // cek apakah musuh sudah mati
                    if (strstr(buffer, "defeated") || strstr(buffer, "fled")) {
                        break;
                    }
                }
                break;
            case '5':
                printf("Exiting game...\n");
                running = 0;
                break;
            // soal h
            default:
                printf("Invalid option. Please choose 1-5.\n"); // h
        }
    } // b

    close(sock);
    return 0;
} // a
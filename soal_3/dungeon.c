#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <time.h>

#include "shop.c"

#define PORT 12345

typedef struct {
    int hp;
    int max_hp;
    int reward;
    int in_battle;
} Enemy;

void generate_health_bar(char* bar, int hp, int max_hp) {
    int bar_width = 20; // total kotak bar (misalnya 20 karakter)
    int filled = (hp * bar_width) / max_hp;

    strcpy(bar, "[");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled)
            strcat(bar, "█");
        else
            strcat(bar, "-");
    }
    strcat(bar, "]");
}

void* handle_client(void* socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];

    Enemy enemy = {.in_battle = 0};

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int read_size = recv(sock, buffer, sizeof(buffer), 0);
        if (read_size <= 0) break;

        if (strncmp(buffer, "shop", 4) == 0) {
            char response[1024];
            list_weapons(response);
            send(sock, response, strlen(response), 0);
        }
        else if (strncmp(buffer, "buy", 3) == 0) {
            int id, money;
            sscanf(buffer + 4, "%d %d", &id, &money);
        
            int dmg;
            char wname[50], passive[100];
            int price = buy_weapon(id, &dmg, wname, passive);
        
            char response[256];
            if (price == -1) {
                sprintf(response, "Invalid weapon ID.");
            } else if (money < price) {
                sprintf(response, "Not enough gold. Price: %d", price);
            } else {
                int remaining = money - price;
                sprintf(response, "Bought %s! New damage: %d. Passive: %s\nRemaining gold: %d", wname, dmg, passive, remaining);
            }
            send(sock, response, strlen(response), 0);
        }
        
        else if (strncmp(buffer, "battle start", 12) == 0) {
            enemy.max_hp = 50 + rand() % 151; 
            enemy.hp = enemy.max_hp;
            enemy.reward = 100 + rand() % 101;
            enemy.in_battle = 1;
        
            char res[128];
            char hp_bar[64];

            generate_health_bar(hp_bar, enemy.hp, enemy.max_hp);
            sprintf(res, "A wild enemy appears! HP: %s %d/%d\n", hp_bar, enemy.hp, enemy.max_hp);
            send(sock, res, strlen(res), 0);
        }
        
        else if (strncmp(buffer, "battle attack", 13) == 0 && enemy.in_battle) {
            int base_dmg = 0;
            char passive[100] = "";
            int chance = rand() % 101;
            char extra_info[128] = "";

            // Parsing format: "battle attack <base_dmg>|<passive>"
            char* dmg_str = strtok(buffer + 14, "|");
            char* passive_str = strtok(NULL, "\n");

            if (dmg_str) base_dmg = atoi(dmg_str);
            if (passive_str) strncpy(passive, passive_str, sizeof(passive) - 1);

            int variation = base_dmg * (rand() % 21 - 10) / 100; // -10% to +10%
            int final_dmg = base_dmg + variation;

            int crit_chance = rand() % 101;
            int is_crit = 0;
            if (crit_chance < 25) { // 25% chance
                final_dmg *= 2;
                is_crit = 1;
                strcat(extra_info, "💥 Critical Hit!\n");
            }

            if (strcmp(passive, "(Burn: 40% chance to burn enemy)") == 0) {
                if (chance < 40) {
                    final_dmg += 2;
                    strcat(extra_info, "🔥 Enemy burned!\n");
                }
            }

            if (strcmp(passive, "(Freeze: 30% chance to freeze enemy)") == 0) {
                if (chance < 30) {
                    final_dmg += 1;
                    strcat(extra_info, "🥶 Enemy frozen!\n");
                }
            }

            if (strcmp(passive, "(Instant Kill: 15% chance to kill enemy instantly)") == 0) {
                if (chance < 15) {
                    final_dmg = enemy.hp;;
                    strcat(extra_info, "💢 Fatality!\n");
                }
            }
        
            enemy.hp -= final_dmg;
            if (enemy.hp < 0) enemy.hp = 0;

            char res[256];
            char hp_bar[64];

            generate_health_bar(hp_bar, enemy.hp, enemy.max_hp);

            if (enemy.hp <= 0) {
                enemy.in_battle = 0;
                sprintf(res, "%s\nYou defeated the enemy! You gained %d gold. [REWARD:%d]\n\n", extra_info, enemy.reward, enemy.reward);
            } else {
                sprintf(res, "%s\nYou dealt %d damage!\nEnemy HP: %s %d/%d\n", extra_info, final_dmg, hp_bar, enemy.hp, enemy.max_hp);
            }
            send(sock, res, strlen(res), 0);
        }
        else if (strncmp(buffer, "battle exit", 11) == 0) {
            enemy.in_battle = 0;
            send(sock, "You fled the battle.\n", 22, 0);
        }

        printf("Received command: %s\n", buffer);
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

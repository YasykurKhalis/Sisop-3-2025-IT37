#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define SHM_KEY 1234

// dungeon generator
#define SHM_KEY_DUNGEON 5678

#define MAX_HUNTERS 10
#define MAX_DUNGEONS 30

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

typedef struct {
    char name[50];
    int minLevel;
    int rewardATK;
    int rewardHP;
    int rewardDEF;
    int rewardEXP;
    int isGenerated; // penanda apakah dungeon valid
    int key;
} Dungeon;

int shmid = -1;  // Declare global variable for shared memory

// Fungsi untuk menghapus shared memory ketika program berhenti
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

// inisialisasi fungsi
void displaySystemMenu();
void displayHunterInfo(Hunter *hunters);
void banOrUnbanHunter(Hunter *hunters);
void resetHunter(Hunter *hunters);
void generateDungeon(Dungeon *dungeons);
void showDungeonInfo(Dungeon *dungeons);


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
                showDungeonInfo(dungeons);
                break;
            case 3:
                // Implementasi Generate Dungeon bisa ditambahkan nanti
                generateDungeon(dungeons);
                break;
            case 4:
                banOrUnbanHunter(hunters);
                break;
            case 5:
                resetHunter(hunters);
                break;
            case 6:
                cleanup(0);
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice, try again.\n");
        }
    }

    return 0;
}


// fungsi
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

char *dungeonNames[] = {
    "Gate of Zulqarnain", "Peak of Vindagnyr", "Annapausis",
    "Vourukasha Oasis", "The Secret of Al-Ahmar", "Stormterror's Lair"
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

            dungeons[i].key = rand(); // Generate pseudo-unique key

            printf("\nDungeon generated!\n");
            printf("Name: %s\n", dungeons[i].name);
            printf("Minimum Level: %d\n", dungeons[i].minLevel);
            return;
        }
    }

    printf("All dungeon slots are full!\n");
}

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
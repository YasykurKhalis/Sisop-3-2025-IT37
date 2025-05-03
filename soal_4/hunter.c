// hunter.c

// soal a
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define SHM_KEY 1234
#define MAX_HUNTERS 10

// dungeon generator
#define DUNGEON_SHM_KEY 5678
#define MAX_DUNGEONS 30

// variable global untuk notifikasi
char currentHunterUsername[20];
int notificationOn = 0;
int currentHunterLevel = 1;
int running = 1;
int pauseNotification = 0;

typedef struct {
    int id; // key
    // soal b
    char username[20];
    int isRegistered;
    int isLoggedIn;
    int level;
    int exp;
    int atk;
    int hp;
    int def; // b
    // soal c
    int isBanned; // c
} Hunter;

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

// inisialisasi fungsi
// soal b
void displayHunterMenu();
void displayHunterSystemMenu();
void registerHunter(Hunter *hunters);
void loginHunter(Hunter *hunters);
void listAvailableDungeon(int hunterLevel);
void raidDungeon(Hunter *hunters);
void hunterBattle(Hunter *self, Hunter *hunters);
void toggleNotification(); // b
void *dungeonNotification(void *arg); 

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
} // a

// fungsi
void displayHunterMenu() {
    printf("\n=== HUNTER MENU ===\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. Exit\n");
}

void displayHunterSystemMenu(const char *username) {
    printf("=== %s's MENU ===\n", username);
    printf("1. List Dungeon\n");
    printf("2. Raid\n");
    printf("3. Battle\n");
    printf("4. Toggle Notification\n");
    printf("5. Exit\n");
    printf("Choice: ");
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
            hunters[i].id = i;
            hunters[i].isRegistered = 1;
            hunters[i].isLoggedIn = 0;
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

void pauseConsole() {
    printf("Press enter to continue...");
    while (getchar() != '\n');
    getchar();
}

void listAvailableDungeon(int hunterLevel) {
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
    int dungeonIndex = 1;
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (dungeons[i].isGenerated && dungeons[i].minLevel <= hunterLevel) {
            printf("%d. %s (Level %d+)\n", dungeonIndex, dungeons[i].name, dungeons[i].minLevel);
            dungeonIndex++;
            found = 1;
        }
    }

    if (!found) {
        printf("No available dungeons for your level.\n");
    }

    shmdt(dungeons);
}

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
    // getchar(); // consume newline

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
        currentHunterLevel = hunter->level; // update variabel global untuk notifikasi
        printf("Congratulations! You've leveled up to Level %d!\n", hunter->level);
    }

    // Hapus dungeon
    dungeons[index].isGenerated = 0;
    
    shmdt(dungeons);
}

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
    // getchar(); // flush newline

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
}

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

            // Pilih dungeon acak yang sesuai level
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
                printf("%s for minimum level %d opened!\n", dungeons[randIndex].name, dungeons[randIndex].minLevel);
                displayHunterSystemMenu(currentHunterUsername);
            }

            shmdt(dungeons);
        }

        sleep(3);
    }

    return NULL;
}

void toggleNotification() {
    notificationOn = !notificationOn;
    if (notificationOn)
        printf("Dungeon notification enabled!\n");
    else
        printf("Dungeon notification disabled.\n");
}

void loginHunter(Hunter *hunters) {
    char inputUsername[20];
    pthread_t notifThread;
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

            // simpan username dan level ke variabel global
            strcpy(currentHunterUsername, hunters[i].username);
            currentHunterLevel = hunters[i].level;

            // Notifikasi
            srand(time(NULL)); // untuk random
            running = 1;
            pthread_create(&notifThread, NULL, dungeonNotification, NULL);

            // Tampilkan menu setelah login berhasil
            int choice;
            while (1) {
                printf("\n=== HUNTER SYSTEM ===\n");
                displayHunterSystemMenu(hunters[i].username);
                scanf("%d", &choice);
                switch (choice) {
                    case 1:
                        pauseNotification = 1;
                        listAvailableDungeon(hunters[i].level);
                        pauseConsole();
                        pauseNotification = 0;
                        break;
                    case 2:
                        pauseNotification = 1;
                        if (hunters[i].isBanned) {
                            printf("You are banned from raiding!\n");
                        } else {
                            raidDungeon(&hunters[i]);
                        }
                        pauseConsole();
                        pauseNotification = 0;
                        break;
                    case 3:
                        if (hunters[i].isBanned) {
                            printf("You are banned from battling!\n");
                        } else {
                            hunterBattle(&hunters[i], hunters);
                        }
                        pauseConsole();
                        pauseNotification = 0;
                        break;
                    case 4:
                        toggleNotification();
                        break;
                    case 5:
                        running = 0;                    // Hentikan thread
                        pthread_join(notifThread, NULL); // Tunggu thread selesai

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
// b

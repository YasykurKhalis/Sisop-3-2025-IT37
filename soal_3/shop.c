// soal d
#include <stdio.h>
#include <string.h>

typedef struct {
    char name[50];
    int price;
    int damage;
    char passive[100];
} Weapon;

Weapon weapon_list[] = {
    {"Dull Blade", 20, 11, "No passive"},
    {"Diamond Axe", 50, 15, "No passive"},
    {"KFP Sword", 80, 12, "(Burn: 40% chance to burn enemy)"},
    {"Obama Prism", 120, 18, "(Freeze: 30% chance to freeze enemy)"},
    {"Interastral Baseball Bat", 150, 20, "(Instant Kill: 15% chance to kill enemy instantly)"}
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

// Returns: price if valid, -1 if invalid
int buy_weapon(int id, int* damage, char* name_out, char* passive_out) {
    if (id < 1 || id > weapon_count) return -1;

    Weapon w = weapon_list[id - 1];
    *damage = w.damage;
    strcpy(name_out, w.name);
    strcpy(passive_out, w.passive);

    return w.price;
} // d
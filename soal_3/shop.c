#include <stdio.h>
#include <string.h>

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[0m"

typedef enum PassiveEffect {
    NONE,
    CRIT,
    INSTANT_KILL,
    BURN,
    BLEED
} PassiveEffect;

typedef struct Weapon {
    char name[50];
    int price;
    int damage;
    PassiveEffect passive;
} Weapon;

Weapon shop[] = {
    {"Fists", 0, 5, NONE},
    {"Terra Blade", 50, 10, NONE},
    {"Flint & Steel", 150, 25, NONE},
    {"Kitchen Knife", 200, 35, NONE},
    {"Staff of Light", 120, 20, INSTANT_KILL},
    {"Dragon Claws", 300, 50, CRIT}
};

const int shop_size = sizeof(shop) / sizeof(Weapon);

const char* get_passive_name(PassiveEffect effect) {
    switch(effect) {
        case CRIT: return "+30% Crit Chance";
        case INSTANT_KILL: return "10% Insta-Kill";
        default: return "None";
    }
}

void show_shop_weapons(int sock) {
    char buffer[1024];
    send_str(sock, GREEN "\n=== WEAPON SHOP ===\n" RESET);

    for(int i = 1; i < shop_size; i++) {
        Weapon w = shop[i];
        snprintf(buffer, sizeof(buffer), 
            "[%d] %s - Price: %d | Damage: %d", 
            i,
            w.name, 
            w.price, 
            w.damage
        );
        
        if(w.passive != NONE) {
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
                " | Passive: %s",
                get_passive_name(w.passive)
            );
        }
        strcat(buffer, "\n");
        send_str(sock, buffer);
    }
    send_str(sock, "Enter weapon number to buy (0 to cancel): ");
}

void buy_weapon(int sock, int idx, int* gold, Weapon* inventory, int* count) {
    if (idx < 1 || idx >= shop_size) {
        send_str(sock, RED "Invalid selection!\n" RESET);
        return;
    }

    Weapon selected = shop[idx];

    for (int i = 0; i < *count; i++) {
        if (strcmp(inventory[i].name, selected.name) == 0) {
            send_str(sock, RED "You already own this weapon!\n" RESET);
            return;
        }
    }

    if (*gold < selected.price) {
        send_str(sock, RED "Not enough gold!\n" RESET);
        return;
    }

    if (*count >= MAX_INVENTORY) {
        send_str(sock, RED "Inventory full!\n" RESET);
        return;
    }

    *gold -= selected.price;
    inventory[*count] = selected;
    (*count)++;
    send_str(sock, GREEN "Purchase successful!\n" RESET);
}

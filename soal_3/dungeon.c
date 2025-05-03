#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define MAX_INVENTORY 10
#define PORT 8080

int send_str(int sock, const char* str);
const char* get_health_bar(int hp, int max_hp);
void clear_input_buffer(int sock);

#include "shop.c"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[0m"

typedef struct {
    int gold;
    int base_damage;
    int kills;
    Weapon equipped_weapon;
    Weapon inventory[MAX_INVENTORY];
    int inventory_count;
} PlayerState;

const char* get_health_bar(int hp, int max_hp) {
    static char bar[21];
    int filled = (hp * 20) / max_hp;
    memset(bar, 0, sizeof(bar));
    for(int i = 0; i < 20; i++) bar[i] = (i < filled) ? '#' : ' ';
    return bar;
}

void clear_input_buffer(int sock) {
    char dummy;
    while(recv(sock, &dummy, 1, MSG_DONTWAIT) > 0);
}

int receive_choice(int sock) {
    int choice;
    ssize_t bytes = recv(sock, &choice, sizeof(choice), MSG_WAITALL);
    return (bytes == sizeof(choice)) ? choice : -1;
}

int send_str(int sock, const char* str) {
    size_t len = strlen(str);
    ssize_t sent = send(sock, str, len, 0);
    return (sent == (ssize_t)len) ? 0 : -1;
}

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

void show_shop(int sock, PlayerState* ps) {
    show_shop_weapons(sock);
    int choice = receive_choice(sock);
    
    if(choice > 0 && choice < shop_size) {
        buy_weapon(sock, choice, &ps->gold, ps->inventory, &ps->inventory_count);
    } else if(choice != 0) {
        send_str(sock, RED "Invalid choice!\n" RESET);
    }
    clear_input_buffer(sock);
}

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

void battle_mode(int sock, PlayerState* ps) {
    char buffer[1024];
    char command[16];
    int enemy_hp = 50 + rand() % 151; // 50-200 HP
    int max_hp = enemy_hp;

    send_str(sock, RED "\n=== BATTLE STARTED ===\n" RESET);
    
    while(1) {
        snprintf(buffer, sizeof(buffer), 
            "Enemy appeared with:\n[%s] %d/%d HP\n"
            "Type 'attack' to attack or 'exit' to leave battle.\n> ",
            get_health_bar(enemy_hp, max_hp), enemy_hp, max_hp
        );
        send_str(sock, buffer);

        memset(command, 0, sizeof(command));
        recv(sock, command, sizeof(command), 0);

        if (strcasecmp(command, "exit\n") == 0) {
            send_str(sock, "\nReturning to main menu...\n");
            break;
        } 
        
        else if (strcasecmp(command, "attack\n") == 0) {
            int damage = ps->equipped_weapon.damage + (rand() % 11);
            int roll = rand() % 100;
            
            if(ps->equipped_weapon.passive == CRIT && roll < 30) {
                damage *= 2;
                send_str(sock, YELLOW "\nCRITICAL HIT!\n" RESET);
            }
            else if(ps->equipped_weapon.passive == INSTANT_KILL && roll < 10) {
                damage = enemy_hp;
                send_str(sock, RED "\nINSTANT KILL!\n" RESET);
            }
            
            enemy_hp -= damage;
            if(enemy_hp < 0) enemy_hp = 0;
            
            snprintf(buffer, sizeof(buffer), 
                "\nYou dealt %d damage!\n\n", damage);
            send_str(sock, buffer);

            if(enemy_hp == 0) {
                int reward = 50 + rand() % 101;
                ps->gold += reward;
                ps->kills++;
                
                snprintf(buffer, sizeof(buffer),
                    GREEN "=== REWARD ===\n"
                    "You earned %d gold!\n" RESET
                    RED "\n=== NEW ENEMY ===\n" RESET,
                    reward
                );
                send_str(sock, buffer);
                
                enemy_hp = 50 + rand() % 151;
                max_hp = enemy_hp;
            }
        }
        else {
            send_str(sock, RED "\nInvalid command! Use 'attack' or 'exit'.\n" RESET);
        }
    }
}

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    
    PlayerState ps = {
        .gold = 700,
        .base_damage = 5,
        .kills = 0,
        .inventory_count = 0,
        .equipped_weapon = shop[0] 
    };

    send_str(client_sock, "Welcome to the Dungeon Game!\n");

    while(1) {
        const char* menu = 
            BLUE "\n===== MAIN MENU =====\n" RESET
            "1. Show Player Stats\n"
            "2. Shop (Buy Weapons)\n"
            "3. View Inventory & Equip Weapons\n"
            "4. Battle Mode\n"
            "5. Exit Game\n"
            "Choose an option: ";
            
        send_str(client_sock, menu);
        
        int choice = receive_choice(client_sock);
        clear_input_buffer(client_sock);
        
        if(choice < 1 || choice > 5) {
            send_str(client_sock, RED "\nInvalid choice!\n" RESET);
            continue;
        }

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
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY };

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, MAX_CLIENTS);
    printf("Server listening on port %d...\n", PORT);

while (1) {
    int* client_sock = malloc(sizeof(int));
    if (!client_sock) {
        perror("malloc");
        continue;
    }

    *client_sock = accept(server_fd, NULL, NULL);
    if (*client_sock < 0) {
        perror("accept");
        free(client_sock);
        continue;
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, handle_client, client_sock) != 0) {
        perror("pthread_create");
        close(*client_sock);
        free(client_sock);
        continue;
    }

    pthread_detach(tid);
}

    close(server_fd);
    return 0;
}

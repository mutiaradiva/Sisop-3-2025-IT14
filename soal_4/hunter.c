#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>

#define MAX_HUNTERS 50
#define MAX_DUNGEONS 50

struct Hunter {
    char username[50];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int banned;
};

struct SystemData {
    struct Hunter hunters[MAX_HUNTERS];
    int num_hunters;
};

struct Dungeon {
    char name[100];
    int min_level;
    int atk_reward;
    int hp_reward;
    int def_reward;
    int exp_reward;
    long long key;
};

struct DungeonData {
    struct Dungeon dungeons[MAX_DUNGEONS];
    int num_dungeons;
};

int running_notification = 0;
pthread_t notification_thread;
struct DungeonData* global_d_data = NULL;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void* notification_loop(void* arg) {
    int index = 0;
    while (running_notification) {
        pthread_mutex_lock(&print_lock);
        printf("\033[s"); 
        printf("\033[2;1H%-60s", " "); 
        if (global_d_data == NULL || global_d_data->num_dungeons == 0) {
            printf("\033[2;1HNo active dungeons.");
        } else {
            struct Dungeon* d = &global_d_data->dungeons[index % global_d_data->num_dungeons];
            printf("\033[2;1H%s (Min. Lv %d)", d->name, d->min_level);
            index++;
        }
        printf("\033[u"); 
        fflush(stdout);
        pthread_mutex_unlock(&print_lock);
        sleep(3);
    }
    return NULL;
}

void start_notification(struct DungeonData* d_data) {
    if (!running_notification) {
        running_notification = 1;
        global_d_data = d_data;
        pthread_create(&notification_thread, NULL, notification_loop, NULL);
    }
}

void print_main_menu() {
    pthread_mutex_lock(&print_lock);
    printf("\n=== HUNTER SYSTEM ===\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. Exit\n");
    printf("Choice: ");
    pthread_mutex_unlock(&print_lock);
}

void print_hunter_menu(const char* username) {
    printf("\n=== %s's MENU ===\n", username);
    printf("1. Dungeon List\n");
    printf("2. Dungeon Raid\n");
    printf("3. Hunters Battle (PVP)\n");
    printf("4. Notification\n");
    printf("5. Exit\n");
    printf("Choice: ");
}

void view_available_dungeons(struct Dungeon* dungeons, int total, int level) {
    printf("\n=== AVAILABLE DUNGEONS ===\n");
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (strlen(dungeons[i].name) > 0 && dungeons[i].min_level <= level) {
            printf("%d. %s\t(Level %d+)\n", ++count, dungeons[i].name, dungeons[i].min_level);
        }
    }
    if (count == 0) {
        printf("No available dungeons for your level.\n");
    }
    printf("\nPress enter to continue...");
    getchar();
}

void raid_dungeon(struct Hunter* hunter, struct DungeonData* d_data) {
    struct Dungeon* available[MAX_DUNGEONS];
    int index_map[MAX_DUNGEONS];
    int count = 0;

    printf("\n=== RAIDABLE DUNGEONS ===\n");
    for (int i = 0; i < d_data->num_dungeons; i++) {
        if (d_data->dungeons[i].min_level <= hunter->level) {
            printf("%d. %s\t(Level %d+)\n", count + 1, d_data->dungeons[i].name, d_data->dungeons[i].min_level);
            available[count] = &d_data->dungeons[i];
            index_map[count] = i;
            count++;
        }
    }

    if (count == 0) {
        printf("No dungeons available for raid.\n");
        printf("Press enter to continue...");
        getchar();
        return;
    }

    int choice;
    printf("Choose Dungeon: ");
    scanf("%d", &choice);
    getchar();

    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        return;
    }

    struct Dungeon* selected = available[choice - 1];
    int sel_index = index_map[choice - 1];

    hunter->atk += selected->atk_reward;
    hunter->hp += selected->hp_reward;
    hunter->def += selected->def_reward;
    hunter->exp += selected->exp_reward;

    if (hunter->exp >= 500) {
        hunter->level++;
        hunter->exp = 0;
    }

    for (int i = sel_index; i < d_data->num_dungeons - 1; i++) {
        d_data->dungeons[i] = d_data->dungeons[i + 1];
    }
    d_data->num_dungeons--;

    printf("\nRaid success! Gained:\n");
    printf("ATK: %d\n", selected->atk_reward);
    printf("HP: %d\n", selected->hp_reward);
    printf("DEF: %d\n", selected->def_reward);
    printf("EXP: %d\n", selected->exp_reward);
    printf("\nPress enter to continue...");
    getchar();
}

void battle_hunter(struct Hunter* self, struct SystemData* data) {
    printf("\n=== PVP LIST ===\n");
    for (int i = 0; i < data->num_hunters; i++) {
        struct Hunter* h = &data->hunters[i];
        if (strcmp(h->username, self->username) != 0 && !h->banned) {
            printf("%s - Power: %d\n", h->username, h->atk + h->hp + h->def);
        }
    }

    char target_name[50];
    printf("Target: ");
    scanf("%s", target_name);
    getchar();

    struct Hunter* target = NULL;
    int index = -1;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, target_name) == 0) {
            target = &data->hunters[i];
            index = i;
            break;
        }
    }

    if (!target || target->banned || strcmp(self->username, target->username) == 0) {
        printf("Invalid target.\n");
        return;
    }

    int power_self = self->atk + self->hp + self->def;
    int power_target = target->atk + target->hp + target->def;

    if (power_self >= power_target) {
        self->atk += target->atk;
        self->hp += target->hp;
        self->def += target->def;
        for (int i = index; i < data->num_hunters - 1; i++) {
            data->hunters[i] = data->hunters[i + 1];
        }
        data->num_hunters--;
        printf("You won and absorbed %s's stats!\n", target->username);
    } else {
        target->atk += self->atk;
        target->hp += self->hp;
        target->def += self->def;
        for (int i = 0; i < data->num_hunters; i++) {
            if (strcmp(data->hunters[i].username, self->username) == 0) {
                for (int j = i; j < data->num_hunters - 1; j++) {
                    data->hunters[j] = data->hunters[j + 1];
                }
                data->num_hunters--;
                break;
            }
        }
        printf("You lost. You are removed from the system.\n");
        exit(0);
    }

    printf("\nPress enter to continue...");
    getchar();
}

// == Session Hunter ==
void hunter_session(struct Hunter* hunter, struct SystemData* data, struct DungeonData* d_data) {
    global_d_data = d_data;

    int choice;
    while (1) {
        pthread_mutex_lock(&print_lock);
        printf("\033[2J\033[H"); 
        printf("=== HUNTER SYSTEM ===\n");  
        pthread_mutex_unlock(&print_lock);

        print_hunter_menu(hunter->username);
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            view_available_dungeons(d_data->dungeons, d_data->num_dungeons, hunter->level);
        } else if (choice == 2) {
            raid_dungeon(hunter, d_data);
        } else if (choice == 3) {
            battle_hunter(hunter, data);
        } else if (choice == 4) {
            start_notification(d_data);
            printf("\n[Notification started. Press enter to return to menu...]\n");
            getchar();
        } else if (choice == 5) {
            running_notification = 0;
            pthread_join(notification_thread, NULL);
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
}

// == Main ==
int main() {
    key_t key_hunter = 1234;
    int shmid_hunter = shmget(key_hunter, sizeof(struct SystemData), 0666);
    if (shmid_hunter == -1) {
        perror("Shared memory for hunter not found. Please run system first.");
        exit(1);
    }
    struct SystemData* data = (struct SystemData*) shmat(shmid_hunter, NULL, 0);

    key_t key_dungeon = 5678;
    int shmid_dungeon = shmget(key_dungeon, sizeof(struct DungeonData), 0666);
    if (shmid_dungeon == -1) {
        perror("Shared memory for dungeon not found.");
        exit(1);
    }
    struct DungeonData* d_data = (struct DungeonData*) shmat(shmid_dungeon, NULL, 0);

    while (1) {
        int choice;
        char username[50];
        print_main_menu();
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            printf("Enter username to register: ");
            scanf("%s", username);
            getchar();

            int exists = 0;
            for (int i = 0; i < data->num_hunters; i++) {
                if (strcmp(data->hunters[i].username, username) == 0) {
                    exists = 1;
                    break;
                }
            }

            if (exists) {
                printf("Username already exists.\n");
            } else if (data->num_hunters >= MAX_HUNTERS) {
                printf("Max hunter limit reached.\n");
            } else {
                struct Hunter* h = &data->hunters[data->num_hunters++];
                strcpy(h->username, username);
                h->level = 1;
                h->exp = 0;
                h->atk = 10;
                h->hp = 100;
                h->def = 5;
                h->banned = 0;
                printf("Registration successful.\n");
            }
        } else if (choice == 2) {
            printf("Enter username to login: ");
            scanf("%s", username);
            getchar();

            int found = 0;
            for (int i = 0; i < data->num_hunters; i++) {
                if (strcmp(data->hunters[i].username, username) == 0) {
                    if (data->hunters[i].banned) {
                        printf("This account has been banned.\n");
                        found = 1;
                        break;
                    }
                    found = 1;
                    hunter_session(&data->hunters[i], data, d_data);
                    break;
                }
            }

            if (!found) {
                printf("Hunter not found.\n");
            }
        } else if (choice == 3) {
            printf("Exiting hunter system...\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }

    shmdt(data);
    shmdt(d_data);
    return 0;
}
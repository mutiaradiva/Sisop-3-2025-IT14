#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>

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

struct Dungeon {
    char name[100];
    int min_level;
    int atk_reward;
    int hp_reward;
    int def_reward;
    int exp_reward;
    unsigned long key;
};

struct SystemData {
    struct Hunter hunters[MAX_HUNTERS];
    int num_hunters;
};

struct DungeonData {
    struct Dungeon dungeons[MAX_DUNGEONS];
    int num_dungeons;
};

const char *dungeon_names[] = {
    "Double Dungeon", "Demon Castle", "Pyramid Dungeon", "Red Gate Dungeon",
    "Hunters Guild Dungeon", "Busan A-Rank Dungeon", "Insects Dungeon",
    "Goblins Dungeon", "D-Rank Dungeon", "Gwanak Mountain Dungeon",
    "Hapjeong Subway Station Dungeon"
};

int shmid_hunter, shmid_dungeon;
struct SystemData* data;
struct DungeonData* d_data;

void cleanup(int signal) {
    printf("\nCleaning up shared memory...\n");
    shmdt(data);
    shmdt(d_data);
    shmctl(shmid_hunter, IPC_RMID, NULL);
    shmctl(shmid_dungeon, IPC_RMID, NULL);
    exit(0);
}

void print_system_menu() {
    printf("\n=== SYSTEM MENU ===\n");
    printf("1. Hunter Info\n");
    printf("2. Dungeon Info\n");
    printf("3. Generate Dungeon\n");
    printf("4. Ban Hunter\n");
    printf("5. Reset Hunter\n");
    printf("6. Unbanned Hunter\n");
    printf("7. Exit\n");
    printf("Choice: ");
}

void display_hunter_info(struct SystemData* data) {
    printf("\n=== HUNTER INFO ===\n");
    for (int i = 0; i < data->num_hunters; i++) {
        struct Hunter* h = &data->hunters[i];
        printf("Name: %s\tLevel: %d\tEXP: %d\tATK: %d\tHP: %d\tDEF: %d\tStatus: %s\n",
               h->username, h->level, h->exp, h->atk, h->hp, h->def,
               h->banned ? "BANNED" : "ACTIVE");
    }
}

void ban_hunter(struct SystemData* data) {
    char name[50];
    printf("Enter hunter name to ban: ");
    scanf("%s", name);
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = 1;
            printf("Hunter '%s' has been banned.\n", name);
            return;
        }
    }
    printf("Hunter not found.\n");
}

void unban_hunter(struct SystemData* data) {
    char name[50];
    printf("Enter hunter name to unban: ");
    scanf("%s", name);
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = 0;
            printf("Hunter '%s' has been unbanned.\n", name);
            return;
        }
    }
    printf("Hunter not found.\n");
}

void reset_hunter(struct SystemData* data) {
    char name[50];
    printf("Enter hunter name to reset: ");
    scanf("%s", name);
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].level = 1;
            data->hunters[i].exp = 0;
            data->hunters[i].atk = 10;
            data->hunters[i].hp = 100;
            data->hunters[i].def = 5;
            data->hunters[i].banned = 0;
            printf("Hunter '%s' has been reset.\n", name);
            return;
        }
    }
    printf("Hunter not found.\n");
}

unsigned long generate_key() {
    return (unsigned long) time(NULL) ^ (rand() << 16) ^ rand();
}

void generate_dungeon(struct DungeonData* d_data) {
    if (d_data->num_dungeons >= MAX_DUNGEONS) {
        printf("Dungeon limit reached.\n");
        return;
    }

    struct Dungeon* d = &d_data->dungeons[d_data->num_dungeons];

    strcpy(d->name, dungeon_names[rand() % (sizeof(dungeon_names) / sizeof(dungeon_names[0]))]);
    d->min_level = rand() % 5 + 1;
    d->atk_reward = rand() % 51 + 100;
    d->hp_reward = rand() % 51 + 50;
    d->def_reward = rand() % 26 + 25;
    d->exp_reward = rand() % 151 + 150;
    d->key = generate_key();

    d_data->num_dungeons++;

    printf("\nDungeon generated!\n");
    printf("Name: %s\n", d->name);
    printf("Minimum Level: %d\n", d->min_level);
    printf("ATK: %d\nHP: %d\nDEF: %d\nEXP: %d\nKEY: %lu\n",
           d->atk_reward, d->hp_reward, d->def_reward, d->exp_reward, d->key);
}

void display_dungeon_info(struct DungeonData* d_data) {
    printf("\n=== DUNGEON INFO ===\n");
    if (d_data->num_dungeons == 0) {
        printf("No dungeons available.\n");
        return;
    }

    for (int i = 0; i < d_data->num_dungeons; i++) {
        struct Dungeon* d = &d_data->dungeons[i];
        printf("[Dungeon %d]\n", i + 1);
        printf("Name: %s\n", d->name);
        printf("Minimum Level: %d\n", d->min_level);
        printf("EXP Reward: %d\n", d->exp_reward);
        printf("ATK: %d\nHP: %d\nDEF: %d\nKEY: %lu\n\n",
               d->atk_reward, d->hp_reward, d->def_reward, d->key);
    }
}

int main() {
    srand(time(NULL));
    signal(SIGINT, cleanup); 

    key_t key_hunter = 1234;
    shmid_hunter = shmget(key_hunter, sizeof(struct SystemData), IPC_CREAT | 0666);
    data = (struct SystemData*) shmat(shmid_hunter, NULL, 0);
    if (data->num_hunters < 0 || data->num_hunters > MAX_HUNTERS) {
        data->num_hunters = 0;
    }

    key_t key_dungeon = 5678;
    shmid_dungeon = shmget(key_dungeon, sizeof(struct DungeonData), IPC_CREAT | 0666);
    d_data = (struct DungeonData*) shmat(shmid_dungeon, NULL, 0);
    if (d_data->num_dungeons < 0 || d_data->num_dungeons > MAX_DUNGEONS) {
        d_data->num_dungeons = 0;
    }

    int choice;
    while (1) {
        print_system_menu();
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: display_hunter_info(data); break;
            case 2: display_dungeon_info(d_data); break;
            case 3: generate_dungeon(d_data); break;
            case 4: ban_hunter(data); break;
            case 5: reset_hunter(data); break;
            case 6: unban_hunter(data); break;
            case 7:
                printf("Exiting system...\n");
                cleanup(0);  
                break;
            default: printf("Invalid choice.\n"); break;
        }
        printf("\n");
    }
}
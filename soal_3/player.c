#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
    };

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    while(1) {
        char buffer[1024];
        int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (bytes <= 0) {
            printf("Connection closed\n");
            break;
        }
        buffer[bytes] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Choose an option:") || strstr(buffer, "Enter")) {
            int choice;
            scanf("%d", &choice);
            send(sock, &choice, sizeof(choice), 0);
        } 
        else if (strstr(buffer, "Type 'attack'") || strstr(buffer, "exit")) {
            char cmd[16];
            scanf("%s", cmd);
            strcat(cmd, "\n");
            send(sock, cmd, strlen(cmd), 0);
        }
    }

    close(sock);
    return 0;
}

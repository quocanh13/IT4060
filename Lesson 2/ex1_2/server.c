#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// gcc server.c -o server && ./server

int main() {
    char *server_ip = "127.0.0.1";
    int server_port = 5000;
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr); 

    int server = socket(AF_INET, SOCK_STREAM, 0);
    bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if(listen(server, 10) < 0){
        printf("Failed to run server\n");
    } else {
        printf("Server is listening\n");
        while(1){
            int client = accept(server, NULL, NULL);
            char *message = (char*)malloc(sizeof(char)*1), buffer[1000], *temp;
            unsigned int message_len = 0;
            message[0] = '\0';
            while(1){
                int len = recv(client, buffer, sizeof(buffer) - 1, 0);
                if(len <= 0) break;
                temp = message;
                message_len += len;
                message = (char*)malloc(sizeof(char)*(message_len + 1));
                buffer[len] = '\0';
                strcat(message, temp);
                strcat(message, buffer);
            }
            printf("%s\n", message);
            int count = 0;
            for(int i = 0; i < message_len; i++) {
                if(message[i] - 48 == 1) {
                    int check = 0;
                    for(int j = 2; j <= 9; j++) {
                        if(message[i + j - 1] - 48 == j) {
                            if(j == 9) count++;
                        } else {
                            i += j - 1;
                            check = 1;
                        }
                        if(check) break;
                    }
                }
            }
            printf("%d\n", count);
            close(client);
        }
    }
}
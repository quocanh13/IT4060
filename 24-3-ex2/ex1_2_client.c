#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// gcc ex1_2_client.c -o ex1_2_client && ./ex1_2_client

int main() {
    char *client_ip = "127.0.0.1";
    int client_port = 5100;
    struct sockaddr_in client_addr = {0};

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    inet_pton(AF_INET, client_ip, &client_addr.sin_addr);

    char *server_ip = "127.0.0.1";
    int server_port = 5000;
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr); 

    int client = socket(AF_INET, SOCK_STREAM, 0);
    bind(client, (struct sockaddr*)&client_addr, sizeof(client_addr));

    if(connect(client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to connect to server\n");
    } else {
        printf("Connected to server\n");
        char messsage[4][100] = {
            "SOICTSOICT012345678901234567890123456789012345",
            "6789SOICTSOICTSOICT012345678901234567890123456",
            "7890123456789012345678901234567890123456789012",
            "3456789SOICTSOICT01234567890123456789012345678"
        };
        for(int i = 0; i < 4; i++){
            send(client, messsage[i], strlen(messsage[i]), 0);
        } 
        close(client);
    }
    
}
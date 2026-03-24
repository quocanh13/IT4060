#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// gcc ex1_3_client.c -o ex1_3_client && ./ex1_3_client

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

    int client = socket(AF_INET, SOCK_DGRAM, 0);
    bind(client, (struct sockaddr*)&client_addr, sizeof(client_addr));
    char *buffer = "Hello";
    sendto(client, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    char message[1000];
    int len = recvfrom(client, message, sizeof(message), 0, NULL, NULL);
    message[len] = '\0';
    printf("%s\n", message);
}
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

    int server = socket(AF_INET, SOCK_DGRAM, 0);
    bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));

    struct sockaddr_in client_addr = {0};
    int client_addr_size;
    char buffer[2000];
    while(1){
        int len = recvfrom(server, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_size);
        buffer[len] = '\0';
        char *client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        printf("%s\n", buffer);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//gcc tcp_client.c -o tcp_client && ./tcp_client 127.0.0.1 5000
int main(int argc, char** args){
    struct sockaddr_in client_addr = {0};
    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);
    client_addr.sin_port = htons(5001);

    char* server_ip_addr = args[1];
    int server_port = atoi(args[2]);

    struct sockaddr_in server_addr_in = {0};
    server_addr_in.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip_addr, &server_addr_in.sin_addr);
    server_addr_in.sin_port = htons(server_port);
    struct sockaddr* server_addr = (struct sockaddr*) &server_addr_in;

    int client = socket(AF_INET, SOCK_STREAM, 0);
    bind(client, (struct sockaddr*) &client_addr, sizeof(client_addr));

    if(connect(client, server_addr, sizeof(server_addr_in)) != 0) {
        printf("Failed to connect to server\n");
    } else {
        printf("Connected to server\n");
        char message[1000];
        char buffer[1000];
        int len = recv(client, buffer, sizeof(buffer) - 1, 0);
        buffer[len] = '\0';
        
        printf("%s\n", buffer);
        scanf("%s", message);
        send(client, message, strlen(message), 0);
    }
}
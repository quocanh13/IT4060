#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//gcc tcp_server.c -o tcp_server && ./tcp_server 5000 hello_message.txt client_message.txt 
int main(int argc, char** args){
    
    FILE* helo_file = fopen(args[2], "r");
    fseek(helo_file, 0, SEEK_END);
    long size = ftell(helo_file);
    char hello_message[size + 1];
    rewind(helo_file);
    fread(hello_message, 1, size, helo_file);
    hello_message[size] = '\0';

    FILE* recv_message_file = fopen(args[3], "w");

    int server_port = atoi(args[1]);

    struct sockaddr_in server_addr_in = {0};
    server_addr_in.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_addr_in.sin_addr);
    server_addr_in.sin_port = htons(server_port);

    struct sockaddr* server_addr = (struct sockaddr*) &server_addr_in;

    int server = socket(AF_INET, SOCK_STREAM, 0);
    bind(server, server_addr, sizeof(*server_addr));

    if(listen(server, 10) != 0) {
        printf("Server Error");
    }
    printf("Server is listening\n");
    while(1) {
        char buffer[1024] = {0};
        int client = accept(server, NULL, NULL);

        send(client, hello_message, sizeof(hello_message), 0);
        while(1) {
            int len = recv(client, buffer, sizeof(buffer), 0);
            if(len > 0) {
                buffer[len] = '\n';
                fwrite(buffer, 1, len + 1, recv_message_file);
                fflush(recv_message_file);
            } else {
                break;
            }
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

// gcc 28-4-ex1-server.c -o 28-4-ex1-server && ./28-4-ex1-server

void add_client(int client);
void close_client(int client, int i);
char* encode(char *msg);
void broadcast(char* msg, int fd);

struct pollfd clients[1000] = {0};
nfds_t nfds = 0;
int server;

int main(){
    char *server_ip = "127.0.0.1";
    int server_port = 5000;
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr); 

    server = socket(AF_INET, SOCK_STREAM, 0);

    if(server < 0){
        printf("Failed to create server socket\n");
        exit(0);
    }
    if(bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to bind server\n");
        exit(0);
    }
    if(listen(server, 10) < 0) {
        printf("Failed to listen\n");
        exit(0);
    }
    printf("Server is listening\n");

    clients[0].fd = server;
    clients[0].events = POLLIN;
    clients[0].revents = 0;
    char client_message[1000];
    nfds++;
    struct pollfd pfd;
    while(1){
        int num_events = poll(clients, nfds, -1);
        for(int i = 0; i < nfds; i++){
            if(!clients[i].revents) continue;

            pfd = clients[i];
            if(pfd.fd == server) {
                int client = accept(server, NULL, NULL);
                add_client(client);
                continue;
            } 
            int fd = pfd.fd;
            int len = recv(fd, client_message, sizeof(client_message) - 1, 0);
            if(len <= 0) {
                close_client(fd, i);
                continue;
            }

            client_message[len - 1] = '\0';
            if(!strcmp(client_message, "exit")) {
                close_client(fd, i);
            } else {
                char* encode_msg = encode(client_message);
                send(fd, encode_msg, strlen(encode_msg), 0);
            }
        }
    }
}

void add_client(int client){
    clients[nfds].fd = client;
    clients[nfds].events = POLLIN;
    clients[nfds].revents = 0;
    char message[100];
    sprintf(message, "Xin Chao, Hien Co %lu Client Dang Ket Noi\n", nfds - 1);
    nfds++;
    send(client, message, strlen(message), 0);
}

void close_client(int client, int i){
    char message[100];
    sprintf(message, "Tam Biet Client\n");
    send(client, message, strlen(message), 0);
    close(client);
    clients[i] = clients[nfds - 1];
    nfds--;
    i--;
}

char* encode(char *msg){
    int len = strlen(msg);
    char *nmsg = malloc((len + 2)*sizeof(char));
    for(int i = 0; i < len; i++) {
        if(msg[i] == '9'){
            nmsg[i] = 'a';
        } else if(msg[i] == 'z'){
            nmsg[i] = 'a';
        } else if(msg[i] == 'Z'){
            nmsg[i] = 'A';
        } else if((msg[i] < 'z' && msg[i] >= 'a') || (msg[i] < 'Z' && msg[i] >= 'A')) {
            nmsg[i] = msg[i] + 1;
        } else {
            nmsg[i] = msg[i];
        }
    }
    nmsg[len] = '\n';
    nmsg[len + 1] = '\0';
    return nmsg;
}

void broadcast(char* msg, int fd){
    for(int k = 0; k < nfds; k++){
        int t = clients[k].fd;
        if(t != server && t != fd) 
            send(t, msg, strlen(msg), 0);
    }
}
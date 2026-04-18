#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

// gcc 14-4-ex1-server.c -o 14-4-ex1-server && ./14-4-ex1-server

void add_client(int client);
int check_name(char name[], int len, int client);

struct pollfd clients[1000] = {0};
nfds_t nfds = 0;
int max_fd;
int server;
int has_name[1000] = {0};
char client_names[1000][100];

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
                printf("Add client");
                int client = accept(server, NULL, NULL);
                add_client(client);
                continue;
            } 
            int fd = pfd.fd;
            int len = recv(fd, client_message, sizeof(client_message) - 1, 0);
            if(len <= 0) {
                close(fd);
                clients[i] = clients[nfds - 1];
                nfds--;
                i--;
                has_name[fd] = 0;
                continue;
            }

            if(!has_name[fd]){
                has_name[fd] = check_name(client_message, len, fd);
                continue;
            }

            client_message[len - 1] = '\0';
            char broadcast_message[1200];
            sprintf(broadcast_message, "%s: %s\n", client_names[fd], client_message);
            for(int k = 0; k < nfds; k++){
                int t = clients[k].fd;
                if(t != server && t != fd && has_name[t]) 
                    send(t, broadcast_message, strlen(broadcast_message), 0);
            }
        }
    }
}

void add_client(int client){
    if(client > max_fd) max_fd = client;
    clients[nfds].fd = client;
    clients[nfds].events = POLLIN;
    clients[nfds].revents = 0;
    nfds++;
    char *message = "Enter your name in the following format: client_id: client_name\n";
    send(client, message, strlen(message), 0);
}

int check_name(char name[], int len, int client){
    name[len - 1] = '\0';
    char *tok = strtok(name, " ");
    if(tok == NULL || strcmp(tok, "client_id:")) {
        char *message = "Invalid format\nTry again with the following format: client_id: client_name\n";
        send(client, message, strlen(message), 0);
        return 0;
    }
    tok = strtok(NULL, "");
    strncpy(client_names[client], tok, 99);
    return 1;
}
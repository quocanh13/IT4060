#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
// gcc 28-4-ex3-server.c -o 28-4-ex3-server && ./28-4-ex3-server

void add_client(int client);
void close_client(int client, int i);
void handle(char* msg, int client);

struct pollfd clients[1000] = {0};
nfds_t nfds = 0;
char client_topic[1000][100][50] = {0};
int client_topic_len[1000] = {0};
int server = 0;

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
            clients[i].revents = 0;
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
                i--;
                continue;
            }

            client_message[len - 1] = '\0';
            handle(client_message, fd);
        }
    }
}

void add_client(int client){
    clients[nfds].fd = client;
    clients[nfds].events = POLLIN;
    clients[nfds].revents = 0;
    char message[100];
    nfds++;
}

void close_client(int client, int i){
    char message[100];
    close(client);
    clients[i] = clients[nfds - 1];
    nfds--;
}

void handle(char* msg, int client){
    char* part = strtok(msg, " ");
    if(!strcmp(part, "SUB")){
        part = strtok(NULL, " ");
        int index = client_topic_len[client];
        strcpy(client_topic[client][index], part); 
        client_topic_len[client]++;
    } else if(!strcmp(part, "UNSUB")) {
        part = strtok(NULL, " ");
        int len = client_topic_len[client];
        for(int i = 0; i <  client_topic_len[client]; i++) {
            if(!strcmp(client_topic[client][i], part)){
                strcpy(client_topic[client][i], client_topic[client][len - 1]); 
                client_topic_len[client]--;
                break;
            }
        }
    } else {
        char *topic = strtok(NULL, " ");
        char *msg = strtok(NULL, "");
        char send_msg[1000];
        sprintf(send_msg, "%s\n", msg);
        for(int k = 0; k < nfds; k++){
            int t = clients[k].fd;
            if(t != server && t != client){
                for(int tp = 0; tp < client_topic_len[t]; tp++){
                    if(!strcmp(topic, client_topic[t][tp])){
                        sendto(t, send_msg, strlen(send_msg), 0, NULL, 0);
                    }
                }
            }
        }
    }
}
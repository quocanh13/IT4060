#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
// gcc 19-5-ex3-server.c -o 19-5-ex3-server && ./19-5-ex3-server
void* create_thread(void* args);
int check_name(char name[], int len, int client);

int max_fd;
int server;
char client_names[1000][100] = {0};

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

    while(1){
        int client = accept(server, NULL, NULL);
        char *message = "Enter your name in the following format: client_id: client_name\n";
        send(client, message, strlen(message), 0);

        pthread_t t;

        int *pclient = malloc(sizeof(int));

        *pclient = client;

        pthread_create(&t, NULL, create_thread, pclient);

        pthread_detach(t);
    }
}

void* create_thread(void* args){
    int client = *(int*)args;

    while(1){
        char msg[1000];
        int len = recv(client, msg, 999, 0);
        if(len <= 0){
            close(client);
            strcpy(client_names[client], "");
            return NULL;
        }

        if(strcmp(client_names[client], "") == 0){
            check_name(msg, len, client);
            continue;
        }
        msg[len] = '\0';
        char final_message[2000];
        sprintf(final_message, "%s: %s", client_names[client], msg);
        for(int i = 0; i < 1000; i++){
            if(strcmp(client_names[i], "") && strcmp(client_names[i], client_names[client])){
                send(i, final_message, strlen(final_message), 0);
            }
        }
    }
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
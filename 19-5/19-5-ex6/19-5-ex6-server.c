#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
// gcc 19-5-ex6-server.c -o 19-5-ex6-server && ./19-5-ex6-server
void* thread();
int client;
int server;

int main(){
    signal(SIGCHLD, SIG_IGN);

    char *server_ip = "127.0.0.1";
    int server_port = 5001;
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

    int num_thread = 8;
    char buf[256];
    pthread_t thread_id;
    for(int i = 0; i <  num_thread; i++) {
        pthread_create(&thread_id, NULL, thread, NULL);
        pthread_detach(thread_id);
    }
    pause();
}

void* thread(){
    while (1){
        char buf[256];
        int client = accept(server, NULL, NULL);
        printf("New client connected: %d\n", client);
        int ret = recv(client, buf, sizeof(buf), 0);
        buf[ret] = 0;
        puts(buf);
        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
        send(client, msg, strlen(msg), 0);
        close(client);
    }
}
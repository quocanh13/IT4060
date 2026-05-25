#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
// gcc 19-5-ex5-server.c -o 19-5-ex5-server && ./19-5-ex5-server

void* thread(void *args);

int main(){
    char *server_ip = "127.0.0.1";
    int server_port = 5000;
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    int server = socket(AF_INET, SOCK_STREAM, 0);
 
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

    while (1)
    {
        int client = accept(server, NULL, NULL);
        pthread_t thread_id;
        int *client_arg = malloc(sizeof(int));
        *client_arg = client;
        pthread_create(&thread_id, NULL, thread, client_arg);
        pthread_detach(thread_id);
    }   
}

void* thread(void *args){
    int client = *(int*)args;
    char buf[100];
    while (1){
        int len = recv(client, buf, sizeof(buf) - 1, 0);
        if(len <= 0) {
            close(client);
            return NULL;
        }
        
        buf[len-1] = '\0';
        char str_time[20] = "";
        char time_part[3][5] = {0};
        char *part = strtok(buf, " ");
        if(strcmp(part, "GET_TIME") != 0){
            send(client, "Invalid Format\n", 16, 0);
            continue;
        }

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        int day   = t->tm_mday;
        int month = t->tm_mon + 1;
        int year  = t->tm_year + 1900;
        int i = 0;
        for(i; i < 3; i++) {
            part = strtok(NULL, "/");
            if(strcmp(part, "dd") == 0){
                sprintf(time_part[i], "%02i", day);
            } else if(strcmp(part, "mm") == 0) {
                sprintf(time_part[i], "%02i", month);
            } else if(strcmp(part, "yyyy") == 0) {
                sprintf(time_part[i], "%i", year);
            } else {
                break;
            }
        }
        part = strtok(NULL, "/");
        if(i == 3 && part == NULL) {
            sprintf(str_time, "%s/%s/%s\n", time_part[0], time_part[1], time_part[2]);
            send(client, str_time, strlen(str_time), 0);
        } else {
            send(client, "Invalid Format\n", 16, 0);
        }
    }
}
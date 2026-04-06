#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#define MAX_EVENTS 100
#define MAX_FD 10000
// gcc 31-3-ex1-server.c -o 31-3-ex1-server && ./31-3-ex1-server

struct student{
    char name[200];
    char id[10];
};
struct student clients[MAX_FD];

int accept_client(int server, int epfd){
    int client = accept(server, NULL, NULL);
    if(client < 0) printf("There is an error when client connects\n");
    else {
        struct epoll_event epev = {0};
        epev.events = EPOLLIN;
        epev.data.fd = client;
        fcntl(client, F_SETFL, O_NONBLOCK);
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, client, &epev) < 0) {
            printf("Failed to add client %d to epfd\n", client);
        }
        strcpy(clients[client].name, "");
        strcpy(clients[client].id, "");
    }
    return client;
}

char* create_email(char name[], char id[]) {
    static char res[100];
    char name_copy[100];
    strcpy(name_copy, name); 

    char *parts[10];
    int count = 0;
    
    char *token = strtok(name_copy, " ");
    while (token != NULL) {
        parts[count++] = token;
        token = strtok(NULL, " ");
    }

    strcpy(res, parts[count - 1]);
    strcat(res, ".");

    for (int i = 0; i < count - 1; i++) {
        char first_letter[2] = {tolower(parts[i][0]), '\0'};
        strcat(res, first_letter);
    }

    strcat(res, id);
    strcat(res, "@sis.hust.edu.vn");

    for (int i = 0; res[i]; i++) {
        res[i] = tolower(res[i]);
    }

    return res;
}

void process_client(int client, int epfd){
    char buffer[100];
    int len = recv(client, buffer, sizeof(buffer), 0);
    if(len <= 0) {
        printf("There is an error when client send message\n");
        epoll_ctl(epfd, EPOLL_CTL_DEL, client, NULL);
        close(client);
        return ;
    }
    buffer[len-1] = '\0';
    if(!strcmp(clients[client].name, "")) {
        strcpy(clients[client].name, buffer);
        send(client, "MSSV: ", 6, 0);
    } else if(!strcmp(clients[client].id, "")) {
        strcpy(clients[client].id, buffer);
        char* email = create_email(clients[client].name, clients[client].id);
        send(client, email, strlen(email), 0);
        epoll_ctl(epfd, EPOLL_CTL_DEL, client, NULL);
        close(client);
    }
}

int main(){
    char server_ip[] = "127.0.0.1";
    int server_port = 6000;
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if(server < 0) {
        printf("Failed to create server socket\n");
        return 1;
    }

    if(bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to bind server\n");
        return 1;
    }

    if(listen(server, 10) < 0) {
        printf("Server failed to listen\n");
        return 1;
    }

    printf("Server is listening\n");

    if(fcntl(server, F_SETFL, O_NONBLOCK) < 0) {
        printf("Server failed to be async\n");
        return 1;
    }

    int epfd = epoll_create1(0);
    struct epoll_event epev = {0};
    epev.events = EPOLLIN;
    epev.data.fd = server;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, server, &epev) < 0) {
        printf("Failed to add server to epfd\n");
        return 1;
    }
    
    struct epoll_event events[MAX_EVENTS];
    while (1){
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

        if(nfds < 0) {
            printf("There is an error while waiting\n");
            return 1;
        } 
        for(int i = 0; i < nfds; i++) {
            if(events[i].data.fd == server){
                int client = accept_client(server, epfd);
                if(client >= 0) send(client, "Name: ", 6, 0);
                continue;
            }
            process_client(events[i].data.fd, epfd);
        }
    }
    
}


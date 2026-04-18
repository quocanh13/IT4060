#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

// gcc 14-4-ex2-server.c -o 14-4-ex2-server && ./14-4-ex2-server
void add_client(int client);
int check_account(char name[], int len, int client);
void execute_command(char command[], int len, int client);

struct pollfd clients[1000];
nfds_t nfds = 0;
int max_fd;
int server;
int verified[1000] = {0};

int main(){
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

    clients[0].fd = server;
    clients[0].events = POLLIN;
    clients[0].revents = 0;
    nfds++;
    char client_command[1000];

    while(1){
        int num_ev = poll(clients, nfds, -1);
        for(int i = 0; i < nfds; i++) {
            if(!clients[i].revents) continue;
            clients[i].revents = 0;
            int fd = clients[i].fd;

            if(fd == server) {
                int client = accept(server, NULL, NULL);
                add_client(client);
                continue;
            } 

            int len = recv(fd, client_command, sizeof(client_command) - 1, 0);
            if(len <= 0) {
                close(fd);
                clients[i] = clients[nfds - 1];
                nfds--;
                verified[fd] = 0;
                continue;
            }

            if(!verified[fd]){
                verified[fd] = check_account(client_command, len, fd);
                continue;
            }

            client_command[len - 1] = '\0';
            execute_command(client_command, len, fd);
        }
    }
}

void add_client(int client){
    if(client > max_fd) max_fd = client;
    clients[nfds].fd = client;
    clients[nfds].events = POLLIN;
    clients[nfds].revents = 0;
    nfds++;
    char *message = "Enter username and password\n";
    send(client, message, strlen(message), 0);
}

int check_account(char name[], int len, int client){
    name[len - 1] = '\0';
    char* client_username, *client_password;
    client_username = strtok(name, " ");
    client_password = strtok(NULL, " ");
    FILE *fptr;
    char buffer[1000];

    fptr = fopen("./account.txt", "r");
    
    if (fptr == NULL) {
        char message[] = "Could not open file, try later\n";
        send(client, message, strlen(message), 0);
        return 0;
    }

    char *username, *password;
    if(client_password != NULL){
        while (fgets(buffer, 255, fptr)) {
            username = strtok(buffer, " \n");
            password = strtok(NULL, " \n");
            if(!strcmp(username, client_username) && !strcmp(password, client_password) ){
                char message[] = "Logged in sucessfully\n";
                send(client, message, strlen(message), 0);
                fclose(fptr);
                return 1;
            }
        }
    }

    char message[] = "Invalid username and password, try again\n";
    send(client, message, strlen(message), 0);
    
    fclose(fptr);
    return 0;
}

void execute_command(char command[], int len, int client){
    command[len - 1] = '\0';
    char cmd[1000];
    sprintf(cmd, "%s > o.txt", command);
    system(cmd);

    FILE *fptr;
    char buffer[2000];
    fptr = fopen("./o.txt", "r");
    if (fptr == NULL) {
        char message[] = "Could not open file, try later\n";
        send(client, message, strlen(message), 0);
    }

    while (fgets(buffer, 255, fptr)){
        send(client, buffer, strlen(buffer), 0);
    }
    fclose(fptr);
}
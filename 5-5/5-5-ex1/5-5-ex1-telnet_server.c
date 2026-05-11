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

// gcc 5-5-ex1-telnet_server.c -o 5-5-ex1-telnet_server && ./5-5-ex1-telnet_server
int check_account(char name[], int len);
void execute_command(char command[], int len);

int client;
int server;
int verified = 0;

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
        client = accept(server, NULL, NULL);
        if(fork() != 0){
            close(client);
            continue;
        }
        close(server);
        char *message = "Enter username and password\n";
        send(client, message, strlen(message), 0);

        char client_command[1000];
        while (1){
            int len = recv(client, client_command, sizeof(client_command) - 1, 0);

            if(len <= 0) {
                close(client);
                signal(SIGCHLD, SIG_IGN);
                return 0;
            }

            if(!verified){
                verified = check_account(client_command, len);
                continue;
            }

            client_command[len - 1] = '\0';
            execute_command(client_command, len);
        }
    }
}

int check_account(char name[], int len){
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
    while (fgets(buffer, 255, fptr)) {
        username = strtok(buffer, " \n\r");
        password = strtok(NULL, " \n\r");
        if(!strcmp(username, client_username) && !strcmp(password, client_password)){
            char message[] = "Logged in sucessfully\n";
            send(client, message, strlen(message), 0);
            fclose(fptr);
            return 1;
        }
    }

    char message[] = "Invalid username and password, try again\n";
    send(client, message, strlen(message), 0);
    
    fclose(fptr);
    return 0;
}

void execute_command(char command[], int len){
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
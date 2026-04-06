#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//gcc sv_client.c -o sv_client && ./sv_client 127.0.0.1 5000
int main(int argc, char** args){
    char* server_ip = args[1];
    int server_port = atoi(args[2]);
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    char* client_ip = "127.0.0.1";
    int client_port = 5001;
    struct sockaddr_in client_addr = {0};

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    inet_pton(AF_INET, client_ip, &client_addr.sin_addr);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr));

    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
        printf("Failed to connect to the server !!!\n");
    } else {
        printf("Connected to the server\n");

        char name[100], birthday[100], message[500];
        int student_id;
        float score;

        printf("Name : ");
        scanf(" %[^\n]", name);
        printf("Student id : ");
        scanf("%d", &student_id);
        printf("Birthday : ");
        scanf("%s", birthday);
        printf("Score : ");
        scanf("%f", &score);
        sprintf(message, "%d %s %s %.2f", student_id, name, birthday, score);

        send(client_socket, message, strlen(message), 0);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
// gcc sv_server.c -o sv_server && ./sv_server 5000 sv_log.txt
int main(int argc, char** args){
    char* server_ip = "127.0.0.1";
    int server_port = atoi(args[1]);
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    bind(server, (struct sockaddr*) &server_addr, sizeof(server_addr));

    if( listen(server, 10) != 0){
        printf("Failed to run server !!!\n");

    } else {
        printf("Server is listening\n");
        char *file_path = args[2];
        FILE *file = fopen(file_path, "w");
        if(file == NULL){
            printf("Failed to open file");
        } else {
            while (1)
            {   
                int client = accept(server, NULL, NULL);
                char buffer[1000];
                int len = recv(client, buffer, sizeof(buffer), 0);
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                char head[100];

                sprintf(head, "127.0.0.1 %02d-%02d-%02d ", t->tm_hour, t->tm_min, t->tm_sec); 
                fwrite(head, sizeof(char), strlen(head), file);

                buffer[len] = '\n';
                fwrite(buffer, sizeof(char), len + 1, file);
                fflush(file);
                close(client);
            }
        }
    }
}
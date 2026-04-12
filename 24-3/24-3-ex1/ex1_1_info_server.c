#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// gcc ex1_1_info_server.c -o ex1_1_info_server && ./ex1_1_info_server
int main() {

    char* server_ip = "127.0.0.1";
    int server_port = 5000;
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
        while(1){
            int client = accept(server, NULL, NULL);
            size_t cwd_len;
            recv(client, &cwd_len, sizeof(size_t), 0);

            char cwd[1000];
            recv(client, cwd, cwd_len, 0);
            cwd[cwd_len] = '\0';

            printf("%s\n", cwd);
            while(1){
                int name_len;
                int ret = recv(client, &name_len, sizeof(int), 0);
                if (ret <= 0) break;
                
                char name[256];
                recv(client, name, name_len, 0);
                name[name_len] = '\0';

                long size;
                int size_len = recv(client, &size, sizeof(long), 0);
                if(name_len <= 0 || size_len <= 0) break;

                printf("%s - %ld bytes\n", name, size);
            }
            close(client);
        }
    }
}
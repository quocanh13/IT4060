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

// gcc info_client.c -o info_client && ./info_client
int main() {

    char* server_ip = "127.0.0.1";
    int server_port = 5000;
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
        char cwd[1000];
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            DIR* d;
            struct dirent *dir;
            struct stat st;
            d = opendir(".");
            if(d) {
                size_t cwd_len = strlen(cwd);
                send(client_socket, &cwd_len, sizeof(size_t), 0);
                send(client_socket, cwd, strlen(cwd), 0);
                
                while ((dir = readdir(d)) != NULL) {
                    if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                        continue;
                    stat(dir->d_name, &st);
                    int name_len = strlen(dir->d_name);

                    send(client_socket, &name_len, sizeof(int), 0);
                    send(client_socket, dir->d_name, name_len, 0);
                    send(client_socket, &st.st_size, sizeof(long), 0);
                }
                closedir(d);
            }
        }
    }
    close(client_socket);
}
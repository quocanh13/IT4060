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
#include <dirent.h>

// gcc 19-5-ex1-server.c -o 19-5-ex1-server && ./19-5-ex1-server
char* get_files_name();
int get_file(char *file_name, char *content);

int client;
int server;

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
        char *message = get_files_name();
        send(client, message, strlen(message), 0);
        if(strcmp(message, "ERROR No files to download \r\n") == 0){
            printf("Close");
            close(client);
            signal(SIGCHLD, SIG_IGN);
            return 0;
        }
        int file_size;
        char file_name[50];
        char content[2000];
        char msg[20];
        while (1){
            int len = recv(client, file_name, sizeof(file_name) - 1, 0);

            if(len <= 0) {
                close(client);
                signal(SIGCHLD, SIG_IGN);
                return 0;
            }
            
            file_name[len - 1] = '\0';
            file_size = get_file(file_name, content);
            if(file_size){
                sprintf(msg, "OK %d\r\n", file_size);
                send(client, msg, strlen(msg), 0);
                send(client, content, strlen(content), 0);
            } else {
                send(client, "There is no such file, try again\n", strlen("There is no such file, try again\n"), 0);
            }
        }
    }
}

char* get_files_name() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir("./files");
    if (dir == NULL) {
        char *err = malloc(32);
        sprintf(err, "OK 0\r\n\r\n");
        return err;
    }

    char *result = malloc(1000);
    result[0] = '\0';

    char files[256][256];
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        strcpy(files[count], entry->d_name);
        count++;
    }

    closedir(dir);
    snprintf(result, 1000, "OK %d\r\n", count);

    for (int i = 0; i < count; i++) {
        strcat(result, files[i]);
        strcat(result, "\r\n");
    }

    strcat(result, "\r\n");
    if(count == 0){
        return "ERROR No files to download \r\n";
    }
    return result;
}

int get_file(char *file_name, char *content) {
    char path[300];

    sprintf(path, "./files/%s", file_name);

    FILE *fp = fopen(path, "rb");

    if (fp == NULL) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);

    fread(content, 1, size, fp);

    content[size] = '\0';

    fclose(fp);

    return size;
}
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

// gcc 26-5-ex1-server.c -o 26-5-ex1-server && ./26-5-ex1-server

int client;
int server;

char** get_body(char* request);
char* get_response(int operand_1, int operand_2, char operator, int res);

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
        char request[1000];
        int len = recv(client, request, 999, 0);
        if(len <= 0){
            close(client);
            continue;
        }
        request[len] = '\0';

        if (strstr(request, "GET /favicon.ico")) {
            char *response =
                "HTTP/1.1 204 No Content\r\n"
                "\r\n";

            send(client, response, strlen(response), 0);
            close(client);
            continue;
        }

        char** data = get_body(request);
        if(data == NULL){
            close(client);
            continue;
        }

        int operand_1 = atoi(data[1]), operand_2 = atoi(data[2]), res;
        char operator = data[0][0];
        if(operator == '+')
            res = operand_1 + operand_2;
        else if(operator == '-')
            res = operand_1 - operand_2;
        else if(operator == '*')
            res = operand_1 * operand_2;
        else
            res = operand_1 / operand_2;

        char *response = get_response(operand_1, operand_2, operator, res);
        send(client, response, strlen(response), 0);
        close(client);
    }
}

char** get_body(char* request) {
    char *data = NULL;

    if (strncmp(request, "GET", 3) == 0) {
        char *q = strchr(request, '?');
        if (!q) return NULL;

        q++;

        char *end = strchr(q, ' ');
        if (!end) return NULL;

        int len = end - q;
        data = (char*)malloc(len + 1);
        strncpy(data, q, len);
        data[len] = '\0';
    }
    else if (strncmp(request, "POST", 4) == 0) {
        char *body = strstr(request, "\r\n\r\n");
        if (!body) return NULL;

        body += 4;
        data = strdup(body);
    }
    else {
        return NULL;
    }

    char **result = (char**)malloc(3 * sizeof(char*));

    char *token = strtok(data, "&");
    while (token) {
        char *eq = strchr(token, '=');

        if (eq) {
            *eq = '\0';

            if (strcmp(token, "operator") == 0)
                result[0] = strdup(eq + 1);
            else if (strcmp(token, "operand_1") == 0)
                result[1] = strdup(eq + 1);
            else if (strcmp(token, "operand_2") == 0)
                result[2] = strdup(eq + 1);
        }

        token = strtok(NULL, "&");
    }

    free(data);
    return result;
}

char* get_response(int operand_1, int operand_2, char operator, int res){
    char *response = malloc(1024);

    sprintf(
        response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html>"
        "<head><title>Calculator</title></head>"
        "<body>"
        "<h1>%d %c %d = %d</h1>"
        "</body>"
        "</html>",
        operand_1, operator, operand_2, res
    );

    return response;
}
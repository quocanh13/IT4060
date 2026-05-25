#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

// gcc 19-5-ex2-server.c -o 19-5-ex2-server -pthread && ./19-5-ex2-server

int server;
int queue[2] = {-1, -1};

typedef struct {
    int client1;
    int client2;
} ClientPair;

void* handle_chat(void *arg) {

    ClientPair *pair = (ClientPair*)arg;

    int c1 = pair->client1;
    int c2 = pair->client2;

    free(pair);

    char message[1000];

    fd_set master;

    FD_ZERO(&master);

    FD_SET(c1, &master);
    FD_SET(c2, &master);

    int maxfd = (c1 > c2) ? c1 : c2;

    while (1) {

        fd_set fds = master;

        select(maxfd + 1, &fds, NULL, NULL, NULL);

        if(FD_ISSET(c1, &fds)) {

            int len =
                recv(c1, message,
                     sizeof(message) - 1, 0);

            if(len <= 0) {
                close(c1);
                close(c2);
                return NULL;
            }

            message[len] = '\0';

            send(c2, message, len, 0);
        }

        if(FD_ISSET(c2, &fds)) {

            int len =
                recv(c2, message,
                     sizeof(message) - 1, 0);

            if(len <= 0) {
                close(c1);
                close(c2);
                return NULL;
            }

            message[len] = '\0';

            send(c1, message, len, 0);
        }
    }
}

int main() {

    char *server_ip = "127.0.0.1";
    int server_port = 5000;

    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    inet_pton(
        AF_INET,
        server_ip,
        &server_addr.sin_addr
    );

    server = socket(AF_INET, SOCK_STREAM, 0);

    if(server < 0) {
        printf("Failed to create socket\n");
        exit(0);
    }

    if(bind(server,
        (struct sockaddr*)&server_addr,
        sizeof(server_addr)) < 0) {

        printf("Failed to bind\n");
        exit(0);
    }

    if(listen(server, 10) < 0) {
        printf("Failed to listen\n");
        exit(0);
    }

    printf("Server is listening\n");

    while (1) {

        int client =
            accept(server, NULL, NULL);

        if(client < 0)
            continue;

        if(queue[0] == -1) {
            queue[0] = client;
            continue;
        }

        if(queue[1] == -1) {
            queue[1] = client;
            ClientPair *pair =
                malloc(sizeof(ClientPair));

            pair->client1 = queue[0];
            pair->client2 = queue[1];

            pthread_t t;

            pthread_create(
                &t,
                NULL,
                handle_chat,
                pair
            );

            pthread_detach(t);
            queue[0] = -1;
            queue[1] = -1;
        }
    }

    close(server);

    return 0;
}
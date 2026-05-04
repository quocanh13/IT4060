#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
// gcc 28-4-ex2.c -o 28-4-ex2 && ./28-4-ex2

struct pollfd fds[2];
int main(int argc, char* arg[]){
    char *my_ip = "127.0.0.1";
    int my_port = atoi(arg[1]);
    struct sockaddr_in my_addr = {0};

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(my_port);
    inet_pton(AF_INET, my_ip, &my_addr.sin_addr); 

    char *client_ip = arg[2];
    int client_port = atoi(arg[3]);
    struct sockaddr_in client_addr = {0};

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    inet_pton(AF_INET, client_ip, &client_addr.sin_addr); 

    int me = socket(AF_INET, SOCK_DGRAM, 0);
    if(bind(me, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        printf("Failed to bind\n");
        exit(0);
    }

    fds[0].fd = STDIN_FILENO; 
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    fds[1].fd = me;
    fds[1].events = POLLIN;
    fds[1].revents = 0;
    char rcv_msg[1000], send_msg[1000];
    while(1){
        int eventc = poll(fds, 2, -1);
        if (fds[1].revents & POLLIN) {
            int n = recvfrom(me, rcv_msg, 1000 - 1, 0, NULL, NULL);
            if (n > 0) {
                rcv_msg[n] = '\0';
            }
            printf("Other: %s", rcv_msg);
        }

        if (fds[0].revents & POLLIN) {
            if(fgets(send_msg, 1000 - 1, stdin) != NULL) {
                sendto(me, send_msg, strlen(send_msg), 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
            }
        }
    }
}
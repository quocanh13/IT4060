#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
// gcc 31-3-ex2-client2.c -o 31-3-ex2-client1 && ./31-3-ex2-client1 6001 127.0.0.1 6002

int main(int argc, char **args){
    char s_ip[] = "127.0.0.1";
    int s_port = atoi(args[1]);
    struct sockaddr_in s_addr = {0};

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(s_port);
    inet_pton(AF_INET, s_ip, &s_addr.sin_addr);

    char *d_ip = args[2];
    int d_port = atoi(args[3]);
    struct sockaddr_in d_addr = {0};
    socklen_t d_addr_len = sizeof(d_addr);

    d_addr.sin_family = AF_INET;
    d_addr.sin_port = htons(d_port);
    inet_pton(AF_INET, d_ip, &d_addr.sin_addr);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s < 0) {
        printf("Failed to create s_socket\n");
        exit(0);
    }

    if(bind(s, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
        printf("Failed to bind s_socket\n");
        exit(0);
    }

    if(fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
        printf("Failed to convert s_socket to async\n");
        exit(0);
    }

    int epfd = epoll_create1(0);
    struct epoll_event epev = {0};
    epev.data.fd = s;
    epev.events = EPOLLIN;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, s, &epev) < 0){
        printf("Failed to add s_socket to epfd\n");
        exit(0); 
    }
    
    epev.data.fd = STDIN_FILENO;
    epev.events = EPOLLIN;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &epev) < 0){
        printf("Failed to add STDIN_FILENO to epfd\n");
        exit(0); 
    }
    struct epoll_event events[100];
    char buffer[1000];
    printf("You: ");
    fflush(stdout);
    while (1) {
        int n = epoll_wait(epfd, events, 2, -1); 

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == s) {
                struct sockaddr_in from_addr;
                socklen_t from_len = sizeof(from_addr);
                int len = recvfrom(s, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&from_addr, &from_len);
                if (len > 0) {
                    buffer[len] = '\0';
                    printf("\rOther: %s", buffer);
                    printf("You: "); fflush(stdout);
                }
            } 
            else if (events[i].data.fd == STDIN_FILENO) {
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    sendto(s, buffer, strlen(buffer), 0, (struct sockaddr*)&d_addr, sizeof(d_addr));
                    printf("You: "); fflush(stdout);
                }
            }
        }
    }
    
}
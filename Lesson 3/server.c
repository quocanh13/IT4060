#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
int main(){
    char *server_ip = "127.0.0.1";
    int server_port = 5000;
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if( bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to bind server\n");
    } else {
        int on = 1;
        ioctl(server, FIONBIO, &on);

        if( listen(server, 10) < 0 ) {
            printf("Failed to listen\n");
        } else {
            int nfds = server + 1;
            fd_set root, readfds;
            FD_ZERO(&root);
            FD_SET(server, &root);
            int clients[100], cur_client = 0;
            while(1){
                readfds = root;
                int n = select(nfds, &readfds, NULL, NULL, NULL);
                if(n < 0) {
                    printf("Select fail\n");
                }
                if(FD_ISSET(server, &readfds)){
                    int client = accept(server, NULL, NULL);
                    if(client >= 0) {
                        clients[cur_client++] = client;
                        nfds = (client + 1 > nfds) ? client + 1 : nfds;
                        FD_SET(client, &root);
                    }
                }
                for(int i = 0; i < cur_client; i++){
                    if(FD_ISSET(clients[i], &readfds)){
                        char buffer[1000];
                        int len = recv(clients[i], buffer, sizeof(buffer)-1, 0);
                        if(len > 0){
                            buffer[len] = '\0';
                            printf("%s\n", buffer);
                        } else {
                            close(clients[i]);
                            FD_CLR(clients[i], &root);
                            clients[i] = clients[cur_client-1];
                            cur_client--;
                            i--;
                            continue;
                        }
                    }
                }
            }
        }
    }
}
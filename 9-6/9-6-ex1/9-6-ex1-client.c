#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define BUF_SIZE 8192

void recv_response(int sock, char *buf, int size)
{
    int len = recv(sock, buf, size - 1, 0);

    if (len <= 0)
    {
        perror("recv");
        exit(1);
    }

    buf[len] = '\0';
    printf("%s", buf);
}

int create_pasv_socket(int control_sock)
{
    char buf[1024];
    int h1, h2, h3, h4, p1, p2;

    send(control_sock, "PASV\r\n", 6, 0);

    recv_response(control_sock, buf, sizeof(buf));

    if (sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
    {
        printf("PASV parse error\n");
        exit(1);
    }

    char ip[32];
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);

    int port = p1 * 256 + p2;

    int data_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(data_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect data");
        exit(1);
    }

    return data_sock;
}

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0)
    {
        perror("socket");
        return 1;
    }

    struct addrinfo hints = {0}, *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("lebavui.io.vn", "ftp", &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    if (connect(client, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("connect");
        return 1;
    }

    char buf[BUF_SIZE];

    recv_response(client, buf, sizeof(buf));

    send(client, "USER user_20235263\r\n", strlen("USER user_20235263\r\n"), 0);
    recv_response(client, buf, sizeof(buf));

    send(client, "PASS 526313\r\n", strlen("PASS 526313\r\n"), 0);
    recv_response(client, buf, sizeof(buf));

    if (strncmp(buf, "230", 3) != 0)
    {
        printf("Login failed\n");
        return 1;
    }

    send(client, "TYPE I\r\n", strlen("TYPE I\r\n"), 0);
    recv_response(client, buf, sizeof(buf));

    int data_sock = create_pasv_socket(client);

    send(client, "LIST\r\n", strlen("LIST\r\n"), 0);
    recv_response(client, buf, sizeof(buf));

    char files[BUF_SIZE];
    int total = 0;
    int len;

    while ((len = recv(data_sock, files + total, sizeof(files) - 1 - total, 0)) > 0)
        total += len;

    files[total] = '\0';

    close(data_sock);

    recv_response(client, buf, sizeof(buf));

    printf("%s\n", files);

    char question_file[256];

    char *p = strstr(files, "question_");

    if (p == NULL)
    {
        printf("Question file not found\n");
        return 1;
    }

    sscanf(p, "%255s", question_file);

    printf("Question file: %s\n", question_file);

    data_sock = create_pasv_socket(client);

    char cmd[512];

    sprintf(cmd, "RETR %s\r\n", question_file);
    send(client, cmd, strlen(cmd), 0);

    recv_response(client, buf, sizeof(buf));

    char question[1024];

    total = 0;

    while ((len = recv(data_sock, question + total, sizeof(question) - 1 - total, 0)) > 0)
        total += len;

    question[total] = '\0';

    close(data_sock);

    recv_response(client, buf, sizeof(buf));

    printf("Question content:\n%s\n", question);

    int qlen = strlen(question);

    while (qlen > 0 && (question[qlen - 1] == '\r' || question[qlen - 1] == '\n'))
        qlen--;

    char answer[1024];

    for (int i = 0; i < qlen; i++)
        answer[i] = question[qlen - 1 - i];

    answer[qlen] = '\0';

    printf("Answer content:\n%s\n", answer);

    char answer_file[256];

    strcpy(answer_file, question_file);
    memcpy(answer_file, "answer", 6);

    printf("Answer file: %s\n", answer_file);

    data_sock = create_pasv_socket(client);

    sprintf(cmd, "STOR %s\r\n", answer_file);
    send(client, cmd, strlen(cmd), 0);

    recv_response(client, buf, sizeof(buf));

    send(data_sock, answer, strlen(answer), 0);

    shutdown(data_sock, SHUT_WR);
    close(data_sock);

    recv_response(client, buf, sizeof(buf));

    send(client, "QUIT\r\n", strlen("QUIT\r\n"), 0);
    recv_response(client, buf, sizeof(buf));

    close(client);
    freeaddrinfo(res);

    return 0;
}
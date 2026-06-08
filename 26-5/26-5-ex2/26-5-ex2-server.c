#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 5000
#define ROOT_DIR "./files"
// gcc 26-5-ex2-server.c -o 26-5-ex2-server && ./26-5-ex2-server

char* get_mime_type(const char *path)
{
    char *ext = strrchr(path, '.');

    if (!ext) return "application/octet-stream";

    if (!strcmp(ext, ".html")) return "text/html";
    if (!strcmp(ext, ".txt"))  return "text/plain";

    if (!strcmp(ext, ".jpg"))  return "image/jpeg";
    if (!strcmp(ext, ".jpeg")) return "image/jpeg";
    if (!strcmp(ext, ".png"))  return "image/png";
    if (!strcmp(ext, ".gif"))  return "image/gif";

    if (!strcmp(ext, ".mp3"))  return "audio/mpeg";
    if (!strcmp(ext, ".mp4"))  return "video/mp4";

    return "application/octet-stream";
}

void send_404(int client)
{
    char *msg =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>404 Not Found</h1>";

    send(client, msg, strlen(msg), 0);
}

void send_directory_listing(int client, const char *real_path, const char *url_path){
    DIR *dir = opendir(real_path);

    if (!dir)
    {
        send_404(client);
        return;
    }

    char *html = malloc(1024 * 1024);

    sprintf(html,
        "<html>"
        "<head><title>Index of %s</title></head>"
        "<body>"
        "<h1>Index of %s</h1>",
        url_path,
        url_path);

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!strcmp(entry->d_name, "."))
            continue;

        char child_real[2048];

        snprintf(child_real, sizeof(child_real), "%s/%s", real_path, entry->d_name);

        struct stat st;

        if (stat(child_real, &st) != 0)
            continue;

        char href[2048];

        if (!strcmp(url_path, "/"))
            snprintf(href, sizeof(href), "/%s", entry->d_name);
        else
            snprintf(href, sizeof(href), "%s/%s", url_path, entry->d_name);

        if (S_ISDIR(st.st_mode))
        {
            sprintf(html + strlen(html), "<b><a href=\"%s\">%s</a></b><br>", href, entry->d_name);
        }
        else
        {
            sprintf(html + strlen(html), "<i><a href=\"%s\">%s</a></i><br>", href, entry->d_name);
        }
    }

    strcat(html, "</body></html>");

    char header[512];

    sprintf(
        header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        strlen(html));

    send(client, header, strlen(header), 0);
    send(client, html, strlen(html), 0);

    closedir(dir);
    free(html);
}

void send_file(int client, const char *path)
{
    FILE *f = fopen(path, "rb");

    if (!f)
    {
        send_404(client);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char header[512];

    sprintf(
        header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        get_mime_type(path),
        size);

    send(client, header, strlen(header), 0);

    char buffer[8192];

    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        send(client, buffer, n, 0);
    }

    fclose(f);
}

int main()
{
    int server_fd;

    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

    listen(server_fd, 10);

    printf("HTTP File Server listening on port %d\n", PORT);

    while (1)
    {
        int client =
            accept(server_fd, NULL, NULL);

        if (client < 0)
            continue;

        char request[4096];

        int len =
            recv(client, request, sizeof(request) - 1, 0);

        if (len <= 0)
        {
            close(client);
            continue;
        }

        request[len] = '\0';

        char method[32];
        char url_path[2048];

        sscanf(request, "%s %s", method, url_path);

        if (!strcmp(url_path, "/favicon.ico"))
        {
            char *resp =
                "HTTP/1.1 204 No Content\r\n\r\n";

            send(client, resp, strlen(resp), 0);

            close(client);
            continue;
        }

        if (strstr(url_path, ".."))
        {
            send_404(client);
            close(client);
            continue;
        }

        char real_path[4096];

        snprintf(real_path, sizeof(real_path), "%s%s", ROOT_DIR, url_path);

        struct stat st;

        if (stat(real_path, &st) != 0)
        {
            send_404(client);
            close(client);
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            send_directory_listing(client, real_path, url_path);
        }
        else
        {
            send_file(client, real_path);
        }

        close(client);
    }

    close(server_fd);

    return 0;
}
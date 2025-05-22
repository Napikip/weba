#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BACKLOG 5
#define REQ_BUF 2048
#define WEB_PORT 8080

struct web_server {
    int fd;
    struct sockaddr_in addr_info;
};

static int init_socket(struct web_server *srv) {
    srv->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->fd == -1) return -1;

    srv->addr_info.sin_family = AF_INET;
    srv->addr_info.sin_port = htons(WEB_PORT);
    srv->addr_info.sin_addr.s_addr = INADDR_ANY;

    if (bind(srv->fd, (struct sockaddr*)&srv->addr_info, sizeof(srv->addr_info)) {
        close(srv->fd);
        return -1;
    }

    if (listen(srv->fd, BACKLOG)) {
        close(srv->fd);
        return -1;
    }
    return 0;
}

static void format_response(int fd, const char *msg) {
    char out[REQ_BUF];
    const char *templ = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><body>"
        "<div style='margin:20px;padding:20px;border:1px solid #eee;'>"
        "%s"
        "</div>"
        "<img src='https://www.mirea.ru/upload/medialibrary/c1a/MIREA_Gerb_Colour.jpg' width=200>"
        "</body></html>";

    snprintf(out, REQ_BUF, templ, msg);
    send(fd, out, strlen(out), 0);
}

static void handle_request(int client_fd) {
    char buf[REQ_BUF] = {0};
    recv(client_fd, buf, REQ_BUF-1, 0);

    char *param = strstr(buf, "message=");
    char content[512] = "No message received";

    if (param) {
        param += 8;
        char *end = strpbrk(param, " &");
        size_t len = end ? (size_t)(end - param) : strlen(param);
        len = len > 511 ? 511 : len;

        CURL *curl=curl_easy_init();
        if (curl) {
            char *dec = NULL;
            dec= curl_easy_unespace(curl,param, len, NULL);
            if (dec) {
            snprintf(content, sizeof(content), "%.511s", dec);
            curl_free(dec);
        }
        curl_easy_cleanup(curl);
        }
    }

    format_response(client_fd, content);
    close(client_fd);
}

int main(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    struct web_server server;

    if (init_socket(&server)) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Listening on port %d...\n", WEB_PORT);

    while (1) {
        int client = accept(server.fd, NULL, NULL);
        if (client == -1) continue;
        handle_request(client);
    }

    close(server.fd);
    curl_global_cleanup();
    return 0;
}

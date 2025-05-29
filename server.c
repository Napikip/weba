#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 2048
#define MAX_MESSAGE_LENGTH 256

// Функция для декодирования URL
void url_decode(char* output, const char* input) {
    char high, low;
    while (*input) {
        if (*input == '%' && (high = input[1]) && (low = input[2]) && isxdigit(high) && isxdigit(low)) {
            high = high >= 'a' ? high - 32 : high;
            high = high >= 'A' ? high - 55 : high - 48;
            low = low >= 'a' ? low - 32 : low;
            low = low >= 'A' ? low - 55 : low - 48;
            *output++ = (high << 4) | low;
            input += 3;
        } else {
            *output++ = *input++;
        }
    }
    *output = 0;
}

// Основной сервер
void start_http_server() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);
    printf("Server listening on port %d\n", SERVER_PORT);

    while (1) {
        int client_socket = accept(server_socket, 0, 0);
        
        char request_buffer[BUFFER_SIZE];
        read(client_socket, request_buffer, BUFFER_SIZE);
        
        char* message_position = strstr(request_buffer, "message=");
        char encoded_message[MAX_MESSAGE_LENGTH] = {0};
        char decoded_message[MAX_MESSAGE_LENGTH] = {0};

        if (message_position) {
            sscanf(message_position + 8, "%255[^& \n]", encoded_message);
            url_decode(decoded_message, encoded_message);
        }

        char html_response[BUFFER_SIZE];
        snprintf(html_response, BUFFER_SIZE,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n\r\n"
            "<!DOCTYPE html><html><head><title>Simple Server</title></head>"
            "<body><div>%s</div>"
            "<img src='https://www.mirea.ru/upload/medialibrary/c1a/MIREA_Gerb_Colour.jpg' width='300'>"
            "</body></html>", 
            decoded_message);
            
        write(client_socket, html_response, strlen(html_response));
        close(client_socket);
    }
}

int main() {
    start_http_server();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

char* fetch_current_time(char* time_format) {
    time_t current_time;
    struct tm *time_info;
    char *time_str = (char*)malloc(256 * sizeof(char));

    time(&current_time);
    time_info = localtime(&current_time);

    if (strncmp(time_format, "dd/mm/yyyy", 10) == 0) {
        strftime(time_str, 256, "%d/%m/%Y", time_info);
    } else if (strncmp(time_format, "dd/mm/yy", 8) == 0) {
        strftime(time_str, 256, "%d/%m/%y", time_info);
    } else if (strncmp(time_format, "mm/dd/yyyy", 10) == 0) {
        strftime(time_str, 256, "%m/%d/%Y", time_info);
    } else if (strncmp(time_format, "mm/dd/yy", 8) == 0) {
        strftime(time_str, 256, "%m/%d/%y", time_info);
    } else {
        strftime(time_str, 256, "Format not recognized!", time_info);
    }
    return time_str;
}

int main() {
    // Tạo socket để kết nối
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("socket() error");
        return 1;
    }

    // Cấu hình địa chỉ máy chủ
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9000);

    // Gán socket với địa chỉ máy chủ
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address))) {
        perror("bind() error");
        return 1;
    }

    // Đặt socket ở trạng thái lắng nghe
    if (listen(server_socket, 5)) {
        perror("listen() error");
        return 1;
    }

    // Sử dụng fork để tạo tiến trình con
    for (int i = 0; i < 8; i++) {
        if (fork() == 0) {
            char buffer[256];
            while (1) {
                int client_socket = accept(server_socket, NULL, NULL);

                int received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
                if (received_bytes <= 0) {
                    close(client_socket);
                    continue;
                }

                if (received_bytes < sizeof(buffer))
                    buffer[received_bytes] = '\0';

                // Xử lý lệnh từ client và trả về thời gian tương ứng
                if (strncmp(buffer, "GET_TIME", 8) == 0) {
                    char* format = buffer + 9;
                    printf("Requested format: %s\n", format);
                    char* current_time;

                    current_time = fetch_current_time(format);

                    send(client_socket, current_time, strlen(current_time), 0);
                    free(current_time);
                } else {
                    char* error_message = "Command not recognized!";
                    send(client_socket, error_message, strlen(error_message), 0);
                }

                close(client_socket);
            }
            exit(0);
        }
    }

    // Nhấn phím để thoát chương trình
    getchar();

    // Đóng socket và thoát chương trình
    close(server_socket);

    return 0;
}

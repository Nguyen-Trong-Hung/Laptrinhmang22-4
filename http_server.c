#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8001
#define BUFFER_SIZE 1024
#define PENDING_CONNECTIONS 5

// Hàm xử lý client
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);

    // Kiểm tra dữ liệu nhận được từ client
    if (received_bytes > 0) {
        // Kết thúc chuỗi bằng null
        buffer[received_bytes] = '\0';

        // In ra dữ liệu nhận được
        printf("Received from client: %s\n", buffer);

        // Trả về phản hồi HTTP
        const char* http_response = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/html\r\n\r\n"
                                    "<html><body><h1>Hello World</h1></body></html>";
        send(client_socket, http_response, strlen(http_response), 0);
    }

    // Đóng kết nối với client
    close(client_socket);
}

int main() {
    // Tạo socket để lắng nghe kết nối
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("socket() failed");
        return EXIT_FAILURE;
    }

    // Định cấu hình địa chỉ cho server
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Gán socket với cấu trúc địa chỉ
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("bind() failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Chuyển socket sang trạng thái chờ kết nối
    if (listen(server_socket, PENDING_CONNECTIONS) != 0) {
        perror("listen() failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Thiết lập việc xử lý tín hiệu để kết thúc chương trình khi nhận tín hiệu từ người dùng
    signal(SIGINT, SIG_IGN);

    // Tạo các tiến trình con để xử lý client
    for (int i = 0; i < 8; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Tiến trình con
            while (1) {
                int client_socket = accept(server_socket, NULL, NULL);

                // Kiểm tra kết nối từ client
                if (client_socket != -1) {
                    handle_client(client_socket);
                } else {
                    perror("accept() failed");
                }
            }
            // Tiến trình con thoát sau khi hoàn thành công việc
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork() failed");
            close(server_socket);
            return EXIT_FAILURE;
        }
    }

    // Chờ người dùng nhập một ký tự để thoát
    getchar();

    // Kết thúc tất cả tiến trình con khi người dùng nhập
    killpg(0, SIGKILL);

    // Đóng socket của server
    close(server_socket);

    return EXIT_SUCCESS;
}

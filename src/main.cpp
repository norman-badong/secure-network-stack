#include "../include/server.hpp"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main() {
    // Initialize OpenSSL context and certificates
    SSL_CTX* ssl_ctx = create_ssl_context();
    configure_certificates(ssl_ctx);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[Error] Failed to create socket.\n";
        return EXIT_FAILURE;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[Error] Bind failed on port " << PORT << "\n";
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "[Error] Listen failed.\n";
        close(server_fd);
        return EXIT_FAILURE;
    }

    std::cout << "[Server] TLS 1.3 Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            std::cerr << "[Error] Accept failed.\n";
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        // Spawn a detached worker thread to manage the TLS session asynchronously
        std::thread(handle_tls_client, ssl_ctx, client_fd, std::string(client_ip)).detach();
    }

    close(server_fd);
    SSL_CTX_free(ssl_ctx);
    return 0;
}
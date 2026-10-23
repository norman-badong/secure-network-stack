#include "../include/server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

SSL_CTX* create_ssl_context() {
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        std::cerr << "Unable to create SSL context\n";
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Enforce TLS 1.3 strictly for modern cryptographic security and speed
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    return ctx;
}

void configure_certificates(SSL_CTX* ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "certs/server.cert", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    // Note the capital 'K' in PrivateKey below:
    if (SSL_CTX_use_PrivateKey_file(ctx, "certs/server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void handle_tls_client(SSL_CTX* ctx, int client_fd, std::string client_ip) {
    std::cout << "[Worker Thread] Active for TLS client: " << client_ip << "\n";

    // Create a new SSL structure for this specific connection
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);

    // Perform the TLS 1.3 cryptographic handshake over the TCP socket
    if (SSL_accept(ssl) <= 0) {
        std::cerr << "[TLS Error] Handshake failed.\n";
        ERR_print_errors_fp(stderr);
    } else {
        std::cout << "[TLS Success] Secure handshake completed. Cipher: " << SSL_get_cipher(ssl) << "\n";

        // Read encrypted data from the client
        char buffer[256];
        int bytes_read = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "[Encrypted Data Received]: " << buffer << "\n";

            // Send encrypted acknowledgment back
            const char* reply = "SECURE_ACK_FROM_TLS_SERVER\n";
            SSL_write(ssl, reply, strlen(reply));
        }
    }

    // Clean teardown
    SSL_free(ssl);
    close(client_fd);
    std::cout << "[Worker Thread] Connection closed for: " << client_ip << "\n";
}
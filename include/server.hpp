#pragma once

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

// OpenSSL Initialization and Configuration
SSL_CTX* create_ssl_context();
void configure_certificates(SSL_CTX* ctx);

// Worker Thread Handler for TLS Clients
void handle_tls_client(SSL_CTX* ctx, int client_fd, std::string client_ip);
#pragma once
#include <cstdio>

#include <openssl/err.h>
#include <openssl/ssl.h>

void setup_openssl() {
    SSL_library_init();
    SSL_load_error_strings();
}

void cleanup_openssl() { }

SSL_CTX *setup_server_ctx() {
    SSL_CTX *out = SSL_CTX_new(SSLv23_server_method());

    if (SSL_CTX_use_certificate_file(out, SERVER_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(out, SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return out;
}

SSL_CTX *setup_client_ctx() {
    SSL_CTX *out = SSL_CTX_new(SSLv23_client_method());

    if (SSL_CTX_use_certificate_file(out, CLIENT_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(out, CLIENT_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return out;
}
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
    SSL_CTX *out = nullptr;

    if ((out = SSL_CTX_new(TLS_server_method())) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if(SSL_CTX_load_verify_locations(out, CA_PATH, NULL) == 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_verify(out, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       NULL);
    SSL_CTX_set_verify_depth(out, 1);

    if (SSL_CTX_use_certificate_file(out, SERVER_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(out, SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_check_private_key(out)) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return out;
}

SSL_CTX *setup_client_ctx() {
    SSL_CTX *out = nullptr;

    if ((out = SSL_CTX_new(TLS_client_method())) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if(SSL_CTX_load_verify_locations(out, CA_PATH, NULL) == 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_verify(out, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       NULL);
    SSL_CTX_set_verify_depth(out, 1);

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
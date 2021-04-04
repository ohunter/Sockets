#! /bin/sh

# The certificates built in this script are NOT safe for deployment.
# They are just here to test functionality and have to be regenerated once per day for that very reason.


# ROOT-CA
# Generate a root CA
openssl req -x509 -newkey rsa:4096 -sha256 -days 1 -nodes -keyout root-ca.key -out root-ca.crt -subj "/CN=libsock"

# SERVER
# Create a certificate key
openssl genrsa -out server.key 2048

# Generate a server certificate sign request
openssl req -new -sha256 -key server.key -subj "/CN=libsock" -out server.csr

# Generate a server certificate
openssl x509 -req -in server.csr -CA root-ca.crt -CAkey root-ca.key -CAcreateserial -out server.crt -days 1 -sha256

# CLIENT
# Create a certificate key
openssl genrsa -out client.key 2048

# Generate a client certificate sign request
openssl req -new -sha256 -key client.key -subj "/CN=libsock" -out client.csr

# Generate a client certificate
openssl x509 -req -in client.csr -CA root-ca.crt -CAkey root-ca.key -CAcreateserial -out client.crt -days 1 -sha256
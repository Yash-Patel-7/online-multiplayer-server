#ifndef P3_NET_H
#define P3_NET_H

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include "helper.h"

// declare enumeration for constants
typedef enum constant {
    NI_MAXSERV = 32,
    NI_MAXHOST = 1025,
} constant;

// prototypes of all functions
int create_server_socket(const char *port);
int create_client_socket(const char *host, const char *port);
int accept_incoming_connection(int server_socket, char **host, char **port);
char* receive_message(int socket, size_t *length);
ssize_t send_message(int socket, const char *message, size_t length);
ssize_t get_readable_socket(const int *sockets, size_t number_of_sockets, int timeout);

#endif //P3_NET_H

#include "net.h"

// function that creates and returns a server socket bound to the localhost and the given port
// returns -1 on error
int create_server_socket(const char *port) {
    // if port is NULL, return -1
    if (port == NULL) {
        return -1;
    }

    // get address info of localhost and the given port using hints and info_list
    struct addrinfo hints;
    struct addrinfo *info_list;
    struct addrinfo *info;
    int server_socket, error;
    memset(&hints, 0, sizeof(struct addrinfo));

    // set hints to ipv4 or ipv6, TCP, passive
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // get address info and check for errors
    error = getaddrinfo(NULL, port, &hints, &info_list);
    if (error) {
        return -1;
    }

    // loop through info_list and try to create a socket and bind it to the address
    for (info = info_list; info != NULL; info = info->ai_next) {
        // create socket and check for errors
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (server_socket == -1) {
            continue;
        }

        // bind socket to address and check for errors
        if (bind(server_socket, info->ai_addr, info->ai_addrlen) != 0) {
            close(server_socket);
            continue;
        }

        // enable socket to accept maximum incoming connections and check for errors
        if (listen(server_socket, SOMAXCONN) != 0) {
            close(server_socket);
            continue;
        }

        // if no errors occurred, break out of loop
        break;
    }

    // free info_list
    freeaddrinfo(info_list);

    // if info is NULL, no socket could be created and bound to the address
    if (info == NULL) {
        return -1;
    }

    // return server socket
    return server_socket;
}

// function that creates and returns a client socket connected to the given host and port
// returns -1 on error
int create_client_socket(const char *host, const char *port) {
    // if host or port is NULL, return -1
    if (host == NULL || port == NULL) {
        return -1;
    }

    // get address info of the given host and port using hints and info_list
    struct addrinfo hints;
    struct addrinfo *info_list;
    struct addrinfo *info;
    int client_socket, error;
    memset(&hints, 0, sizeof(struct addrinfo));

    // set hints to ipv4 or ipv6, TCP
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // get address info and check for errors
    error = getaddrinfo(host, port, &hints, &info_list);
    if (error) {
        return -1;
    }

    // loop through info_list and try to create a socket and connect it to the address
    for (info = info_list; info != NULL; info = info->ai_next) {
        // create socket and check for errors
        client_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (client_socket == -1) {
            continue;
        }

        // connect socket to address and check for errors
        if (connect(client_socket, info->ai_addr, info->ai_addrlen) != 0) {
            close(client_socket);
            continue;
        }

        // if no errors occurred, break out of loop
        break;
    }

    // free info_list
    freeaddrinfo(info_list);

    // if info is NULL, no socket could be created and connected to the address
    if (info == NULL) {
        return -1;
    }

    // return client socket
    return client_socket;
}

// function that accepts an incoming connection on the given server socket and returns the client socket
// sets the given host and port pointers to new allocated strings containing the numeric host and port of the client
// returns -1 on error
int accept_incoming_connection(int server_socket, char **host, char **port) {
    // if server socket is invalid, return -1
    if (server_socket < 0) {
        return -1;
    }

    // if host or port is NULL, return -1
    if (host == NULL || port == NULL) {
        return -1;
    }

    // initialize host and port pointers to NULL
    *host = NULL;
    *port = NULL;

    // accept incoming connection and check for errors
    struct sockaddr_storage client_address;
    socklen_t client_address_length = sizeof(struct sockaddr_storage);
    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_length);
    if (client_socket == -1) {
        return -1;
    }

    // get numeric host and port of client and check for errors
    *host = malloc(sizeof(char) * NI_MAXHOST);
    if (*host == NULL) {
        close(client_socket);
        return -1;
    }
    *port = malloc(sizeof(char) * NI_MAXSERV);
    if (*port == NULL) {
        close(client_socket);
        *host = Free(*host);
        return -1;
    }

    int error = getnameinfo(
            (struct sockaddr *) &client_address,
            client_address_length,
            *host,
            NI_MAXHOST,
            *port,
            NI_MAXSERV,
            NI_NUMERICHOST | NI_NUMERICSERV
    );

    if (error) {
        close(client_socket);
        *host = Free(*host);
        *port = Free(*port);
        return -1;
    }

    // return client socket
    return client_socket;
}

// function that receives a message from the given socket and returns a new allocated string containing the message
// sets the given length pointer to the length of the message (number of bytes)
// returns NULL on error
char* receive_message(int socket, size_t *length) {
    // if socket is invalid, return NULL
    if (socket < 0) {
        return NULL;
    }

    // if length is NULL, return NULL
    if (length == NULL) {
        return NULL;
    }

    // initialize length to 0
    *length = 0;

    // receive message from the socket using read() and poll() and check for errors
    size_t size = 101;
    char *buffer = malloc(sizeof(char) * size);
    if (buffer == NULL) {
        return NULL;
    }

    ssize_t bytes_read = read(socket, buffer, size - 1);
    if (bytes_read == -1 || bytes_read == 0) {
        Free(buffer);
        return NULL;
    }

    struct pollfd poll_socket;
    memset(&poll_socket, 0, sizeof(struct pollfd));
    poll_socket.fd = socket;
    poll_socket.events = POLLIN;

    while (1) {
        // poll socket
        int poll_result = poll(&poll_socket, 1, 0);

        // if poll() returned an error, free buffer and return NULL
        if (poll_result <= -1) {
            Free(buffer);
            return NULL;
        }

        // if poll() returned 0, break out of loop
        if (poll_result == 0) {
            break;
        }

        // if poll() returns more than 1, then there is an error so free buffer and return NULL
        if (poll_result > 1) {
            Free(buffer);
            return NULL;
        }

        // at this point poll() returned 1, so there is data to be read
        // if the buffer is full, increase size by 100
        if (bytes_read == size - 1) {
            size += 100;
            char *temp = realloc(buffer, sizeof(char) * size);
            if (temp == NULL) {
                Free(buffer);
                return NULL;
            } else {
                buffer = temp;
            }
        }

        // check if revents is POLLIN and then read from socket
        if (poll_socket.revents & POLLIN) {
            ssize_t read_status = read(socket, buffer + bytes_read, size - bytes_read - 1);
            if (read_status == -1) {
                Free(buffer);
                return NULL;
            }
            if (read_status == 0) {
                break;
            }
            bytes_read += read_status;
        } else {
            Free(buffer);
            return NULL;
        }
    }

    // set last byte of buffer to '\0' based on bytes_read
    buffer[bytes_read] = '\0';

    // set length to bytes_read
    *length = bytes_read;

    // return buffer
    return buffer;
}

// function that sends the given message to the given socket
// returns 0 on success and -1 on error
ssize_t send_message(int socket, const char *message, size_t length) {
    // if socket is invalid, return -1
    if (socket < 0) {
        return -1;
    }

    // if message is NULL, return 0
    if (message == NULL) {
        return 0;
    }

    // if length is 0, return 0
    if (length == 0) {
        return 0;
    }

    // send message to the socket using write() and check for errors
    ssize_t bytes_written = write(socket, message, length);
    if (bytes_written == -1) {
        return -1;
    }

    // if the number of bytes written is not equal to the length, keep writing until all bytes are written
    while (bytes_written < length) {
        ssize_t write_status = write(socket, message + bytes_written, length - bytes_written);
        if (write_status == -1) {
            return -1;
        }
        bytes_written += write_status;
    }

    // return 0 on success
    return 0;
}

// function that polls the given list of sockets and returns the index of the socket that has data to be read
// returns -1 on error or poll timed out (in milliseconds)
ssize_t get_readable_socket(const int *sockets, size_t number_of_sockets, int timeout) {
    // if sockets is NULL, return -1
    if (sockets == NULL) {
        return -1;
    }

    // if number_of_sockets is 0, return -1
    if (number_of_sockets == 0) {
        return -1;
    }

    // allocate memory for pollfd array
    struct pollfd *poll_sockets = malloc(sizeof(struct pollfd) * number_of_sockets);
    if (poll_sockets == NULL) {
        return -1;
    }

    // memset pollfd array to 0
    memset(poll_sockets, 0, sizeof(struct pollfd) * number_of_sockets);

    // initialize pollfd array
    for (size_t i = 0; i < number_of_sockets; i++) {
        poll_sockets[i].fd = sockets[i];
        poll_sockets[i].events = POLLIN;
    }

    // poll sockets for the timeout period and check for errors
    int poll_result = poll(poll_sockets, number_of_sockets, timeout);

    // if poll() returned an error, free poll_sockets and return -1
    if (poll_result <= -1) {
        Free(poll_sockets);
        return -1;
    }

    // if poll() returned 0, free poll_sockets and return -1
    if (poll_result == 0) {
        Free(poll_sockets);
        return -1;
    }

    // if poll() returns more than the number of sockets, then there is an error so free poll_sockets and return -1
    if (poll_result > number_of_sockets) {
        Free(poll_sockets);
        return -1;
    }

    // at this point poll() returned the number of sockets that may have data to be read
    // loop through poll_sockets and check if revents is POLLIN
    ssize_t index = 0;
    for (size_t i = 0; i < number_of_sockets; i++, index++) {
        if (poll_sockets[i].revents & POLLIN) {
            Free(poll_sockets);
            return index;
        }
    }

    // at this point, if no sockets have data to be read, then there is an error so free poll_sockets and return -1
    Free(poll_sockets);
    return -1;
}


#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <pthread.h>
#include "helper.h"
#include "net.h"

int socket_global = -1;
char *host_global = NULL;
char *port_global = NULL;

// prototypes of all functions
void check_arguments(int argc);
void setup_signal_handlers();
void signal_handler(int signal);
void* from_server(void *arg);
void simulate_client(const char *host, const char *port);
void log_message_client(const char *message, const char *host, const char *port, const size_t *is_sent);

// driver
int main(int argc, char **argv) {
    // set stdout and stderr buffer to NULL
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    // check if the arguments are correct
    check_arguments(argc);

    // set up the signal handlers
    setup_signal_handlers();

    host_global = argv[1];
    port_global = argv[2];

    // simulate the client
    simulate_client(argv[1], argv[2]);

    // exit the program successfully
    return EXIT_SUCCESS;
}

// function that checks if the arguments are correct
void check_arguments(int argc) {
    // check if the number of arguments is correct
    if (argc != 3) {
        if (write(STDERR_FILENO, "Usage: ./ttt <host> <port>\n", 27) != 27) {
            perror("write");
        }
        exit(EXIT_FAILURE);
    }
}

// function that sets up the signal handlers
void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = &signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) != 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

// function for handling signals
void signal_handler(int signal) {
    switch (signal) {
        case SIGPIPE:
            if (write(STDERR_FILENO, "\nSIGPIPE received\n", 18) != 18) {
                perror("write");
            }
            break;
        case SIGINT:
            if (write(STDERR_FILENO, "\nSIGINT received\n", 17) != 17) {
                perror("write");
            }
            break;
        case SIGTERM:
            if (write(STDERR_FILENO, "\nSIGTERM received\n", 18) != 18) {
                perror("write");
            }
            break;
        default:
            if (write(STDERR_FILENO, "\nUnknown signal received\n", 25) != 25) {
                perror("write");
            }
            break;
    }
}

// function that prints out the messages received from the server
void* from_server(void *arg) {
    if (arg != NULL) {
        pthread_exit(NULL);
    }
    size_t is_sent = 0;
    size_t length = 0;
    while (1) {
        char *msg = receive_message(socket_global, &length);
        if (msg == NULL) {
            perror("receive_message");
            pthread_exit(NULL);
        }
        log_message_client(msg, host_global, port_global, &is_sent);
        Free(msg);
    }
}

// function that simulates the client
void simulate_client(const char *host, const char *port) {
    // create a client socket
    int client_socket = create_client_socket(host, port);
    socket_global = client_socket;
    if (client_socket == -1) {
        perror("create_client_socket");
        exit(EXIT_FAILURE);
    }

    // log the server's host and port using log_message_client()
    log_message_client("Connected", host, port, NULL);

    pthread_t thread;
    if (pthread_create(&thread, NULL, &from_server, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    if (pthread_detach(thread) != 0) {
        perror("pthread_detach");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // send the message to the server
        char *msg_to_send = read_file(STDIN_FILENO);
        if (msg_to_send == NULL) {
            msg_to_send = strdup("EOF");
            if (msg_to_send == NULL) {
                close(client_socket);
                perror("strdup");
                exit(EXIT_FAILURE);
            }
        } else if (strlen(msg_to_send) == 0) {
            Free(msg_to_send);
            continue;
        }

        ssize_t result = send_message(client_socket, msg_to_send, strlen(msg_to_send));
        if (result == -1) {
            close(client_socket);
            Free(msg_to_send);
            perror("send_message");
            exit(EXIT_FAILURE);
        }

        // log the message using log_message_client()
        size_t is_sent = 1;
        log_message_client(msg_to_send, host, port, &is_sent);

        // free the message to send buffer
        Free(msg_to_send);
    }

    // close the client socket
//    close(client_socket);
}

// function that logs a message
void log_message_client(const char *message, const char *host, const char *port, const size_t *is_sent) {
    // log the message in format [CLIENT] [SERVER host:port] [SENT or RECV] [message]
    size_t log_length =
            strlen("[CLIENT] [SERVER :] [SENT] []") +
            strlen(host) +
            strlen(port) +
            strlen(message) + 2;

    char *log = malloc(sizeof(char) * log_length);

    if (log == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // use string concatenation to create the log message
    strcpy(log, "[CLIENT] [SERVER ");
    strcat(log, host);
    strcat(log, ":");
    strcat(log, port);
    strcat(log, "]");

    if (is_sent != NULL) {
        if (*is_sent == 0) {
            strcat(log, " [RECV] [");
        } else {
            strcat(log, " [SENT] [");
        }
    } else {
        strcat(log, " [");
    }

    strcat(log, message);
    strcat(log, "]\n");

    // write the log message to stdout using strlen()
    if (write(STDOUT_FILENO, log, strlen(log)) != strlen(log)) {
        Free(log);
        perror("write");
        exit(EXIT_FAILURE);
    }

    // free the log buffer
    Free(log);
}

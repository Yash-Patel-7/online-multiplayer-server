#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <pthread.h>
#include "msg.h"

// define struct for the game
typedef struct game {
    int client1_socket;
    int client2_socket;
    char *client1_host;
    char *client2_host;
    char *client1_port;
    char *client2_port;
    char *player1_name;
    char *player2_name;
    char *msg_buffer1;
    char *msg_buffer2;
} game;

// prototypes of all functions
void check_arguments(int argc);
void setup_signal_handlers();
void signal_handler(int signal);
void obtain_mutex_lock(pthread_mutex_t *mutex);
void release_mutex_lock(pthread_mutex_t *mutex);
void add_player_name(const char *player_name);
void remove_player_name(const char *player_name);
size_t is_player_name_taken(const char *player_name);
int get_client(int server_socket, char **client_host, char **client_port, char **player_name, char **msg_buffer);
int get_server(const char *port);
void simulate_server(const char *port);
void* handle_game(void *arg);
void free_game(game *arg);
ssize_t get_game_status(const char *board, char *status, char *winner);
ssize_t make_move(char *board, char role, size_t row, size_t col);
ssize_t generate_MOVD(const char *board, char role, size_t row, size_t col, char **movd_msg);
ssize_t generate_BEGN(char role, const char *opponent_name, char **begn_msg);

// global variable for the protocol that is thread-safe because it is read only
static const char* PROTOCOL[] = {
        "WAIT|0|",
        "MOVD|16|",
        "INVL|24|That space is occupied.|",
        "INVL|17|!Protocol error.|",
        "INVL|21|Name already in use.|",
        "DRAW|2|S|",
        "DRAW|2|A|",
        "DRAW|2|R|",
        "BEGN|",
        "OVER|35|W|One player has completed a line.|",
        "OVER|35|L|One player has completed a line.|",
        "OVER|27|W|One player has resigned.|",
        "OVER|27|L|One player has resigned.|",
        "OVER|32|D|Both players declared a draw.|",
        "OVER|20|D|The grid is full.|",
};

// global variable for the player names that are currently in the game
// this is not thread-safe because it is modified by multiple threads, so use a mutex lock
static size_t number_of_players = 0;
static char **player_names = NULL;

// create a mutex lock for the set of related shared resources, in this case {number_of_players, player_names}
static pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

// driver
int main(int argc, char **argv) {
    // set stdout and stderr buffer to NULL
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    // check if the arguments are correct
    check_arguments(argc);

    // set up the signal handlers
    setup_signal_handlers();

    // simulate the server
    simulate_server(argv[1]);

    // exit the program successfully
    return EXIT_SUCCESS;
}

// function that checks if the arguments are correct
void check_arguments(int argc) {
    // check if the number of arguments is correct
    if (argc != 2) {
        if (write(STDERR_FILENO, "Usage: ./ttts <port>\n", 21) != 21) {
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

// function that obtains a mutex lock
void obtain_mutex_lock(pthread_mutex_t *mutex) {
    if (pthread_mutex_lock(mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
}

// function that releases a mutex lock
void release_mutex_lock(pthread_mutex_t *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

// function that adds a player's name to the player list of names
void add_player_name(const char *player_name) {
    // obtain the mutex lock
    obtain_mutex_lock(&players_mutex);

    // if player_name is NULL, do nothing
    if (player_name == NULL || strlen(player_name) == 0) {
        release_mutex_lock(&players_mutex);
        return;
    }

    // if player_names is NULL, allocate memory for it of 1 element
    if (player_names == NULL) {
        player_names = malloc(sizeof(char *));
        if (player_names == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        player_names[0] = strdup(player_name);
        number_of_players++;
        release_mutex_lock(&players_mutex);
        return;
    }

    // if there is a NULL pointer in the array, replace it with the player name to save space
    for (size_t i = 0; i < number_of_players; i++) {
        if (player_names[i] == NULL) {
            player_names[i] = strdup(player_name);
            release_mutex_lock(&players_mutex);
            return;
        }
    }

    number_of_players++;
    char **temp = realloc(player_names, sizeof(char *) * number_of_players);
    if (temp == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
    } else {
        player_names = temp;
    }
    player_names[number_of_players - 1] = strdup(player_name);

    // release the mutex lock
    release_mutex_lock(&players_mutex);
}

// function that removes a player's name from the player list of names
void remove_player_name(const char *player_name) {
    // obtain the mutex lock
    obtain_mutex_lock(&players_mutex);

    // if player_name is NULL, do nothing
    if (player_name == NULL || strlen(player_name) == 0) {
        release_mutex_lock(&players_mutex);
        return;
    }

    // if the player name is in the array, replace it with a NULL pointer to save space
    for (size_t i = 0; i < number_of_players; i++) {
        if (player_names[i] != NULL && strcmp(player_names[i], player_name) == 0) {
            player_names[i] = Free(player_names[i]);
            break;
        }
    }

    // release the mutex lock
    release_mutex_lock(&players_mutex);
}

// function that checks if a player's name is in the player list of names
size_t is_player_name_taken(const char *player_name) {
    // obtain the mutex lock
    obtain_mutex_lock(&players_mutex);

    // if player_name is NULL, return 1
    if (player_name == NULL || strlen(player_name) == 0) {
        release_mutex_lock(&players_mutex);
        return 1;
    }

    // if the player name is in the array, return 1
    for (size_t i = 0; i < number_of_players; i++) {
        if (player_names[i] != NULL && strcmp(player_names[i], player_name) == 0) {
            release_mutex_lock(&players_mutex);
            return 1;
        }
    }

    // release the mutex lock
    release_mutex_lock(&players_mutex);

    // otherwise, return 0
    return 0;
}

// function that gets the client socket that is waiting to begin the game
int get_client(int server_socket, char **client_host, char **client_port, char **player_name, char **msg_buffer) {
    while (1) {
        // accept an incoming connection
        int client_socket = accept_incoming_connection(server_socket, client_host, client_port);
        if (client_socket == -1) {
            perror("accept_incoming_connection");
            continue;
        }

        // log the client's host and port using log_message()
        log_message("Connected", *client_host, *client_port, NULL);

        // declare a variable to determine if the message was sent or received
        size_t is_sent = 0;

        // declare a variable to determine if the server should continue accepting connections
        size_t continue_accepting;

        // get message from the client about player name
        *msg_buffer = NULL;
        while (1) {
            // get the message from the client
            char *msg = NULL;
            if (get_message(client_socket, msg_buffer, &msg) == -1) {
                // send INVL message to the client which is PROTOCOL[3]
                if (send_message(client_socket, PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                    perror("send_message");
                } else {
                    is_sent = 1;
                    log_message(PROTOCOL[3], *client_host, *client_port, &is_sent);
                }
                close(client_socket);
                *client_host = Free(*client_host);
                *client_port = Free(*client_port);
                *msg_buffer = Free(*msg_buffer);
                msg = Free(msg);
                perror("get_message");
                continue_accepting = 1;
                break;
            }

            // log the message using log_message()
            is_sent = 0;
            log_message(msg, *client_host, *client_port, &is_sent);

            // parse the message to get the player name
            *player_name = NULL;
            if (parse_play(msg, player_name) == -1) {
                // send a protocol error message to the client
                if (send_message(client_socket, PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                    close(client_socket);
                    *client_host = Free(*client_host);
                    *client_port = Free(*client_port);
                    *msg_buffer = Free(*msg_buffer);
                    msg = Free(msg);
                    perror("send_message");
                    continue_accepting = 1;
                    break;
                }

                // log the message using log_message()
                is_sent = 1;
                log_message(PROTOCOL[3], *client_host, *client_port, &is_sent);
                msg = Free(msg);
                continue;
            }

            // check if the player name is taken
            if (is_player_name_taken(*player_name)) {
                // send a INVL message to the client
                if (send_message(client_socket, PROTOCOL[4], strlen(PROTOCOL[4])) == -1) {
                    close(client_socket);
                    *client_host = Free(*client_host);
                    *client_port = Free(*client_port);
                    *msg_buffer = Free(*msg_buffer);
                    msg = Free(msg);
                    *player_name = Free(*player_name);
                    perror("send_message");
                    continue_accepting = 1;
                    break;
                }

                // log the message using log_message()
                is_sent = 1;
                log_message(PROTOCOL[4], *client_host, *client_port, &is_sent);
                msg = Free(msg);
                *player_name = Free(*player_name);
                continue;
            }

            // now we are done getting a valid player name so break out of the loop
            continue_accepting = 0;
            msg = Free(msg);
            break;
        }

        // if we should continue accepting connections, then continue
        if (continue_accepting == 1) {
            continue;
        }

        // send a WAIT message to the client
        if (send_message(client_socket, PROTOCOL[0], strlen(PROTOCOL[0])) == -1) {
            close(client_socket);
            *client_host = Free(*client_host);
            *client_port = Free(*client_port);
            *player_name = Free(*player_name);
            *msg_buffer = Free(*msg_buffer);
            perror("send_message");
            continue;
        }

        // log the message using log_message()
        is_sent = 1;
        log_message(PROTOCOL[0], *client_host, *client_port, &is_sent);

        // add the player name to the list of player names
        add_player_name(*player_name);

        // return the client socket
        return client_socket;
    }
}

// function that gets a server socket that is ready to begin the game
int get_server(const char *port) {
    // create a server socket
    int server_socket = create_server_socket(port);
    if (server_socket == -1) {
        perror("create_server_socket");
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

// function that simulates the server
void simulate_server(const char *port) {
    // get a server socket
    int server_socket = get_server(port);

    while (1) {
        // get a client socket
        char *client1_host = NULL;
        char *client1_port = NULL;
        char *player1_name = NULL;
        char *msg_buffer1 = NULL;
        int client1_socket = get_client(server_socket, &client1_host, &client1_port, &player1_name, &msg_buffer1);

        // get another client socket
        char *client2_host = NULL;
        char *client2_port = NULL;
        char *player2_name = NULL;
        char *msg_buffer2 = NULL;
        int client2_socket = get_client(server_socket, &client2_host, &client2_port, &player2_name, &msg_buffer2);

        // create a game struct
        game *arg = malloc(sizeof(game));
        if (arg == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        arg->client1_socket = client1_socket;
        arg->client1_host = client1_host;
        arg->client1_port = client1_port;
        arg->player1_name = player1_name;
        arg->msg_buffer1 = msg_buffer1;
        arg->client2_socket = client2_socket;
        arg->client2_host = client2_host;
        arg->client2_port = client2_port;
        arg->player2_name = player2_name;
        arg->msg_buffer2 = msg_buffer2;

        // create a game thread
        pthread_t game_thread;
        if (pthread_create(&game_thread, NULL, &handle_game, arg) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // detach the game thread
        if (pthread_detach(game_thread) != 0) {
            perror("pthread_detach");
            exit(EXIT_FAILURE);
        }
    }
}

// function that handles a game thread
void* handle_game(void *arg) {
    // extract the variables from the game struct into local variables on the stack
    game *args = (game*) arg;
    int client1_socket = args->client1_socket;
    char *client1_host = args->client1_host;
    char *client1_port = args->client1_port;
    char *player1_name = args->player1_name;
    char **msg_buffer1 = &(args->msg_buffer1);
    int client2_socket = args->client2_socket;
    char *client2_host = args->client2_host;
    char *client2_port = args->client2_port;
    char *player2_name = args->player2_name;
    char **msg_buffer2 = &(args->msg_buffer2);

    // initialize the game board on the stack
    char board[10] = ".........";

    // initialize variable to keep track of whose turn it is (0 for client1 and 1 for client2)
    size_t turn = 0;

    // initialize variable to keep track of which client needs to respond to draw suggestion
    size_t draw_response_index = 0;
    size_t is_draw_suggested = 0;

    // initialize variable that keeps track of each player's role
    // client1 is X and client2 is O
    char role[3] = "XO";

    // initialize a sockets array that contains both client sockets in order
    int sockets[2];
    sockets[0] = client1_socket;
    sockets[1] = client2_socket;

    // first generate BEGN message for each client
    char *begn_msg1 = NULL;
    char *begn_msg2 = NULL;
    if (generate_BEGN(role[0], player2_name, &begn_msg1) == -1) {
        perror("generate_BEGN");
        free_game(args);
        pthread_exit(NULL);
    }
    if (generate_BEGN(role[1], player1_name, &begn_msg2) == -1) {
        perror("generate_BEGN");
        free_game(args);
        pthread_exit(NULL);
    }

    // initialize variable for logging
    size_t is_sent = 1;

    // send BEGN message to each client
    if (send_message(sockets[0], begn_msg1, strlen(begn_msg1)) == -1) {
        perror("send_message");
        free_game(args);
        pthread_exit(NULL);
    }
    log_message(begn_msg1, client1_host, client1_port, &is_sent);
    if (send_message(sockets[1], begn_msg2, strlen(begn_msg2)) == -1) {
        perror("send_message");
        free_game(args);
        pthread_exit(NULL);
    }
    log_message(begn_msg2, client2_host, client2_port, &is_sent);

    // free BEGN messages
    Free(begn_msg1);
    Free(begn_msg2);

    // wait for either client to respond and process the messages
    while (1) {
        // if both msg buffers are NULL, then get a readable socket
        ssize_t index = -1;
        if (*msg_buffer1 == NULL && *msg_buffer2 == NULL) {
            index = get_readable_socket(sockets, 2, -1);
            if (index == -1 || index > 1) {
                perror("get_readable_socket");
                break;
            }
        } else if (*msg_buffer1 != NULL) {
            index = 0;
        } else {
            index = 1;
        }

        // initialize msg variable
        char *msg = NULL;

        // check which client sent the message and read the message
        is_sent = 0;
        if (index == 0) {
            // read the message from client1
            if (get_message(sockets[0], msg_buffer1, &msg) == -1) {
                // send INVL message to the client which is PROTOCOL[3]
                if (send_message(sockets[0], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                    perror("send_message");
                } else {
                    is_sent = 1;
                    log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
                }
                perror("get_message");
                break;
            }
            log_message(msg, client1_host, client1_port, &is_sent);
        } else {
            // read the message from client2
            if (get_message(sockets[1], msg_buffer2, &msg) == -1) {
                // send INVL message to the client which is PROTOCOL[3]
                if (send_message(sockets[1], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                    perror("send_message");
                } else {
                    is_sent = 1;
                    log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
                }
                perror("get_message");
                break;
            }
            log_message(msg, client2_host, client2_port, &is_sent);
        }
        is_sent = 1;

        // initialize a variable that keeps track of whether any of these messages were sent
        size_t is_valid_msg = 0;

        // if a client sends a RSGN message, the server should send OVER to both clients
        if (parse_rsgn(msg) == 0) {
            is_valid_msg = 1;
            // send OVER message to both clients which is in PROTOCOL[11] for winner
            // and PROTOCOL[12] for loser
            if (index == 0) {
                if (send_message(sockets[0], PROTOCOL[12], strlen(PROTOCOL[12])) == -1) {
                    perror("send_message");
                    msg = Free(msg);
                    break;
                }
                log_message(PROTOCOL[12], client1_host, client1_port, &is_sent);
                if (send_message(sockets[1], PROTOCOL[11], strlen(PROTOCOL[11])) == -1) {
                    perror("send_message");
                    msg = Free(msg);
                    break;
                }
                log_message(PROTOCOL[11], client2_host, client2_port, &is_sent);
            } else {
                if (send_message(sockets[1], PROTOCOL[12], strlen(PROTOCOL[12])) == -1) {
                    perror("send_message");
                    msg = Free(msg);
                    break;
                }
                log_message(PROTOCOL[12], client2_host, client2_port, &is_sent);
                if (send_message(sockets[0], PROTOCOL[11], strlen(PROTOCOL[11])) == -1) {
                    perror("send_message");
                    msg = Free(msg);
                    break;
                }
                log_message(PROTOCOL[11], client1_host, client1_port, &is_sent);
            }
            msg = Free(msg);
            break;
        }

        // process DRAW message
        char action = '\0';
        if (parse_draw(msg, &action) == 0) {
            is_valid_msg = 1;
            // if a draw has already been suggested, then check if this is the client that is expected to respond
            if (is_draw_suggested == 1 && index == draw_response_index) {
                if (action == 'R') {
                    // if the client responds with reject, then send PROTOCOL[7] to the other client
                    if (send_message(sockets[1 - index], PROTOCOL[7], strlen(PROTOCOL[7])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    if (index == 0) {
                        log_message(PROTOCOL[7], client2_host, client2_port, &is_sent);
                    } else {
                        log_message(PROTOCOL[7], client1_host, client1_port, &is_sent);
                    }
                    is_draw_suggested = 0;
                } else if (action == 'A') {
                    // if the client responds with accept, then send PROTOCOL[13] to both clients
                    if (send_message(sockets[0], PROTOCOL[13], strlen(PROTOCOL[13])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    log_message(PROTOCOL[13], client1_host, client1_port, &is_sent);
                    if (send_message(sockets[1], PROTOCOL[13], strlen(PROTOCOL[13])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    log_message(PROTOCOL[13], client2_host, client2_port, &is_sent);
                    msg = Free(msg);
                    break;
                } else {
                    // if the client responds with suggest or anything else, then send PROTOCOL[3] to the same client
                    if (send_message(sockets[index], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    if (index == 0) {
                        log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
                    } else {
                        log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
                    }
                }
            } else if (is_draw_suggested == 1) {
                // if a draw has already been suggested, but this is not the client that is expected to respond
                // then send PROTOCOL[3] to the same client
                if (send_message(sockets[index], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                    perror("send_message");
                    msg = Free(msg);
                    break;
                }
                if (index == 0) {
                    log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
                } else {
                    log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
                }
            } else {
                // if a draw has not been suggested, then check if the client wants to suggest a draw
                // otherwise send PROTOCOL[3] to the same client
                if (action == 'S') {
                    // if the client wants to suggest a draw, then send PROTOCOL[5] to the other client
                    if (send_message(sockets[1 - index], PROTOCOL[5], strlen(PROTOCOL[5])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    if (index == 0) {
                        log_message(PROTOCOL[5], client2_host, client2_port, &is_sent);
                    } else {
                        log_message(PROTOCOL[5], client1_host, client1_port, &is_sent);
                    }
                    is_draw_suggested = 1;
                    draw_response_index = 1 - index;
                } else {
                    // if the client does not want to suggest a draw, then send PROTOCOL[3] to the same client
                    if (send_message(sockets[index], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    if (index == 0) {
                        log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
                    } else {
                        log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
                    }
                }
            }
        }

        // process MOVE message
        char rol = '\0';
        size_t row = 0;
        size_t col = 0;
        if (parse_move(msg, &rol, &row, &col) == 0) {
            is_valid_msg = 1;
            // a move will only be processed if a draw has not been suggested
            // so if a draw has been suggested, then send PROTOCOL[3] to the same client
            if (is_draw_suggested == 1 || rol != role[index]) {
                if (send_message(sockets[index], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                    perror("send_message");
                    msg = Free(msg);
                    break;
                }
                if (index == 0) {
                    log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
                } else {
                    log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
                }
            } else {
                // if a draw has not been suggested, then process the move if it is the client's turn
                // otherwise send PROTOCOL[3] to the same client
                if (turn == index) {
                    // if it is the client's turn, then process the move
                    if (make_move(board, rol, row, col) == -1) {
                        // if the move is invalid, then send PROTOCOL[2] to the same client
                        if (send_message(sockets[index], PROTOCOL[2], strlen(PROTOCOL[2])) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        if (index == 0) {
                            log_message(PROTOCOL[2], client1_host, client1_port, &is_sent);
                        } else {
                            log_message(PROTOCOL[2], client2_host, client2_port, &is_sent);
                        }
                        msg = Free(msg);
                        continue;
                    }

                    // check if the game is over
                    char status = '\0';
                    char winner = '\0';
                    if (get_game_status(board, &status, &winner) == -1) {
                        perror("get_game_status");
                        msg = Free(msg);
                        break;
                    }

                    // check the status of the game
                    if (status == 'W') {
                        // get the winner's index
                        size_t winner_index = 0;
                        if (winner == role[index]) {
                            winner_index = index;
                        } else {
                            winner_index = 1 - index;
                        }

                        // send PROTOCOL[9] to the winner and send PROTOCOL[10] to the loser
                        if (send_message(sockets[winner_index], PROTOCOL[9], strlen(PROTOCOL[9])) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        if (winner_index == 0) {
                            log_message(PROTOCOL[9], client1_host, client1_port, &is_sent);
                        } else {
                            log_message(PROTOCOL[9], client2_host, client2_port, &is_sent);
                        }
                        if (send_message(sockets[1 - winner_index], PROTOCOL[10], strlen(PROTOCOL[10])) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        if (winner_index == 0) {
                            log_message(PROTOCOL[10], client2_host, client2_port, &is_sent);
                        } else {
                            log_message(PROTOCOL[10], client1_host, client1_port, &is_sent);
                        }
                        msg = Free(msg);
                        break;
                    } else if (status == 'D') {
                        // if the game is a draw, then send PROTOCOL[14] to both clients
                        if (send_message(sockets[0], PROTOCOL[14], strlen(PROTOCOL[14])) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        log_message(PROTOCOL[14], client1_host, client1_port, &is_sent);
                        if (send_message(sockets[1], PROTOCOL[14], strlen(PROTOCOL[14])) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        log_message(PROTOCOL[14], client2_host, client2_port, &is_sent);
                        msg = Free(msg);
                        break;
                    } else {
                        // if the game is not over, then generate MOVD message
                        char *movd_msg = NULL;
                        if (generate_MOVD(board, rol, row, col, &movd_msg) == -1) {
                            perror("generate_MOVD");
                            msg = Free(msg);
                            break;
                        }

                        // send the MOVD message to both clients
                        if (send_message(sockets[0], movd_msg, strlen(movd_msg)) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        log_message(movd_msg, client1_host, client1_port, &is_sent);
                        if (send_message(sockets[1], movd_msg, strlen(movd_msg)) == -1) {
                            perror("send_message");
                            msg = Free(msg);
                            break;
                        }
                        log_message(movd_msg, client2_host, client2_port, &is_sent);

                        // free the movd_msg variable
                        movd_msg = Free(movd_msg);

                        // update the turn
                        turn = 1 - turn;
                    }
                } else {
                    // if it is not the client's turn, then send PROTOCOL[3] to the same client
                    if (send_message(sockets[index], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                        perror("send_message");
                        msg = Free(msg);
                        break;
                    }
                    if (index == 0) {
                        log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
                    } else {
                        log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
                    }
                }
            }
        }

        // if none of the above messages were sent, then send PROTOCOL[3] to the same client
        if (is_valid_msg == 0) {
            if (send_message(sockets[index], PROTOCOL[3], strlen(PROTOCOL[3])) == -1) {
                perror("send_message");
                msg = Free(msg);
                break;
            }
            if (index == 0) {
                log_message(PROTOCOL[3], client1_host, client1_port, &is_sent);
            } else {
                log_message(PROTOCOL[3], client2_host, client2_port, &is_sent);
            }
        }

        // free the msg variable
        msg = Free(msg);
    }

    // before exiting, free the game struct and any other dynamically allocated memory
    free_game(args);
    pthread_exit(NULL);
}

// function that frees the game struct
// returns NULL
void free_game(game *arg) {
    // input validation
    if (arg == NULL) {
        return;
    }

    // make sure to remove the player names from the shared list of players
    remove_player_name(arg->player1_name);
    remove_player_name(arg->player2_name);

    // close the sockets and free the host and port strings
    if (close(arg->client1_socket) == -1) {
        perror("close");
    }
    if (close(arg->client2_socket) == -1) {
        perror("close");
    }
    arg->client1_host = Free(arg->client1_host);
    arg->client1_port = Free(arg->client1_port);
    arg->player1_name = Free(arg->player1_name);
    arg->msg_buffer1 = Free(arg->msg_buffer1);
    arg->client2_host = Free(arg->client2_host);
    arg->client2_port = Free(arg->client2_port);
    arg->player2_name = Free(arg->player2_name);
    arg->msg_buffer2 = Free(arg->msg_buffer2);

    // free the game struct
    Free(arg);
}

// function that writes the status of the game ("W" or "D" or "N") and the winner ("X" or "O") if there is one
// returns -1 on error, 0 on success
ssize_t get_game_status(const char *board, char *status, char *winner) {
    // input validation
    if (board == NULL || status == NULL || winner == NULL || strlen(board) != 9) {
        return -1;
    }

    // check if there is a winner horizontally
    for (size_t i = 0; i < 9; i += 3) {
        // make sure none of the spaces have a period (empty space)
        if (board[i] != '.' && board[i] == board[i + 1] && board[i] == board[i + 2]) {
            *status = 'W';
            *winner = board[i];
            return 0;
        }
    }

    // check if there is a winner vertically
    for (size_t i = 0; i < 3; i++) {
        // make sure none of the spaces have a period (empty space)
        if (board[i] != '.' && board[i] == board[i + 3] && board[i] == board[i + 6]) {
            *status = 'W';
            *winner = board[i];
            return 0;
        }
    }

    // check if there is a winner diagonally
    if (board[0] != '.' && board[0] == board[4] && board[0] == board[8]) {
        *status = 'W';
        *winner = board[0];
        return 0;
    }
    if (board[2] != '.' && board[2] == board[4] && board[2] == board[6]) {
        *status = 'W';
        *winner = board[2];
        return 0;
    }

    // check if there is a draw
    for (size_t i = 0; i < 9; i++) {
        // if there is an empty space, then there is no draw
        if (board[i] == '.') {
            *status = 'N';
            *winner = '.';
            return 0;
        }
    }

    // if there is no winner and no empty spaces, then there is a draw
    *status = 'D';
    *winner = '.';
    return 0;
}

// function that makes a move on the board
// returns -1 on error, 0 on success
ssize_t make_move(char *board, char role, size_t row, size_t col) {
    // input validation
    if (board == NULL || strlen(board) != 9) {
        return -1;
    }

    // check role
    if (role != 'X' && role != 'O') {
        return -1;
    }

    // check row and col
    if (row > 2 || col > 2) {
        return -1;
    }

    // check if the space is empty
    if (board[row * 3 + col] != '.') {
        return -1;
    }

    // make the move
    board[row * 3 + col] = role;

    return 0;
}

// function that generates a MOVD message
// returns -1 on error, 0 on success
ssize_t generate_MOVD(const char *board, char role, size_t row, size_t col, char **movd_msg) {
    // input validation
    if (board == NULL || strlen(board) != 9 || movd_msg == NULL) {
        return -1;
    }

    // check role
    if (role != 'X' && role != 'O') {
        return -1;
    }

    // check row and col
    if (row > 2 || col > 2) {
        return -1;
    }

    // check if the space is empty
    if (board[row * 3 + col] != role) {
        return -1;
    }

    // generate the message
    char *message = malloc(strlen(PROTOCOL[1]) + 17);
    if (message == NULL) {
        return -1;
    }
    strcpy(message, PROTOCOL[1]);
    message[8] = role;
    message[9] = '|';
    message[10] = row + 1 + '0'; // NOLINT(cppcoreguidelines-narrowing-conversions)
    message[11] = ',';
    message[12] = col + 1 + '0'; // NOLINT(cppcoreguidelines-narrowing-conversions)
    message[13] = '|';
    message[14] = '\0';
    strcat(message, board);
    strcat(message, "|");

    // set the movd_msg pointer to point to the message
    *movd_msg = message;
    return 0;
}

// function that generates a BEGN message
// returns -1 on error, 0 on success
ssize_t generate_BEGN(char role, const char *opponent_name, char **begn_msg) {
    // input validation
    if (role != 'X' && role != 'O') {
        return -1;
    }
    if (opponent_name == NULL || strlen(opponent_name) == 0) {
        return -1;
    }

    // calculate number of remaining bytes
    size_t remaining_bytes = 2 + strlen(opponent_name) + 1;

    // calculate the number of digits in remaining_bytes
    size_t num_digits = 0;
    size_t temp = remaining_bytes;
    while (temp > 0) {
        temp /= 10;
        num_digits++;
    }

    // calculate size of the message BEGN|6|X|bar| + 1 for null terminator
    size_t size = strlen(PROTOCOL[8]) + num_digits + 1 + 2 + strlen(opponent_name) + 1 + 1;

    // example of message is BEGN|6|X|bar|, where 6 is the remaining number of bytes after "|" and "X" is the role
    // and bar is the opponent's name

    // allocate memory for the message
    char *message = malloc(size);
    if (message == NULL) {
        return -1;
    }

    // generate the message
    strcpy(message, PROTOCOL[8]);
    // convert remaining_bytes to a string
    char *remaining_bytes_str = malloc(num_digits + 1);
    if (remaining_bytes_str == NULL) {
        Free(message);
        return -1;
    }
    sprintf(remaining_bytes_str, "%zu", remaining_bytes);
    strcat(message, remaining_bytes_str);
    Free(remaining_bytes_str);
    strcat(message, "|");
    size_t intermediate_size = strlen(message);
    message[intermediate_size] = role;
    message[intermediate_size + 1] = '\0';
    strcat(message, "|");
    strcat(message, opponent_name);
    strcat(message, "|");

    // set the begn_msg pointer to point to the message
    *begn_msg = message;
    return 0;
}


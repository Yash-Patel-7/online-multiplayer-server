#ifndef P3_MSG_H
#define P3_MSG_H

#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include "helper.h"
#include "net.h"

// prototypes of all functions
void log_message(const char *message, const char *host, const char *port, const size_t *is_sent);
ssize_t get_message(int socket, char **msg_buffer, char **msg);
ssize_t receive_and_add(int socket, char **msg_buffer);
ssize_t parse_play(const char *msg, char **player_name);
size_t check_protocol(const char *msg, const char *protocol);
size_t is_complete_msg(const char *msg_buffer, size_t *max_index);
ssize_t get_complete_message(char **msg_buffer, char **msg, const size_t *max_size);
size_t check_num_of_bars(const char *msg_buffer, const char* protocol, const size_t *num_of_remaining_bytes, const size_t *num_of_digits, size_t *max_size);
ssize_t get_remaining_bytes(const char *msg_buffer, size_t *num_of_remaining_bytes, size_t *num_of_digits);
ssize_t to_unsigned_long(const char* str_num, size_t *num_of_remaining_bytes);
ssize_t translate_protocol(const char* protocol, size_t *code);
ssize_t parse_move(const char *msg, char *role, size_t *row, size_t *col);
ssize_t parse_rsgn(const char *msg);
ssize_t parse_draw(const char *msg, char *action);


// function that logs a message to STDOUT for the server
void log_message(const char *message, const char *host, const char *port, const size_t *is_sent) {
    // log the message in format [SERVER] [CLIENT host:port] [SENT or RECV] [message]
    size_t log_length =
            strlen("[SERVER] [CLIENT :] [SENT] []") +
            strlen(host) +
            strlen(port) +
            strlen(message) + 2;

    char *log = malloc(sizeof(char) * log_length);

    if (log == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // use string concatenation to create the log message
    strcpy(log, "[SERVER] [CLIENT ");
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

// function that gets the next message from the socket
// returns -1 on error and 0 on success
ssize_t get_message(int socket, char **msg_buffer, char **msg) {
    // input validation
    if (msg_buffer == NULL || msg == NULL || socket < 0) {
        return -1;
    }

    size_t max_index = 0;

    // write complete message if there is one
    if (is_complete_msg(*msg_buffer, &max_index) == 1) {
        if (get_complete_message(msg_buffer, msg, &max_index) == -1) {
            return -1;
        } else {
            return 0;
        }
    }

    // if there is nothing in the buffer
    // receive the first message from the socket and add it to the message buffer (indefinitely blocking)
    if (*msg_buffer == NULL || strlen(*msg_buffer) == 0) {
        if (receive_and_add(socket, msg_buffer) == -1) {
            return -1;
        }
    }

    // get all the messages from the socket within the timeout period
    while (1) {
        ssize_t index = get_readable_socket(&socket, 1, 501);
        if (index == -1) {
            break;
        } else {
            if (receive_and_add(socket, msg_buffer) == -1) {
                break;
            }
        }
    }

    // if there is no complete message after receiving all packets, then it is a malformed message
    if (is_complete_msg(*msg_buffer, &max_index) == 0) {
        *msg_buffer = Free(*msg_buffer);
        return -1;
    }

    // write the complete message
    if (get_complete_message(msg_buffer, msg, &max_index) == -1) {
        return -1;
    } else {
        return 0;
    }
}

// function that receives a message from the socket and adds it to the buffer
// returns -1 on error and 0 on success
ssize_t receive_and_add(int socket, char **msg_buffer) {
    if (socket < 0 || msg_buffer == NULL) {
        return -1;
    }
    size_t length = 0;
    char *msg = receive_message(socket, &length);
    if (msg == NULL) {
        return -1;
    }
    if (*msg_buffer == NULL) {
        *msg_buffer = strdup("");
    }
    char *new_buffer = malloc(strlen(*msg_buffer) + length + 1);
    if (new_buffer == NULL) {
        Free(msg);
        return -1;
    }
    strcpy(new_buffer, *msg_buffer);
    strcat(new_buffer, msg);
    msg = Free(msg);
    *msg_buffer = Free(*msg_buffer);
    *msg_buffer = new_buffer;

    return 0;

}


// function that checks whether given buffer contains a complete message
size_t is_complete_msg(const char *msg_buffer, size_t *max_index) {
    // input validation
    if (msg_buffer == NULL || max_index == NULL) {
        return 0;
    }
    // no complete message if length of buffer is less than minimum number of bytes required for complete message
    if (strlen(msg_buffer) < 7) {
        return 0;
    }

    size_t remaining_bytes = 0;
    size_t num_of_digits = 0;

    // check if number of bars is correct based on protocol and is within number of specified bytes
    if (check_protocol(msg_buffer, "PLAY") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "PLAY", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "MOVE") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "MOVE", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "RSGN") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "RSGN", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "DRAW") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "DRAW", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "WAIT") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "WAIT", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "BEGN") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "BEGN", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "MOVD") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "MOVD", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "OVER") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "OVER", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (check_protocol(msg_buffer, "INVL") == 1) {
        if (get_remaining_bytes(msg_buffer, &remaining_bytes, &num_of_digits) == -1) {
            return 0;
        } else {
            if (check_num_of_bars(msg_buffer, "INVL", &remaining_bytes, &num_of_digits, max_index) == 0) {
                return 0;
            } else {
                return 1;
            }
        }
    } else {
        return 0;
    }
}

// function that gets the number of remaining bytes in the number field of the message
// returns -1 on error and 0 on success
ssize_t get_remaining_bytes(const char *msg_buffer, size_t *num_of_remaining_bytes, size_t *num_of_digits) {
    // input validation
    if (msg_buffer == NULL || strlen(msg_buffer) == 0) {
        perror("invalid input\n");
        return -1;
    }

    // check if there is a bar after the 4-character code
    if (msg_buffer[4] != '|') {
        return -1;
    }

    // tokenize buffer to get the number field of the first message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(msg_buffer, "|", &num_of_tokens, "");

    // token validation
    if (tokens == NULL || num_of_tokens < 2) {
        tokens = freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    if (strlen(tokens[1]) < 1 || strlen(tokens[1]) > 3) {
        tokens = freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    // number field of first message is contained in second token (after 4-character code)
    if (to_unsigned_long(tokens[1], num_of_remaining_bytes) == -1) {
        tokens = freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    } else if (*num_of_remaining_bytes >= 0 && *num_of_remaining_bytes < strlen(msg_buffer)) {
        *num_of_digits = strlen(tokens[1]);
        tokens = freeArrayOfStrings(tokens, num_of_tokens);
        return 0;
    }

    tokens = freeArrayOfStrings(tokens, num_of_tokens);
    return -1;
}

// function that checks if number of specified bytes if correct and within the size of
size_t check_num_of_bytes (const char *msg_buffer, const size_t *num_of_remaining_bytes) {
    return 0;
}

// function that converts char * to unsigned long
// return -1 on error and 0 on success
ssize_t to_unsigned_long(const char* str_num, size_t *num_of_remaining_bytes) {
    errno = 0;
    size_t converted_num = strtoull(str_num, NULL, 10);
    if (errno != 0) {
        return -1;
    } else {
        *num_of_remaining_bytes = converted_num;
        return 0;
    }
}

// function that checks for the correct number of bars based on the given protocol within the given number of bytes
size_t check_num_of_bars(const char *msg_buffer, const char* protocol, const size_t *num_of_remaining_bytes, const size_t *num_of_digits, size_t *max_size) {
    // input validation
    if (msg_buffer == NULL || strlen(msg_buffer) == 0) {
        return 0;
    }
    if (protocol == NULL || strlen(protocol) == 0) {
        return 0;
    }

    size_t bar_count = 0;
    size_t code = 0;

    // translate protocol to a code
    if (translate_protocol(protocol, &code) == -1) {
        return 0;
    }

    // assign correct number of bars based on the protocol
    size_t correct_num_of_bars = 0;
    size_t overlap_bars = 0;
    if (code == 0 || code == 3 || code == 7) {
        correct_num_of_bars = 3;
        overlap_bars = 1;
    } else if (code == 1 || code == 5|| code == 8) {
        correct_num_of_bars = 4;
        overlap_bars = 2;
    } else if (code == 2 || code == 4) {
        correct_num_of_bars = 2;
        overlap_bars = 0;
    } else if (code == 6) {
        correct_num_of_bars = 5;
        overlap_bars = 3;
    }
    // get the maximum number of bytes of a complete message
    *max_size = 4 + *num_of_digits + *num_of_remaining_bytes + correct_num_of_bars - overlap_bars;

    if (*max_size > strlen(msg_buffer)) {
        return 0;
    }

    // check if the number of bars is correct within the specified number of bytes
    for (size_t i = 0; i < *max_size; i++) {
        if (msg_buffer[i] == '|') {
            bar_count++;
        }
    }

    // if we got the specified number of bytes and the right number of bars (and the last char was a bar),
    // then the message is complete
    if ((bar_count != correct_num_of_bars) || (msg_buffer[*max_size - 1] != '|')) {
        return 0;
    } else {
        return 1;
    }

}

// function that translates protocol to a code
// returns -1 on error and 0 on success
ssize_t translate_protocol(const char* protocol, size_t *code) {
    if (strcmp(protocol, "PLAY") == 0) {
        *code = 0;
    } else if (strcmp(protocol, "MOVE") == 0) {
        *code = 1;
    } else if (strcmp(protocol, "RSGN") == 0) {
        *code = 2;
    } else if (strcmp(protocol, "DRAW") == 0) {
        *code = 3;
    } else if (strcmp(protocol, "WAIT") == 0) {
        *code = 4;
    } else if (strcmp(protocol, "BEGN") == 0) {
        *code = 5;
    } else if (strcmp(protocol, "MOVD") == 0) {
        *code = 6;
    } else if (strcmp(protocol, "INVL") == 0) {
        *code = 7;
    } else if (strcmp(protocol, "OVER") == 0) {
        *code = 8;
    } else {
        return -1;
    }
    return 0;
}


// function that writes the complete message and updates buffer
// returns 0 on success and -1 on error
ssize_t get_complete_message(char **msg_buffer, char **msg, const size_t *max_size) {
    //input validation
    if (msg_buffer == NULL || *msg_buffer == NULL || msg == NULL || max_size == NULL) {
        return -1;
    }

    // an allocated string to store the complete message
    char *complete_msg = malloc((*max_size + 1) * sizeof(char));

    // copy the complete message from the message buffer to the allocated string
    strncpy(complete_msg, *msg_buffer, *max_size);

    complete_msg[*max_size] = '\0';

    *msg = complete_msg;

    // if there is no leftover data in the message buffer after the complete message, then clear the message buffer
    if (*max_size == strlen(*msg_buffer)) {
        *msg_buffer = Free(*msg_buffer);
        return 0;
    }

    // if there is leftover data in the message buffer after the complete message, update the message buffer by using memmove
    // to move the partial message at the end of the buffer to the beginning
    size_t leftover_length = strlen(*msg_buffer) - *max_size;
    memmove(*msg_buffer, *msg_buffer + *max_size, leftover_length + 1);

    // ex: MOVE\06|X|2,2|MOVE\0
    // MOVE|6|X|2,2| is the message which you write as a new allocated string
    // MOVE is the partial message
    // make the message buffer MOVE

    return 0;
}

// function that checks whether the message is of a specific protocol
// msg is the message that has arrived and protocol is the name of the command (ex: "PLAY", "MOVE")
size_t check_protocol(const char *msg, const char *protocol) {
    if (msg == NULL || protocol == NULL || strlen(msg) == 0 || strlen(protocol) == 0) {
        return 0;
    }
    size_t num_of_tokens = 0;

    char **tokens = strTokenize(msg, "|", &num_of_tokens, "");
    if (tokens == NULL) {
        return 0;
    }
    if (strcmp(tokens[0], protocol) == 0) {
        tokens = freeArrayOfStrings(tokens, num_of_tokens);
        return 1;
    }
    tokens = freeArrayOfStrings(tokens, num_of_tokens);
    return 0;
}


// function that writes the player_name
// returns -1 on error and 0 on success
ssize_t parse_play(const char *msg, char **player_name) {
    // input validation
    if (msg == NULL || player_name == NULL || strlen(msg) == 0) {
        return -1;
    }

    // check if protocol given is play message
    if (check_protocol(msg, "PLAY") == 0) {
        return -1;
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        return -1;
    }

    // write name field to player name
    *player_name = strdup(tokens[2]);

    // free tokens
    freeArrayOfStrings(tokens, num_of_tokens);

    return 0;
}

ssize_t parse_move(const char *msg, char *role, size_t *row, size_t *col) {
    // input validation
    if (msg == NULL || role == NULL || row == NULL || col == NULL || strlen(msg) == 0) {
        return -1;
    }

    // check if protocol given is move message
    if (check_protocol(msg, "MOVE") == 0) {
        return -1;
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        return -1;
    }

    if (num_of_tokens != 4) {
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    // write to role
    if ((strcmp(tokens[2], "X") != 0) && (strcmp(tokens[2], "O") != 0)) {
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    } else {
        *role = tokens[2][0];
    }

    size_t num_of_tokens_comma = 0;
    char **tokens_comma = strTokenize(tokens[3], ",", &num_of_tokens, "");
    if (tokens_comma == NULL) {
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }
    if (num_of_tokens != 2) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    // extract the row and col
    if (to_unsigned_long(tokens_comma[0], row) == -1) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    if (*row < 1 || *row > 3) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    *row -= 1;

    if (to_unsigned_long(tokens_comma[1], col) == -1) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    if (*col < 1 || *col > 3) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    *col -= 1;

    freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
    freeArrayOfStrings(tokens, num_of_tokens);

    return 0;
}

ssize_t parse_rsgn(const char *msg) {
    // input validation
    if (msg == NULL || strlen(msg) == 0) {
        return -1;
    }

    // check if protocol given is rsgn message
    if (check_protocol(msg, "RSGN") == 0) {
        return -1;
    }

    return 0;
}

ssize_t parse_draw(const char *msg, char *action) {
    // input validation
    if (msg == NULL || strlen(msg) == 0) {
        return -1;
    }

    // check if protocol given is draw message
    if (check_protocol(msg, "DRAW") == 0) {
        return -1;
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        return -1;
    }

    if (num_of_tokens != 3) {
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    if ((strcmp(tokens[2], "S") != 0) && (strcmp(tokens[2], "R") != 0) && (strcmp(tokens[2], "A") != 0)) {
        freeArrayOfStrings(tokens, num_of_tokens);
        return -1;
    }

    *action = tokens[2][0];

    freeArrayOfStrings(tokens, num_of_tokens);
    return 0;
}

#endif //P3_MSG_H

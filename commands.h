#ifndef P3_COMMANDS_H
#define P3_COMMANDS_H

#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include "msg.h"

// prototypes for functions
ssize_t check_connection_drop(int client_socket);
void emergency(int client_socket, char *msg);
int get_client_socket(const char *host, const char *port);
void p_recv(int client_socket, char** msg, char **buffer);
void p_play(int client_socket, const char *bytes, const char *player_name);
void p_move(int client_socket, const char *bytes, const char *role, const char *coordinates);
void p_draw(int client_socket, const char *bytes, const char *action);
void p_rsgn(int client_socket, const char *bytes);
void p_movd(int client_socket, const char *bytes);
void p_wait(int client_socket, const char *bytes);
void p_begn(int client_socket, const char *bytes);
void p_invl(int client_socket, const char *bytes);
void p_over(int client_socket, const char *bytes);
void parse_wait(int client_socket, char **buffer);
void parse_begn(int client_socket, const char *correct_role, const char *opponent_name, char **buffer);
void parse_movd(int client_socket, const char *correct_role, const char *correct_row, const char *correct_col, const char *correct_board, char **buffer);
void parse_invl(int client_socket, char *correct_error, char **buffer);
void parse_over(int client_socket, const char *correct_outcome, const char *correct_explanation, char **buffer);
void parse_draw_from_server(int client_socket, const char* correct_action, char **buffer);

// function that checks if the connection is dropped
ssize_t check_connection_drop(int client_socket) {
    char *buffer = NULL;
    char *msg = NULL;

    if (get_message(client_socket, &buffer, &msg) == 0) {
        return -1;
    }

    return 0;
}

// function that gets client socket
int get_client_socket(const char *host, const char *port) {
    int client_socket = create_client_socket(host, port);
    if (client_socket == -1) {
        perror("create_client_socket");
        pthread_exit(NULL);
    }
    return client_socket;
}

void p_recv(int client_socket, char** msg, char **buffer) {
    if (get_message(client_socket, buffer, msg) == -1) {
        perror("RECV: cannot get message");
        pthread_exit(NULL);
    }
}

void p_play(int client_socket, const char *bytes, const char *player_name) {
    // allocate enough memory to hold the combined message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "PLAY|");

    // concatenate the rest of the fields to form the combined message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, player_name);
    strcat(combined_msg, "|");

	// send the message to server
    size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("PLAY: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_move(int client_socket, const char *bytes, const char *role, const char *coordinates) {
	// allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "MOVE|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, role);
    strcat(combined_msg, "|");
	strcat(combined_msg, coordinates);
    strcat(combined_msg, "|");

	// send the message to server
    size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("MOVE: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_draw(int client_socket, const char *bytes, const char *action) {
	// allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "DRAW|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, action);
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("DRAW: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_rsgn(int client_socket, const char *bytes) {
	// allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "RSGN|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("RSGN: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

// Allow for client to send messages typically sent by server for error checking

void p_movd(int client_socket, const char *bytes) {
    // allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "MOVD|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, "X|2,2|...X...O.");
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("MOVD: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_wait(int client_socket, const char *bytes) {
    // allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "WAIT|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("WAIT: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_begn(int client_socket, const char *bytes) {
    // allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "BEGN|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, "X|Opponent");
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("BEGN: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_invl(int client_socket, const char *bytes) {
    // allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "INVL|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, "!Protocol error.");
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("INVL: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

void p_over(int client_socket, const char *bytes) {
    // allocate enough memory to hold the combined_msg message that will be sent to the server
    char* combined_msg = malloc(101);

    // copy the first field into the dynamically allocated array
    strcpy(combined_msg, "OVER|");

    // concatenate the rest of the fields to form the combined_msg message
    strcat(combined_msg, bytes);
    strcat(combined_msg, "|");
    strcat(combined_msg, "L");
    strcat(combined_msg, "|");
    strcat(combined_msg, "One player has resigned.");
    strcat(combined_msg, "|");

	// send the message to server
     size_t result = send_message(client_socket, combined_msg, strlen(combined_msg));
	if (result == -1) {
		perror("OVER: connection dropped");
		pthread_exit(NULL);
	}

    // free combined_msg
    combined_msg = Free(combined_msg);

	
}

// parse through messages received by server

// function that checks if message is WAIT message
// returns -1 on error and 0 on success
void parse_wait(int client_socket, char **buffer) {
    char *recvd_msg = NULL;

    p_recv(client_socket, &recvd_msg, buffer);

    // check if protocol given is wait message
    if (check_protocol(recvd_msg, "WAIT") == 0) {
        perror("WAIT: expected WAIT");
        pthread_exit(NULL);
    }

    
}

// function that checks if received player name is equal to opponent's player name
// returns -1 on error and 0 on success
void parse_begn(int client_socket, const char *correct_role, const char *opponent_name, char **buffer) {
    // input validation
    if (opponent_name == NULL) {
        perror("BEGN: input validation");
        pthread_exit(NULL);
    }

    char *recvd_msg = NULL;

    p_recv(client_socket, &recvd_msg, buffer);

    // check if protocol given is begn message
    if (check_protocol(recvd_msg, "BEGN") == 0) {
        perror("BEGN: expected BEGN");
        pthread_exit(NULL);
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(recvd_msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        perror("BEGN: not tokenized");
        pthread_exit(NULL);
    }

    // check if received player role is equal to correct player role
    if (strcmp(tokens[2], correct_role) != 0) {
        perror("BEGN: not correct role");
        pthread_exit(NULL);
    }

    // check if received player name is equal to opponent's player name
    if (strcmp(tokens[3], opponent_name) != 0) {
        perror("BEGN: not correct opponent name");
        pthread_exit(NULL);
    }

    // free tokens
    freeArrayOfStrings(tokens, num_of_tokens);

    
}

// function that checks if received MOVD message contains correct information in its fields
// returns -1 on error and 0 on success
void parse_movd(int client_socket, const char *correct_role, const char *correct_row, const char *correct_col, const char *correct_board, char **buffer) {
    // input validation
    if (correct_role == NULL || correct_row == NULL || correct_col == NULL) {
        perror("MOVD: input validation");
        pthread_exit(NULL);
    }

    char *recvd_msg = NULL;

    p_recv(client_socket, &recvd_msg, buffer);

    // check if protocol given is movd message
    if (check_protocol(recvd_msg, "MOVD") == 0) {
        perror("MOVD: expected MOVD");
        pthread_exit(NULL);
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(recvd_msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        perror("MOVD: not tokenized");
        pthread_exit(NULL);
    }

    // check if number of fields is 5
    if (num_of_tokens != 5) {
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: not correct tokens");
        pthread_exit(NULL);
    }

    // write to role
    char *role = NULL;
    if ((strcmp(tokens[2], "X") != 0) && (strcmp(tokens[2], "O") != 0)) {
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: not X or O");
        pthread_exit(NULL);
    } else {
        role = tokens[2];
    }

    // check if role from server is correct role
    if (strcmp(role, correct_role) != 0) {
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: not correct role");
        pthread_exit(NULL);
    }

    size_t num_of_tokens_comma = 0;
    char **tokens_comma = strTokenize(tokens[3], ",", &num_of_tokens, "");
    if (tokens_comma == NULL) {
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: coordinates not tokenized");
        pthread_exit(NULL);
    }
    if (num_of_tokens != 2) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: incorrect number of coordinate fields");
        pthread_exit(NULL);
    }

    // check if row from server is correct row
    if (strcmp(tokens_comma[0], correct_row) != 0) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: not correct row");
        pthread_exit(NULL);
    }


    // check if column from server is correct column
    if (strcmp(tokens_comma[1], correct_col) != 0) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: not correct row");
        pthread_exit(NULL);
    }

    // check if board from server is the correct board
    if (strcmp(tokens[4], correct_board) != 0) {
        freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("MOVD: not correct board");
        pthread_exit(NULL);
    }
    
    freeArrayOfStrings(tokens_comma, num_of_tokens_comma);
    freeArrayOfStrings(tokens, num_of_tokens);

    
}

// function that checks if received INVL message contains correct error
// returns -1 on error and 0 on success
void parse_invl(int client_socket, char *correct_error, char **buffer) {
    // input validation
    if (correct_error == NULL) {
        perror("INVL: input validation");
        pthread_exit(NULL);
    }

    char *recvd_msg = NULL;

    p_recv(client_socket, &recvd_msg, buffer);

    // check if protocol given is invl message
    if (check_protocol(recvd_msg, "INVL") == 0) {
        perror("INVL: expected INVL");
        pthread_exit(NULL);
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(recvd_msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        perror("INVL: not tokenized");
        pthread_exit(NULL);
    }

    // check if received error is equal to correct error
    if (strcmp(tokens[2], correct_error) != 0) {
        perror("INVL: not correct error");
        pthread_exit(NULL);
    }

    // free tokens
    freeArrayOfStrings(tokens, num_of_tokens);

    
}

// function that checks if received OVER message contains correct outcome and explanation
// returns -1 on error and 0 on success
void parse_over(int client_socket, const char *correct_outcome, const char *correct_explanation, char **buffer) {
    // input validation
    if (correct_outcome == NULL || correct_explanation == NULL) {
        perror("OVER: input validation");
        pthread_exit(NULL);
    }

    char *recvd_msg = NULL;

    p_recv(client_socket, &recvd_msg, buffer);

    // check if protocol given is OVER message
    if (check_protocol(recvd_msg, "OVER") == 0) {
        perror("OVER: expected OVER");
        pthread_exit(NULL);
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(recvd_msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        perror("OVER: not tokenized");
        pthread_exit(NULL);
    }

    // check if received outcome is equal to correct outcome
    if (strcmp(tokens[2], correct_outcome) != 0) {
        perror("OVER: incorrect outcome");
        pthread_exit(NULL);
    }

    // check if received explanation is equal to correct explanation
    if (strcmp(tokens[3], correct_explanation) != 0) {
        perror("OVER: incorrect explanation");
        pthread_exit(NULL);
    }
    

    // free tokens
    freeArrayOfStrings(tokens, num_of_tokens);

    
}

// function that checks if recieved DRAW message contains the correct action
// returns -1 on error and 0 on success
void parse_draw_from_server(int client_socket, const char* correct_action, char **buffer) {
    // input validation
    if (correct_action == NULL) {
        perror("DRAW: input validation");
        pthread_exit(NULL);
    }

    char *recvd_msg = NULL;

    p_recv(client_socket, &recvd_msg, buffer);

    // check if protocol given is draw message
    if (check_protocol(recvd_msg, "DRAW") == 0) {
        perror("DRAW: expected DRAW");
        pthread_exit(NULL);
    }

    // tokenize the message
    size_t num_of_tokens = 0;
    char **tokens = strTokenize(recvd_msg, "|", &num_of_tokens, "");
    if (tokens == NULL || num_of_tokens == 0) {
        perror("DRAW: not tokenized");
        pthread_exit(NULL);
    }

    if (num_of_tokens != 3) {
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("DRAW: incorrect tokens");
        pthread_exit(NULL);
    }

    // check if received action in DRAW message is the correct action
    if (strcmp(tokens[2], correct_action) != 0) {
        freeArrayOfStrings(tokens, num_of_tokens);
        perror("DRAW: incorrect action");
        pthread_exit(NULL);
    }

    freeArrayOfStrings(tokens, num_of_tokens);
    
}

void emergency(int client_socket, char *msg) {
    if (send_message(client_socket, msg, strlen(msg)) == -1) {
        perror("could not send emergency message");
        pthread_exit(NULL);
    }
}

#endif //P3_COMMANDS_H

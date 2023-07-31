#include <time.h>
#include "commands.h"

// prototypes for all functions
void obtain_mutex_lock(pthread_mutex_t *mutex);
void release_mutex_lock(pthread_mutex_t *mutex);
void check_arguments(int argc);
void thread_create(pthread_t *thread, void *(*routine)(void *));
void thread_detach(pthread_t *thread);
void* A_client_1(void *arg);
void* A_client_2(void *arg);
void* A_client_3(void *arg);
void* A_client_4(void *arg);
void* A_client_5(void *arg);
void* A_client_6(void *arg);
void* A_client_7(void *arg);
void* A_client_8(void *arg);
void* B_client_1(void *arg);
void* B_client_2(void *arg);
void* C_client_1(void *arg);
void* C_client_2(void *arg);
void* D_client_1(void *arg);
void* D_client_2(void *arg);
void* E_client_1(void *arg);
void* E_client_2(void *arg);

char *host = NULL;
char *port = NULL;

static pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;

size_t game_role = 0;

size_t should_exit = 0;


// driver
int main(int argc, char **argv) {
    // set stdout and stderr buffer to NULL
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    // check if the arguments are correct
    check_arguments(argc);

    // global variables for host and port
    host = argv[1];
    port = argv[2];

    pthread_t thread;

    // A client 1
    thread_create(&thread, &A_client_1);
    thread_detach(&thread);


    // A client 2
    thread_create(&thread, &A_client_2);
    thread_detach(&thread);

    // A client 3
    thread_create(&thread, &A_client_3);
    thread_detach(&thread);


    // A client 4
    thread_create(&thread, &A_client_4);
    thread_detach(&thread);

    // A client 5
    thread_create(&thread, &A_client_5);
    thread_detach(&thread);


    // A client 6
    thread_create(&thread, &A_client_6);
    thread_detach(&thread);

    // A client 7
    thread_create(&thread, &A_client_7);
    thread_detach(&thread);

    // A client 8
    thread_create(&thread, &A_client_8);
    thread_detach(&thread);

    // B client 1
    thread_create(&thread, &B_client_1);
    thread_detach(&thread);

    // B client 2
    thread_create(&thread, &B_client_2);
    thread_detach(&thread);

    // C client 1
    thread_create(&thread, &C_client_1);
    thread_detach(&thread);

    // C client 2
    thread_create(&thread, &C_client_2);
    thread_detach(&thread);

    while (1) {
        nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
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

void thread_create(pthread_t *thread, void *(*routine)(void *)) {
    if (pthread_create(thread, NULL, routine, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
}

void thread_detach(pthread_t *thread) {
    if (pthread_detach(*thread) != 0) {
        perror("pthread_detach");
        exit(EXIT_FAILURE);
    }
}

void* A_client_1(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 0) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|11|Yash Patel|
    p_play(client_socket, "11", "Yash Patel");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "X", "Jasmit Singh", &buffer);

    // MOVE|6|X|2,2|
    p_move(client_socket, "6", "X", "2,2");
    parse_movd(client_socket, "X", "2", "2", "....X....", &buffer);

    // Opponent: MOVE|6|O|3,3|
    parse_movd(client_socket, "O", "3", "3", "....X...O", &buffer);

    // MOVE|6|X|1,2|
    p_move(client_socket, "6", "X", "1,2");
    parse_movd(client_socket, "X", "1", "2", ".X..X...O", &buffer);

    // Opponent: MOVE|6|O|3,2|
    parse_movd(client_socket, "O", "3", "2", ".X..X..OO", &buffer);

    // MOVE|6|X|1,1|
    p_move(client_socket, "6", "X", "1,1");
    parse_movd(client_socket, "X", "1", "1", "XX..X..OO", &buffer);

    //Opponent: DRAW|2|S|
    parse_draw_from_server(client_socket, "S", &buffer);

    // DRAW|2|R|
    p_draw(client_socket, "2", "R");

    // [OUT OF TURN] DRAW|2|S|
    p_draw(client_socket, "2", "S");

    // Opponent: DRAW|2|A|
    parse_over(client_socket, "D", "Both players declared a draw.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 1: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);
    pthread_exit(NULL);
}

void* A_client_2(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 1) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|13|Jasmit Singh|
    p_play(client_socket, "13", "Jasmit Singh");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "O", "Yash Patel", &buffer);

    // Opponent: MOVE|6|X|2,2|
    parse_movd(client_socket, "X", "2", "2", "....X....", &buffer);

    // MOVE|6|O|3,3|
    p_move(client_socket, "6", "O", "3,3");
    parse_movd(client_socket, "O", "3", "3", "....X...O", &buffer);

    // Opponent: MOVE|6|X|1,2|
    parse_movd(client_socket, "X", "1", "2", ".X..X...O", &buffer);

    // MOVE|6|O|3,2|
    p_move(client_socket, "6", "O", "3,2");
    parse_movd(client_socket, "O", "3", "2", ".X..X..OO", &buffer);

    // Opponent: MOVE|6|X|1,1|
    parse_movd(client_socket, "X", "1", "1", "XX..X..OO", &buffer);

    // DRAW|2|S|
    p_draw(client_socket, "2", "S");

    // Opponent: DRAW|2|R|
    parse_draw_from_server(client_socket, "R", &buffer);

    // [OUT OF TURN] Opponent: DRAW|2|S|
    parse_draw_from_server(client_socket, "S", &buffer);

    // DRAW|2|A|
    p_draw(client_socket, "2", "A");
    parse_over(client_socket, "D", "Both players declared a draw.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 2: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}


void* A_client_3(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 2) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|8|Barbara|
    p_play(client_socket, "8", "Barbara");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "X", "James", &buffer);

    // MOVE|6|X|1,2|
    p_move(client_socket, "6", "X", "1,2");
    parse_movd(client_socket, "X", "1", "2", ".X.......", &buffer);

    // Opponent: MOVE|6|O|2,1|
    parse_movd(client_socket, "O", "2", "1", ".X.O.....", &buffer);

    // MOVE|6|X|3,1|
    p_move(client_socket, "6", "X", "3,1");
    parse_movd(client_socket, "X", "3", "1", ".X.O..X..", &buffer);

    // Opponent: MOVE|6|O|1,3|
    parse_movd(client_socket, "O", "1", "3", ".XOO..X..", &buffer);

    // MOVE|6|X|2,2|
    p_move(client_socket, "6", "X", "2,2");
    parse_movd(client_socket, "X", "2", "2", ".XOOX.X..", &buffer);

    // Opponent: MOVE|6|O|1,1|
    parse_movd(client_socket, "O", "1", "1", "OXOOX.X..", &buffer);

    // RSGN|0|
    p_rsgn(client_socket, "0");
    parse_over(client_socket, "L", "One player has resigned.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 3: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* A_client_4(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 3) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|6|James|
    p_play(client_socket, "6", "James");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "O", "Barbara", &buffer);

    // Opponent: MOVE|6|X|1,2|
    parse_movd(client_socket, "X", "1", "2", ".X.......", &buffer);

    // MOVE|6|O|2,1|
    p_move(client_socket, "6", "O", "2,1");
    parse_movd(client_socket, "O", "2", "1", ".X.O.....", &buffer);

    // Opponent: MOVE|6|X|3,1|
    parse_movd(client_socket, "X", "3", "1", ".X.O..X..", &buffer);

    // MOVE|6|O|1,3|
    p_move(client_socket, "6", "O", "1,3");
    parse_movd(client_socket, "O", "1", "3", ".XOO..X..", &buffer);

    // Opponent: MOVE|6|X|2,2|
    parse_movd(client_socket, "X", "2", "2", ".XOOX.X..", &buffer);

    // MOVE|6|O|1,1|
    p_move(client_socket, "6", "O", "1,1");
    parse_movd(client_socket, "O", "1", "1", "OXOOX.X..", &buffer);

    // Opponent: RSGN|0|
    parse_over(client_socket, "W", "One player has resigned.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 4: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* A_client_5(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 4) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|2|H|
    p_play(client_socket, "2", "H");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "X", "Yo", &buffer);

    // MOVE|6|X|1,1|
    p_move(client_socket, "6", "X", "1,1");
    parse_movd(client_socket, "X", "1", "1", "X........", &buffer);

    // Opponent: MOVE|6|O|1,2|
    parse_movd(client_socket, "O", "1", "2", "XO.......", &buffer);

    // MOVE|6|X|2,2|
    p_move(client_socket, "6", "X", "2,2");
    parse_movd(client_socket, "X", "2", "2", "XO..X....", &buffer);

    // Opponent: MOVE|6|O|1,3|
    parse_movd(client_socket, "O", "1", "3", "XOO.X....", &buffer);

    // MOVE|6|X|2,3|
    p_move(client_socket, "6", "X", "2,3");
    parse_movd(client_socket, "X", "2", "3", "XOO.XX...", &buffer);

    // Opponent: MOVE|6|O|2,1|
    parse_movd(client_socket, "O", "2", "1", "XOOOXX...", &buffer);

    // MOVE|6|X|3,2|
    p_move(client_socket, "6", "X", "3,2");
    parse_movd(client_socket, "X", "3", "2", "XOOOXX.X.", &buffer);

    // Opponent: MOVE|6|O|3,3|
    parse_movd(client_socket, "O", "3", "3", "XOOOXX.XO", &buffer);

    // MOVE|6|X|3,1|
    p_move(client_socket, "6", "X", "3,1");

    parse_over(client_socket, "D", "The grid is full.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 5: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* A_client_6(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 5) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|3|Yo|
    p_play(client_socket, "3", "Yo");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "O", "H", &buffer);

    // Opponent: MOVE|6|X|1,1|
    parse_movd(client_socket, "X", "1", "1", "X........", &buffer);

    // MOVE|6|O|1,2|
    p_move(client_socket, "6", "O", "1,2");
    parse_movd(client_socket, "O", "1", "2", "XO.......", &buffer);

    // Opponent: MOVE|6|X|2,2|
    parse_movd(client_socket, "X", "2", "2", "XO..X....", &buffer);

    // MOVE|6|O|1,3|
    p_move(client_socket, "6", "O", "1,3");
    parse_movd(client_socket, "O", "1", "3", "XOO.X....", &buffer);

    // Opponent: MOVE|6|X|2,3|
    parse_movd(client_socket, "X", "2", "3", "XOO.XX...", &buffer);

    // MOVE|6|O|2,1|
    p_move(client_socket, "6", "O", "2,1");
    parse_movd(client_socket, "O", "2", "1", "XOOOXX...", &buffer);

    // Opponent: MOVE|6|X|3,2|
    parse_movd(client_socket, "X", "3", "2", "XOOOXX.X.", &buffer);

    // MOVE|6|O|3,3|
    p_move(client_socket, "6", "O", "3,3");
    parse_movd(client_socket, "O", "3", "3", "XOOOXX.XO", &buffer);

    // Opponent: MOVE|6|X|3,1|

    parse_over(client_socket, "D", "The grid is full.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 6: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* A_client_7(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 6) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|4|Jay|
    p_play(client_socket, "4", "Jay");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "X", "Brown", &buffer);

    // MOVE|6|X|1,1|
    p_move(client_socket, "6", "X", "1,1");
    parse_movd(client_socket, "X", "1", "1", "X........", &buffer);

    // Opponent: MOVE|6|O|1,2|
    parse_movd(client_socket, "O", "1", "2", "XO.......", &buffer);

    // MOVE|6|X|2,2|
    p_move(client_socket, "6", "X", "2,2");
    parse_movd(client_socket, "X", "2", "2", "XO..X....", &buffer);

    // Opponent: MOVE|6|O|1,3|
    parse_movd(client_socket, "O", "1", "3", "XOO.X....", &buffer);

    // MOVE|6|X|2,3|
    p_move(client_socket, "6", "X", "2,3");
    parse_movd(client_socket, "X", "2", "3", "XOO.XX...", &buffer);

    // Opponent: MOVE|6|O|2,1|
    parse_movd(client_socket, "O", "2", "1", "XOOOXX...", &buffer);

    // MOVE|6|X|3,2|
    p_move(client_socket, "6", "X", "3,2");
    parse_movd(client_socket, "X", "3", "2", "XOOOXX.X.", &buffer);

    // Opponent: MOVE|6|O|3,1|
    parse_movd(client_socket, "O", "3", "1", "XOOOXXOX.", &buffer);

    // MOVE|6|X|3,3|
    p_move(client_socket, "6", "X", "3,3");

    parse_over(client_socket, "W", "One player has completed a line.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 7: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* A_client_8(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 7) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|6|Brown|
    p_play(client_socket, "6", "Brown");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "O", "Jay", &buffer);

    // Opponent: MOVE|6|X|1,1|
    parse_movd(client_socket, "X", "1", "1", "X........", &buffer);

    // MOVE|6|O|1,2|
    p_move(client_socket, "6", "O", "1,2");
    parse_movd(client_socket, "O", "1", "2", "XO.......", &buffer);

    // Opponent: MOVE|6|X|2,2|
    parse_movd(client_socket, "X", "2", "2", "XO..X....", &buffer);

    // MOVE|6|O|1,3|
    p_move(client_socket, "6", "O", "1,3");
    parse_movd(client_socket, "O", "1", "3", "XOO.X....", &buffer);

    // Opponent: MOVE|6|X|2,3|
    parse_movd(client_socket, "X", "2", "3", "XOO.XX...", &buffer);

    // MOVE|6|O|2,1|
    p_move(client_socket, "6", "O", "2,1");
    parse_movd(client_socket, "O", "2", "1", "XOOOXX...", &buffer);

    // Opponent: MOVE|6|X|3,2|
    parse_movd(client_socket, "X", "3", "2", "XOOOXX.X.", &buffer);

    // MOVE|6|O|3,1|
    p_move(client_socket, "6", "O", "3,1");
    parse_movd(client_socket, "O", "3", "1", "XOOOXXOX.", &buffer);

    // Opponent: MOVE|6|X|3,3|
    parse_over(client_socket, "L", "One player has completed a line.", &buffer);

    // close client
    close(client_socket);

    // print success
    printf("A CLIENT 8: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* B_client_1(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 8) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|10|Joe Sally|
    p_play(client_socket, "10", "Joe Sally");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "X", "Jay Brown", &buffer);

    // MOVE|6|X|1,2|
    p_move(client_socket, "6", "X", "1,2");
    parse_movd(client_socket, "X", "1", "2", ".X.......", &buffer);

    // Opponent: MOVE|6|O|1,2|

    // Opponent: MOVE|6|O|3,3|
    parse_movd(client_socket, "O", "3", "3", ".X......O", &buffer);

    // MOVE|6|X|4,2|
    p_move(client_socket, "6", "X", "4,2");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // MOVE|6|X|1,0|
    p_move(client_socket, "6", "X", "1,0");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // MOVE|6|O|2,1|
    p_move(client_socket, "6", "O", "2,1");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // MOVE|7|X|PLAY|
    p_move(client_socket, "7", "X", "PLAY");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // MOVD|16|X|1,2|.........|
    p_movd(client_socket, "16");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // INVL|17|!Protocol error.|
    p_invl(client_socket, "17");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // WAIT|0|
    p_wait(client_socket, "0");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // OVER|27|L|One player has resigned.|
    p_over(client_socket, "27");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // BEGN|11|X|Opponent|
    p_begn(client_socket, "11");
    parse_invl(client_socket, "!Protocol error.", &buffer);

    // PLAY|12|X|Joe Sally|
    emergency(client_socket, "PLAY|12|X|Joe Sally|");
    while (check_connection_drop(client_socket) != 0) {}

    // close client
    close(client_socket);

    // print success
    printf("B CLIENT 1: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* B_client_2(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 9) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|10|Jay Brown|
    p_play(client_socket, "10", "Jay Brown");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "O", "Joe Sally", &buffer);

    // Opponent: MOVE|6|X|1,2|
    parse_movd(client_socket, "X", "1", "2", ".X.......", &buffer);

    // MOVE|6|O|1,2|
    p_move(client_socket, "6", "O", "1,2");
    parse_invl(client_socket, "That space is occupied.", &buffer);

    // MOVE|6|O|3,3|
    p_move(client_socket, "6", "O", "3,3");
    parse_movd(client_socket, "O", "3", "3", ".X......O", &buffer);

    while (check_connection_drop(client_socket) != 0) {}

    // close client
    close(client_socket);

    // print success
    printf("B CLIENT 2: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* C_client_1(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 10) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|10|Jay Brown|
    p_play(client_socket, "10", "Jay Brown");
    parse_invl(client_socket, "Name already in use.", &buffer);

    // PLAY|10|Moe Gates|
    p_play(client_socket, "10", "Moe Gates");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "X", "Jays", &buffer);

    // MOVE|6|X|2,2|
    p_move(client_socket, "6", "X", "2,2");
    parse_movd(client_socket, "X", "2", "2", "....X....", &buffer);

    // Opponent: Drop connection
    while (check_connection_drop(client_socket) != 0) {}

    // close client
    close(client_socket);

    // print success
    printf("C CLIENT 1: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}

void* C_client_2(void *arg) {
    while (1) {
        obtain_mutex_lock(&game_mutex);
        if (game_role != 11) {
            release_mutex_lock(&game_mutex);
            continue;
        } else {
            game_role++;
            break;
        }
    }

    // get client socket
    int client_socket = get_client_socket(host, port);
    char *buffer = NULL;

    // PLAY|5|Jays|
    p_play(client_socket, "5", "Jays");
    parse_wait(client_socket, &buffer);

    // release the game lock
    release_mutex_lock(&game_mutex);

    parse_begn(client_socket, "O", "Moe Gates", &buffer);

    // Opponent: MOVE|6|X|2,2|
    parse_movd(client_socket, "X", "2", "2", "....X....", &buffer);

    // Drop connection
    close(client_socket);

    // print success
    printf("C CLIENT 2: PASSED\n");

    obtain_mutex_lock(&main_mutex);
    should_exit++;
    release_mutex_lock(&main_mutex);

    pthread_exit(NULL);
}


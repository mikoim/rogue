#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "auth.h"
#include "socket.h"
#include "packet.h"
#include "config.h"
#include "game.h"

sig_atomic_t running;

Game g_games[MAX_DEPTH];

void sigintHandler(int signum) {
    running = 0;
}

void *main_loop(void *args) {
    int sock, running_loop = 1;
    char buf[MAX_BUF];
    Character *character = NULL;
    Game *game = NULL;

    Message *message = (Message *) &buf;
    Message_Login *message_login = (Message_Login *) &buf;
    Message_Profile *message_profile = (Message_Profile *) &buf;
    Message_Move *message_move = (Message_Move *) &buf;
    Message_Attack *message_attack = (Message_Attack *) &buf;

    pthread_detach(pthread_self());

    sock = *(int *) args;
    free(args);

    while (running && running_loop) {
        if (packet_receive(sock, buf) < 0) break;

        switch (message->type) {
            case MES_QUIT:
                running_loop = 0;
                break;

            case MES_LOGIN:
                if (auth_login(message_login->username, message_login->password, &character) == 0) {
                    game = &g_games[character->depth];

                    memset(message_profile, 0, sizeof(Message_Profile));
                    message_profile->type = MES_PROFILE;
                    message_profile->status = STATUS_OK;
                    game_correct_player_position_us(game, character);
                    memcpy(&message_profile->profile, character, sizeof(Character));
                    packet_send(sock, buf, sizeof(Message_Profile));

                    game_join(game, sock, character);

                } else {
                    memset(buf, 0, sizeof(Message_Profile));
                    message_profile->type = MES_PROFILE;
                    message_profile->status = STATUS_NG;
                    packet_send(sock, buf, sizeof(Message_Profile));

                    memset(buf, 0, sizeof(Message));
                    message->type = MES_QUIT;
                    packet_send(sock, buf, sizeof(Message));
                }
                break;

            case MES_MOVE:
                message_move->uuid = character->uuid;
                game_add_event(game, character, message_move, sizeof(Message_Move));
                break;

            case MES_ATTACK:
                message_attack->by_uuid = character->uuid;
                game_add_event(game, character, message_attack, sizeof(Message_Attack));
                break;

            case MES_MAP:
            case MES_ALIVE:
            case MES_DEAD:
            case MES_PROFILE:
            default:
                printf("main_loop(): Unknown packet recieved. [%d]\n", message->type);
                break;
        }
    }

    game_leave(game, character);

    socket_close(sock);

    return NULL;
}

int main(int argc, char *argv[]) {
    int sock, *client_sock, i;
    pthread_t thread;

    printf("Rogue Server\n");

    if (argc != 2) {
        printf("Usage : %s port\n", argv[0]);
        return 0;
    }

    if ((sock = socket_listen(argv[1])) < 0)
        return -1;

    packet_no_printf(1);

    running = 1;

    auth_init();
    auth_dump();

    for (i = 0; i < MAX_DEPTH; ++i)
        game_init(&g_games[i], (uint64_t) i);

    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("signal()");
        return -1;
    }

    printf("Server is running.\n");

    for (i = 0; i < MAX_DEPTH; i++)
        pthread_create(&thread, NULL, game_thread, &g_games[i]);

    while (running) {
        client_sock = calloc(1, sizeof(int));

        if ((*client_sock = socket_accept(sock)) < 0) {
            free(client_sock);
            continue;
        }

        printf("[%d] accept\n", *client_sock);

        pthread_create(&thread, NULL, main_loop, client_sock);
    }

    printf("Server is shutting down.\n");

    socket_close(sock);

    auth_free();

    for (i = 0; i < MAX_DEPTH; ++i)
        game_free(&g_games[i]);

    return 0;
}
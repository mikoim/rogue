#ifndef HEADER_GAME_H
#define HEADER_GAME_H

#include <stdint.h>
#include <pthread.h>
#include "auth.h"
#include "map.h"
#include "linkedList.h"

typedef struct {
    int sock;
    Character *character;
} Player;

typedef struct game_t {
    uint64_t depth;
    pthread_mutex_t lock;
    LinkedList *list_players;
    LinkedList *list_mobs;
    LinkedList *queue_event;
    Player dummy;
    Map map;
} Game;

void game_init(Game *game, uint64_t depth);

void game_free(Game *game);

void *game_thread(void *args);

void game_join(Game *game, int sock, Character *character);

void game_leave(Game *game, Character *character);

void game_add_event(Game *game, Character *character, void *message, size_t size);

void game_add_event_us(Game *game, Character *character, void *message, size_t size);

void game_correct_player_position_us(Game *game, Character *character);

void game_routine_broadcast_message_us(Game *game, void *message, size_t size);

void game_new_character(Character *c, uint8_t lv);

int game_routine_exp(Character *c);

#endif
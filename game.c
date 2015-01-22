#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include "map.h"
#include "message.h"
#include "linkedList.h"
#include "packet.h"
#include "game.h"
#include "config.h"
#include "mob.h"

extern sig_atomic_t running;

void game_routine_broadcast_message_us(Game *game, void *message, size_t size);

void game_routine_move_us(Game *game, Message_Move *m);

void game_routine_attack_us(Game *game, Message_Attack *message_attack);

int game_routine_exp(Character *c);

void game_init(Game *game, uint64_t depth) {
    printf("game_init(): depth %lu...", depth);

    game->depth = depth;
    pthread_mutex_init(&game->lock, NULL);
    game->list_players = linkedList_init();
    game->list_mobs = linkedList_init();
    game->queue_event = linkedList_init();

    map_init(&game->map, 400, 300);
    map_generate(&game->map, (unsigned int) rand());

    mob_routine_spawn(game);

    printf("OK!\n");
}

void game_free(Game *game) {
    printf("game_free(): depth %lu\n", game->depth);

    linkedList_free(game->list_players);
    linkedList_free(game->queue_event);

    map_free(&game->map);
}

void *game_thread(void *args) {
    Game *game = (Game *) args;
    char buf[MAX_BUF];
    Message *message = (Message *) &buf;
    Message_Dead *message_dead = (Message_Dead *) &buf;
    Message_Alive *message_alive = (Message_Alive *) &buf;
    Message_Move *message_move = (Message_Move *) &buf;
    Message_Attack *message_attack = (Message_Attack *) &buf;

    printf("game_thread(): depth %lu (seed %d) is running.\n", game->depth, game->map.seed);

    while (running) {
        pthread_mutex_lock(&game->lock);

        while (linkedList_pop(game->queue_event, buf, 0) > 0) {
            switch (message->type) {
                case MES_ALIVE:
                    game_routine_broadcast_message_us(game, message_alive, sizeof(Message_Alive));
                    break;

                case MES_DEAD:
                    game_routine_broadcast_message_us(game, message_dead, sizeof(Message_Dead));
                    break;

                case MES_MOVE:
                    game_routine_move_us(game, message_move);
                    break;

                case MES_ATTACK:
                    game_routine_attack_us(game, message_attack);
                    break;

                default:
                    break;
            }
        }

        mob_routine_move(game);

        pthread_mutex_unlock(&game->lock);

        struct timespec sl = {0, 100000};

        nanosleep(&sl, NULL);
    }

    return NULL;
}

Player *game_find_player_by_uuid(Game *game, int16_t uuid, int remove) {
    int i;
    LinkedList *list;

    if (uuid < 0) {
        for (list = game->list_mobs, i = 0; list != NULL; list = list->next, i++) {
            if (((Character *) list->data)->uuid == uuid) {
                if (remove == 1) {
                    linkedList_removeIndexOf(game->list_mobs, i);
                    return NULL;
                } else {
                    game->dummy.sock = -1;
                    game->dummy.character = list->data;
                    return &game->dummy;
                }

            }
        }
    } else {
        for (list = game->list_players, i = 0; list != NULL; list = list->next, i++) {
            if (((Player *) list->data)->character->uuid == uuid) {
                if (remove == 1) {
                    linkedList_removeIndexOf(game->list_players, i);
                    return NULL;
                } else {
                    return list->data;
                }
            }
        }
    }

    return NULL;
}

void game_routine_broadcast_message_us(Game *game, void *message, size_t size) {
    LinkedList *t;

    for (t = game->list_players; t != NULL && t->data != NULL; t = t->next)
        packet_send(((Player *) t->data)->sock, message, size);
}

void game_routine_attack_us(Game *game, Message_Attack *message_attack) {
    Player *pt, *pb;
    Character *ct, *cb;
    double damage, experience;

    pt = game_find_player_by_uuid(game, message_attack->uuid, 0);
    pb = game_find_player_by_uuid(game, message_attack->by_uuid, 0);

    if (pt != NULL && pb != NULL) {
        ct = pt->character;
        cb = pb->character;
    } else {
        return;
    }

    damage = ct->atk.cur + (3 * ct->atk.cur * pow(ct->lv, 2)) / 512;
    experience = (ct->atk.cur + ct->def.cur) - (cb->atk.cur + cb->def.cur);

    ct->hp.cur -= damage;

    if (experience <= 0)
        experience = 1;

    if (ct->hp.cur <= 0) {
        cb->exp.cur += experience;

        Message_Dead dead;
        dead.type = MES_DEAD;
        dead.uuid = ct->uuid;
        dead.by_uuid = cb->uuid;

        linkedList_push(game->queue_event, &dead, sizeof(dead));

        game_find_player_by_uuid(game, ct->uuid, 1);

        if (game_routine_exp(cb) && cb->type == CHARACTER_TYPE_PLAYER) {
            Message_Profile profile;
            profile.type = MES_PROFILE;
            profile.status = STATUS_OK;
            memcpy(&profile.profile, cb, sizeof(Message_Profile));

            packet_send(pb->sock, &profile, sizeof(Message_Profile));
        }
    }

    if (ct->type == CHARACTER_TYPE_PLAYER) {
        Message_Profile profile;
        profile.type = MES_PROFILE;
        profile.status = STATUS_OK;
        memcpy(&profile.profile, ct, sizeof(Character));

        packet_send(pt->sock, &profile, sizeof(Message_Profile));
    }
}

void game_routine_move_us(Game *game, Message_Move *m) {
    Player *p;

    if (m->x < 0 || m->y < 0 || m->x >= game->map.x || m->y >= game->map.y || game->map.field[m->y][m->x] != FIELD_TYPE_ROAD)
        return;

    p = game_find_player_by_uuid(game, m->uuid, 0);

    if (p != NULL) {
        p->character->x = m->x;
        p->character->y = m->y;

        game_routine_broadcast_message_us(game, m, sizeof(Message_Move));
    }
}

void game_correct_player_position_us(Game *game, Character *character) {
    uint16_t x, y;

    do {
        x = (uint16_t) (rand() % game->map.x);
        y = (uint16_t) (rand() % game->map.y);
    } while (game->map.field[y][x] != FIELD_TYPE_ROAD);

    character->x = x;
    character->y = y;
}

void game_join_us(Game *game, int sock, Character *character) {
    LinkedList *t;
    Player player, *pt;
    Message_Alive alive;
    Message_Map map;

    memset(&player, 0, sizeof(Player));
    player.sock = sock;
    player.character = character;

    memset(&alive, 0, sizeof(Message_Alive));
    alive.type = MES_ALIVE;
    memcpy(&alive.character, character, sizeof(Character));

    linkedList_push(game->list_players, &player, sizeof(player));
    linkedList_push(game->queue_event, &alive, sizeof(alive));

    /* Init client */
    memset(&map, 0, sizeof(Message_Map));
    map.type = MES_MAP;
    map.seed = game->map.seed;
    map.x = (uint16_t) game->map.x;
    map.y = (uint16_t) game->map.y;

    packet_send(sock, &map, sizeof(map));

    /* Send all player information in same depth */
    for (t = game->list_players; t != NULL && (pt = t->data) != NULL; t = t->next) {
        if (pt->character->uuid == character->uuid) continue;

        memset(&alive, 0, sizeof(Message_Alive));
        alive.type = MES_ALIVE;
        memcpy(&alive.character, pt->character, sizeof(Character));

        packet_send(sock, &alive, sizeof(Message_Alive));
    }

    /* Send all player information in same depth for MOBS */
    Character *c;

    for (t = game->list_mobs; t != NULL && (c = t->data) != NULL; t = t->next) {
        memset(&alive, 0, sizeof(Message_Alive));
        alive.type = MES_ALIVE;
        memcpy(&alive.character, c, sizeof(Character));

        packet_send(sock, &alive, sizeof(Message_Alive));
    }
}

void game_leave_us(Game *game, Character *character) {
    LinkedList *t;
    int i;
    Message_Dead dead;

    for (i = 0, t = game->list_players; t != NULL && t->data != NULL; t = t->next, i++)
        if (((Player *) t->data)->character->uuid == character->uuid) {
            linkedList_removeIndexOf(game->list_players, i);
            break;
        }

    memset(&dead, 0, sizeof(Message_Dead));
    dead.type = MES_DEAD;
    dead.uuid = character->uuid;

    linkedList_push(game->queue_event, &dead, sizeof(dead));
}

void game_join(Game *game, int sock, Character *character) {
    pthread_mutex_lock(&game->lock);

    game_join_us(game, sock, character);

    printf("game_join(): %d join depth %lu. [Total %i]\n", character->uuid, game->depth, linkedList_getLength(game->list_players));

    pthread_mutex_unlock(&game->lock);
}

void game_leave(Game *game, Character *character) {
    if (game == NULL || character == NULL)
        return;

    pthread_mutex_lock(&game->lock);

    game_leave_us(game, character);

    printf("game_leave(): %d leave depth %lu. [Total %i]\n", character->uuid, game->depth, linkedList_getLength(game->list_players));

    pthread_mutex_unlock(&game->lock);
}

void game_add_event(Game *game, Character *character, void *message, size_t size) {
    pthread_mutex_lock(&game->lock);

    linkedList_push(game->queue_event, message, size);

    pthread_mutex_unlock(&game->lock);
}

int game_routine_exp(Character *c) { // Level up : 0, Normal : -1
    int flag = -1;
    double a, d, e, h;

    if (c->exp.cur >= c->exp.max) {
        c->lv++;
        c->exp.cur = 0;
        flag = 0;
    }

    a = (double) c->atk.base * pow(1.2, c->lv);
    d = (double) c->def.base * pow(1.2, c->lv);
    e = pow(1.2, c->lv) * 10;
    h = (double) c->hp.base * pow(1.2, c->lv);

    c->atk.cur = (int64_t) ceil(a);
    c->def.cur = (int64_t) ceil(d);
    c->exp.max = (int64_t) ceil(e);
    c->hp.max = (int64_t) ceil(h);

    if (flag == 0)
        c->hp.cur = c->hp.max;

    return flag;
}

void game_new_character(Character *c, uint8_t lv) {
    c->lv = 0;
    c->depth = 0;
    c->x = 0;
    c->y = 0;
    c->exp.cur = 1;
    c->exp.max = 1;
    c->delay = 0;

    c->atk.base = (int64_t) rand() % 10 + 5;
    c->def.base = (int64_t) rand() % 10 + 5;
    c->hp.base = (int64_t) rand() % 10 + 10;

    game_routine_exp(c);
}

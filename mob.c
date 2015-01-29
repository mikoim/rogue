#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "game.h"
#include "map.h"
#include "message.h"
#include "vector.h"

typedef struct {
    Game *game;
    int16_t **map;
} UMap;

UMap mob_umap_init(Game *game) {
    UMap uMap;
    LinkedList *t;
    Player *p;
    int y, x;

    uMap.game = game;

    uMap.map = calloc(game->map.y, sizeof(int16_t*));
    for (y = 0; y < game->map.y; y++)
        uMap.map[y] = calloc(game->map.x, sizeof(int16_t));

    for (y = 0; y < game->map.y; y++)
        for (x = 0; x < game->map.x; x++)
            uMap.map[y][x] = -1;

    for (t = game->list_players; t != NULL && (p = t->data) != NULL; t = t->next) {
        x = p->character->x;
        y = p->character->y;

        uMap.map[y][x] = p->character->uuid;
    }

    return uMap;
}

int16_t mob_umap_get_near_player(UMap *uMap, int x, int y) {
    int i;
    Vector d[] = {
            {0, -1, 0},
            {1, -1, 0},
            {1, 0, 0},
            {1, 1, 0},
            {0, 1, 0},
            {-1, 1, 0},
            {-1, 0, 0},
            {-1, -1, 0}
    }, c;

    for (i = 0; i < 8; i++) {
        c = vector_assignment(x, y, 0);
        vector_addition(&c, &d[i]);

        if (c.x < 0 || c.y < 0 || c.x >= uMap->game->map.x || c.y >= uMap->game->map.y)
            continue;

        if (uMap->map[c.y][c.x] != -1)
            return uMap->map[c.y][c.x];
    }

    return -1;
}

void mob_umap_free(UMap *uMap) {
    int y;

    if (uMap == NULL)
        return;

    for (y = 0; y < uMap->game->map.y; y++)
        free(uMap->map[y]);

    free(uMap->map);
}

void mob_routine_spawn(Game *game) {
    Character mob;

    for (int i = 0; i < 50; ++i) {
        memset(&mob, 0, sizeof(Character));
        mob.uuid = (uint16_t) (UINT16_MAX - i);
        mob.type = CHARACTER_TYPE_MOB;
        game_new_character(&mob, (uint8_t) (game->depth + 1));
        game_correct_player_position_us(game, &mob);

        linkedList_push(game->list_mobs, &mob, sizeof(Character));
    }
}

void mob_routine_move(Game *game) {
    LinkedList *t;
    Character *c;
    int d;
    uint16_t x, y;
    Message_Move message_move;

    for (t = game->list_mobs; t != NULL && (c = t->data) != NULL; t = t->next) {

        if (c->delay-- > 0)
            continue;

        while (1) {
            x = c->x, y = c->y;
            d = rand() % 4;

            switch (d) {
                case 0:
                    y--;
                    break;

                case 1:
                    y++;
                    break;

                case 2:
                    x--;
                    break;

                case 3:
                    x++;
                    break;

                default:
                    break;
            }

            if(x >= game->map.x || y >= game->map.y || game->map.field[y][x] != FIELD_TYPE_ROAD) {
                return;
            }

            break;
        }

        c->x = x;
        c->y = y;

        c->delay = rand() % 1000;

        memset(&message_move, 0, sizeof(message_move));
        message_move.type = MES_MOVE;
        message_move.uuid = c->uuid;
        message_move.x = x;
        message_move.y = y;

        game_routine_broadcast_message_us(game, &message_move, sizeof(Message_Move));
    }
}

void mob_routine_attack(Game *game) {
    LinkedList *t;
    Character *c;
    uint16_t x, y;
    int16_t target;
    Message_Attack attack;
    UMap uMap = mob_umap_init(game);

    for (t = game->list_mobs; t != NULL && (c = t->data) != NULL; t = t->next) {
        if ((target = mob_umap_get_near_player(&uMap, c->x, c->y)) == -1)
            continue;

        memset(&attack, 0, sizeof(Message_Attack));
        attack.type = MES_ATTACK;
        attack.uuid = target;
        attack.by_uuid = c->uuid;

        game_add_event_us(game, c, &attack, sizeof(Message_Attack));
    }

    mob_umap_free(&uMap);
}
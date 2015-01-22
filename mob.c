#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "game.h"

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
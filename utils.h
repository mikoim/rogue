#ifndef HEADER_UTILS_H
#define HEADER_UTILS_H

#include <stdint.h>
#include <ncurses.h>
#include "game.h"

char utils_genRandomChar();

chtype utils_gen_enemy_char(Character *me, Character *enemy);

char *utils_genRandomString(char *buf, int max);

char *utils_genProgressbar(char *buf, int64_t cur, int64_t max, int size);

char utils_sanitize(char input);

void utils_dump(void *data, size_t size);

#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <curses.h>
#include "game.h"

char utils_genRandomChar() {
    return (char) (rand() % ('~' - ' ') + ' ');
}

chtype utils_gen_enemy_char(Character *me, Character *enemy) {
    int64_t rate = (me->atk.cur + me->def.cur) * 100 / (enemy->atk.cur + enemy->def.cur);

    if (rate > 100) rate = 100;

    return (chtype) (COLOR_PAIR((int) (3 - rate * 2 / 100)) | (char) ('A' + (char) (rate * 25 / 100)));
}

char *utils_genRandomString(char *buf, int max) {
    int i, cur;

    if (buf == NULL || max == 0) return NULL;

    cur = rand() % max + 1;

    for (i = 0; i < cur; ++i) {
        buf[i] = utils_genRandomChar();
    }

    buf[i] = '\0';

    return buf;
}

char *utils_genProgressbar(char *buf, int64_t cur, int64_t max, int size) {
    int i;
    double p, l, tmp;

    if (buf == NULL || max == 0 || size == 0) return NULL;

    p = cur / (double) max;
    l = 1 / (double) size;

    for (i = 0; i < size; i++) {
        tmp = (i + 1) * l;

        if (p >= tmp)
            buf[i] = '|';
        else if (p >= tmp - (l * 0.5))
            buf[i] = '>';
        else
            buf[i] = '.';
    }

    buf[i] = '\0';

    return buf;
}

char utils_sanitize(char input) {
    if (' ' > input || '~' < input) return '.';

    return input;
}

void utils_dump(void *data, size_t size) {
    size_t index, row, column;
    unsigned char *tmp;

    printf("utils_dump(): %lubytes\n", size);

    row = size / 16;
    tmp = data;
    index = 0;

    if (size % 16 != 0) row++;

    while (row--) {
        for (column = 0; column < 16; column++) {
            if (index < size)
                printf("%02X ", tmp[index + column]);
            else
                printf("-- ");
        }

        printf("| ");

        for (column = 0; column < 16; column++) {
            if (index < size)
                printf("%c", utils_sanitize(tmp[index + column]));
            else
                printf(" ");
        }

        index += 16;

        printf("\n");
    }
}
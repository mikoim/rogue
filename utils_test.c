#include <stdint.h>
#include "utils.h"

int main() {
    for (uint64_t i = 0; i < 60; ++i) {
        Character me, enemy;

        me.atk.cur = 1;
        me.def.cur = 1;

        enemy.atk.cur = i;
        enemy.def.cur = 0;

        printf("%lu\n", utils_gen_enemy_char(&me, &enemy));
    }

    return 0;
}
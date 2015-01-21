#include <stdio.h>
#include <signal.h>
#include "game.h"

sig_atomic_t running;

int main() {
    Character character;

    game_new_character(&character, 1);
    character.exp.cur = character.exp.max;

    printf("%d\n", game_routine_exp(&character));

    return 0;
}
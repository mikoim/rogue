#include <stdio.h>
#include "map.h"

int main() {
    Map map;

    map_init(&map, 100, 100);

    map_generate(&map, 1);

    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            switch (map.field[y][x]) {
                case FIELD_TYPE_NONE:
                default:
                    printf(".");
                    break;

                case FIELD_TYPE_ROAD:
                    printf(" ");
                    break;

                case FIELD_TYPE_LADDER_UP:
                    printf(">");
                    break;

                case FIELD_TYPE_LADDER_DOWN:
                    printf("<");
                    break;
            }
        }

        printf("\n");
    }

    return 0;
}
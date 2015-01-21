#ifndef HEADER_MAP_H
#define HEADER_MAP_H

#include <stdint.h>

enum {
    FIELD_TYPE_NONE, FIELD_TYPE_ROAD, FIELD_TYPE_LADDER_UP, FIELD_TYPE_LADDER_DOWN
};

typedef struct {
    char **field;
    char **sight;
    unsigned int seed;
    int x, y;
} Map;

void map_init(Map *map, int x, int y);

void map_generate(Map *map, unsigned int seed);

void map_sight_update(Map *map, int sight, int x, int y);

void map_free(Map *map);

#endif
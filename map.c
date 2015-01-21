#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "map.h"
#include "vector.h"

enum GRID_TYPE {
    // U, D : ラダーがある部屋
            EMPTY, PATH, ROOM, UROOM, DROOM
};

typedef struct {
    enum GRID_TYPE **type;
    int **pathfinder;
    uint64_t x, y;
} Grid;

void map_init(Map *map, int x, int y) {
    int i;

    map->x = x;
    map->y = y;

    map->field = calloc((size_t) map->y, sizeof(char *));
    map->sight = calloc((size_t) map->y, sizeof(char *));

    for (i = 0; i < map->y; i++) {
        map->field[i] = calloc((size_t) map->x, sizeof(char));
        map->sight[i] = calloc((size_t) map->x, sizeof(char));
    }
}

void map_free(Map *map) {
    int i;

    for (i = 0; i < map->y; i++) {
        free(map->field[i]);
        free(map->sight[i]);
    }

    free(map->field);
    free(map->sight);
}

void grid_init(Grid *grid) {
    int i, j;

    grid->type = calloc((size_t) grid->y, sizeof(enum GRID_TYPE *));
    grid->pathfinder = calloc((size_t) (grid->y + 1), sizeof(int *));

    for (i = 0; i < grid->y; i++)
        grid->type[i] = calloc((size_t) grid->x, sizeof(enum GRID_TYPE));
    for (i = 0; i < grid->y + 1; i++)
        grid->pathfinder[i] = calloc((size_t) (grid->x + 1), sizeof(int));

    for (i = 0; i < grid->y; i++)
        for (j = 0; j < grid->x; j++)
            grid->type[i][j] = EMPTY;

    for (i = 0; i < grid->y + 1; i++)
        for (j = 0; j < grid->x + 1; j++)
            grid->pathfinder[i][j] = -10;
}

void grid_free(Grid *grid) {
    int i;

    for (i = 0; i < grid->y; i++) {
        free(grid->type[i]);
        free(grid->pathfinder[i]);
    }

    free(grid->pathfinder[i]);

    free(grid->type);
    free(grid->pathfinder);
}

void draw_path(Map *map, int sx, int sy, int gx, int gy) {
    int diffX, diffY, absdiffX, absdiffY, i, j, x, y;
    double addX, addY, length;

    // 差
    diffX = gx - sx;
    diffY = gy - sy;
    absdiffX = diffX;
    absdiffY = diffY;
    if (absdiffX < 0) absdiffX = absdiffX * -1;
    if (absdiffY < 0) absdiffY = absdiffY * -1;

    // 長さ
    length = sqrt(pow(diffX, 2) + pow(diffY, 2));

    // 傾き
    addX = diffX / length;
    addY = diffY / length;

    for (i = 0; i < length; ++i) {
        for (j = 0; j < 1 + rand() % 4; j++) {
            if (rand() % 2 == 0) {
                x = (int) (sx + (addX * i) + (absdiffY >= absdiffX ? j : 0));
                y = (int) (sy + (addY * i) + (absdiffX >= absdiffY ? j : 0));
            }
            else {
                x = (int) (sx + (addX * i) + (absdiffY >= absdiffX ? -j : 0));
                y = (int) (sy + (addY * i) + (absdiffX >= absdiffY ? -j : 0));
            }

            if (x < 0 || y < 0 || x >= map->x - 1 || y >= map->y - 1) continue;

            map->field[y][x] = FIELD_TYPE_ROAD;
        }
    }
}

void room_generate(Map *map, Grid *grid) {
    const uint64_t ROOMNUM = 80;    // 部屋の数
    const uint64_t MAX = 65;    // 最大比率
    const uint64_t MIN = 30;    // 最小比率
    const uint64_t MISSALIGN = 1;    // ズレの許容範囲
    uint64_t sizey = map->y / grid->y;    // 1グリッドあたりのYサイズ
    uint64_t sizex = map->x / grid->x;    // 1グリッドあたりのXサイズ
    uint64_t gx, gy, i, j, k, aligny, alignx;

    for (i = 0; i < ROOMNUM; i++) {
        do {
            gx = rand() % grid->x;
            gy = rand() % grid->y;
        } while (grid->type[gy][gx] != PATH);

        grid->type[gy][gx] = ROOM;

        uint64_t ratiox = rand() % (MAX - MIN) + MIN;
        uint64_t ratioy = rand() % (MAX - MIN) + MIN;

        aligny = (-1) * (MISSALIGN / 2) + (rand() % (MISSALIGN + 1));
        alignx = (-1) * (MISSALIGN / 2) + (rand() % (MISSALIGN + 1));

        for (j = 0; j < sizey; j++)
            for (k = 0; k < sizex; k++) {
                if (j >= sizey * (100 - ratiox) / 200 &&
                        j <= sizey - sizey * (100 - ratiox) / 200 &&
                        k >= sizex * (100 - ratioy) / 200 &&
                        k <= sizex - sizex * (100 - ratioy) / 200)
                    map->field[gy * sizey + j + aligny][gx * sizex + k + alignx] = FIELD_TYPE_ROAD;
            }
    }

}

void radder_generate(Map *map, Grid *grid) {
    const uint64_t MAX = 45;    // 最大比率
    const uint64_t MIN = 40;    // 最小比率
    uint64_t sizey = map->y / grid->y;    // 1グリッドあたりのYサイズ
    uint64_t sizex = map->x / grid->x;    // 1グリッドあたりのXサイズ
    uint64_t gy, gx, y, x;

    uint64_t ratiox = rand() % (MAX - MIN) + MIN;
    uint64_t ratioy = rand() % (MAX - MIN) + MIN;

    // upper radder generate
    while (1) {
        do {
            gx = rand() % grid->x;
            gy = rand() % grid->y;
        } while (grid->type[gy][gx] != ROOM);
        grid->type[gy][gx] = UROOM;

        y = gy * sizey + (sizey * (100 - ratioy) / 200) +
                rand() % (sizey - sizey * (100 - ratioy) / 200) - (sizey * (100 - ratioy) / 200);
        x = gx * sizex + (sizex * (100 - ratiox) / 200) +
                rand() % (sizex - sizex * (100 - ratiox) / 200) - (sizey * (100 - ratiox) / 200);

        if (map->field[y][x] == FIELD_TYPE_ROAD) break;
    }

    map->field[y][x] = FIELD_TYPE_LADDER_UP;


    // down radder generate
    while (1) {
        do {
            gx = rand() % grid->x;
            gy = rand() % grid->y;
        } while (grid->type[gy][gx] != ROOM);
        grid->type[gy][gx] = DROOM;

        y = gy * sizey + (sizey * (100 - ratioy) / 200) +
                rand() % (sizey - sizey * (100 - ratioy) / 200) - (sizey * (100 - ratioy) / 200);
        x = gx * sizex + (sizex * (100 - ratiox) / 200) +
                rand() % (sizex - sizex * (100 - ratiox) / 200) - (sizey * (100 - ratiox) / 200);

        if (map->field[y][x] == FIELD_TYPE_ROAD) break;
    }

    map->field[y][x] = FIELD_TYPE_LADDER_DOWN;
}

void path_generate(Map *map, Grid *grid) {
    uint64_t sgridx, sgridy;        // スタートグリッド
    uint64_t i, j, k, l, r, c, limit;
    uint64_t pastdir = 0;            // 前の方向
    const int NUM = 30;        // パスの数
    const int CURVE = 18;    // 1パスあたりのCURVE数
    uint64_t sizey = map->y / grid->y;    // 1グリッドあたりのYサイズ
    uint64_t sizex = map->x / grid->x;    // 1グリッドあたりのXサイズ

    for (i = 0; i < NUM; i++) {
        // スタートグリッドの指定
        if (i == 0) {
            sgridx = rand() % grid->x;
            sgridy = rand() % grid->y;
        }
        else {
            // 既存の路を避けての任意の点を選ぶ
            /* どうやらこの方がダンジョンっぽくなるぞ */
            c = 0;
            do {
                sgridx = rand() % grid->x;
                sgridy = rand() % grid->y;
                c++;
                if (c > 15)
                    break;
            } while (grid->type[sgridy][sgridx] == PATH);
        }

        // 1パスの生成
        for (j = 0; j < CURVE; j++) {
            // 方向の設定
            limit = 0;
            do {
                k = (uint64_t) ((1 + rand()) % 4);
                switch (k) {
                    case 0:
                        if (sgridy == 0 || pastdir == 8)
                            k = 0;
                        else
                            k = 8;
                        break;                // 上
                    case 1:
                        if (sgridx == grid->x - 1 || pastdir == 6)
                            k = 0;
                        else
                            k = 6;
                        break;    // 右
                    case 2:
                        if (sgridy == grid->y - 1 || pastdir == 5)
                            k = 0;
                        else
                            k = 5;
                        break;    // 下
                    case 3:
                        if (sgridx == 0 || pastdir == 4)
                            k = 0;
                        else
                            k = 4;
                        break;                // 左
                    default:
                        break;
                }
                limit++;
            } while (k < 4 && limit < 8);
            pastdir = k;
            // GRID_TYPEにPATHを設定 & ROADの生成
            grid->type[sgridy][sgridx] = PATH;
            r = (uint64_t) (1 + rand() % 2);
            for (l = 0; l < r; l++) {
                switch (k) {
                    case 8:
                        if (sgridy != 0) {
                            grid->type[sgridy - 1][sgridx] = PATH;
                            draw_path(map, sgridx * sizex + sizex / 2, sgridy * sizey + sizey / 2,
                                    sgridx * sizex + sizex / 2, (sgridy - 1) * sizey + sizey / 2);
                            sgridy--;
                        }
                        break;
                    case 6:
                        if (sgridx != grid->x - 1) {
                            grid->type[sgridy][sgridx + 1] = PATH;
                            draw_path(map, sgridx * sizex + sizex / 2, sgridy * sizey + sizey / 2,
                                    (sgridx + 1) * sizex + sizex / 2, sgridy * sizey + sizey / 2);
                            sgridx++;
                        }
                        break;
                    case 5:
                        if (sgridy != grid->y - 1) {
                            grid->type[sgridy + 1][sgridx] = PATH;
                            draw_path(map, sgridx * sizex + sizex / 2, sgridy * sizey + sizey / 2,
                                    sgridx * sizex + sizex / 2, (sgridy + 1) * sizey + sizey / 2);
                            sgridy++;
                        }
                        break;
                    case 4:
                        if (sgridx != 0) {
                            grid->type[sgridy][sgridx - 1] = PATH;
                            draw_path(map, sgridx * sizex + sizex / 2, sgridy * sizey + sizey / 2,
                                    (sgridx - 1) * sizex + sizex / 2, sgridy * sizey + sizey / 2);
                            sgridx--;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void map_generate(Map *map, unsigned int seed) {
    Grid g;

    srand(seed);
    map->seed = seed;

    g.x = 12;    // x分割数
    g.y = 18;    // y分割数

    grid_init(&g);

    path_generate(map, &g);

    room_generate(map, &g);

    radder_generate(map, &g);

    grid_free(&g);
}

int map_sight_update_sub(Map *map, Vector c) {
    if (c.x < 0 || c.y < 0 || c.x >= map->x || c.y >= map->y)
        return -1;

    if (map->field[c.y][c.x] != FIELD_TYPE_NONE) {
        map->sight[c.y][c.x] = 1;

    } else {
        return -1;
    }

    return 0;
}

void map_sight_update(Map *map, int sight, int x, int y) {
    int limit, i, j, w;
    Vector d[] = {
            {0, -1, 0},
            {1, -1, 0},
            {1, 0, 0},
            {1, 1, 0},
            {0, 1, 0},
            {-1, 1, 0},
            {-1, 0, 0},
            {-1, -1, 0}
    }, c, rc;

    for (i = 0; i < map->y; i++) {
        for (j = 0; j < map->x; j++) {
            map->sight[i][j] = 0;
        }
    }

    map->sight[y][x] = 1;

    for (i = 0; i < 8; i++) {
        limit = sight;
        c = vector_assignment(x, y, 0);

        while (limit--) {
            c = vector_addition(&c, &d[i]);
            w = 10;

            if (sqrt(pow(x - c.x, 2) + pow(y - c.y, 2)) > 10)
                break;

            if (map_sight_update_sub(map, c) < 0)
                break;

            while (w--) {
                rc = c;

                while (1) {
                    for (j = 0; j < w; j++)
                        rc = vector_addition(&rc, &d[i]);

                    rc = vector_addition(&rc, &d[(i + 1) % 8]);

                    if (sqrt(pow(x - rc.x, 2) + pow(y - rc.y, 2)) > 10)
                        break;

                    if (map_sight_update_sub(map, rc) < 0)
                        break;
                }
            }
        }
    }
}
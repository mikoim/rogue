#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "socket.h"
#include "packet.h"
#include "utils.h"
#include "config.h"

Map g_map;
Character g_character;
LinkedList *g_players;
WINDOW *mapWindow, *logWindow, *statusWindow, *playerWindow;
int show_logWindow, show_statusWindow, show_playerWindow;

int sock;

sig_atomic_t running = 1;

pthread_mutex_t mutex_cui;
pthread_mutex_t mutex_players;

void sigintHandler(int signum) {
    running = 0;
}

char *inputStrings(char *buf, const char *name, int min, int max) {
    size_t length;

    while (1) {
        printf("%s: ", name);
        fgets(buf, MAX_BUF, stdin);

        length = strlen(buf);
        buf[length - 1] = '\0';
        length--;

        if (length > max) {
            printf("%s must be less than %d characters.\n", name, max);
            continue;
        }

        if (length < min) {
            printf("%s must be more than %d characters.\n", name, min);
            continue;
        }

        break;
    }

    return buf;
}

void cui_set_profile(const Character *p) {
    char buf[MAX_BUF], progress[MAX_BUF];

    if (p == NULL) return;

    sprintf(buf, "Name: %10s Lv:%3d", p->name, p->lv);
    mvwaddstr(statusWindow, 2, 3, buf);

    sprintf(buf, "HP  : %s (%3ld/%3ld)", utils_genProgressbar(progress, p->hp.cur, p->hp.max, 10), p->hp.cur, p->hp.max);
    mvwaddstr(statusWindow, 3, 3, buf);

    sprintf(buf, "EXP : %s (%3ld/%3ld)", utils_genProgressbar(progress, p->exp.cur, p->exp.max, 10), p->exp.cur, p->exp.max);
    mvwaddstr(statusWindow, 4, 3, buf);

    sprintf(buf, "ATK : %3ld", p->atk.cur);
    mvwaddstr(statusWindow, 5, 3, buf);

    sprintf(buf, "DEF : %3ld", p->def.cur);
    mvwaddstr(statusWindow, 6, 3, buf);

    sprintf(buf, "DEV : UUID = %2hu, Depth = %2hhu, (%3hu,%3hu)", p->uuid, p->depth, p->x, p->y);
    mvwaddstr(statusWindow, 7, 3, buf);

    sprintf(buf, "DEV : Seed = %u", g_map.seed);
    mvwaddstr(statusWindow, 8, 3, buf);

    sprintf(buf, "DEV : Z = %u", linkedList_getLength(g_players));
    mvwaddstr(statusWindow, 9, 3, buf);
}

void cui_set_player() {
    char buf[MAX_BUF], *names[7] = {NULL};
    int i = 0;

    for (LinkedList *tmp = g_players; tmp != NULL; tmp = tmp->next) {
        i++;

        sprintf(buf, "%d players online.", i);
        mvwaddstr(playerWindow, 2, 3, buf);
    }
}

void cui_draw_map() {
    Character *c;
    LinkedList *t;
    int absx, absy;
    int relux, reluy, relbx, relby;
    int x, y;

    // 絶対座標
    absx = (int) g_character.x;
    absy = (int) g_character.y;

    // 相対座標
    relux = (-1) * COLS / 2;
    reluy = (-1) * (LINES / 4 * 3) / 2;
    relbx = relux + COLS;
    relby = reluy + (LINES / 4 * 3);

    //map_sight_update(&g_map, 50, g_character.x, g_character.y);

    for (y = reluy; y <= relby + LINES / 4; y++)
        for (x = relux; x < relbx; x++) {
            if (absy + y < 0 || absy + y >= g_map.y || absx + x < 0 || absx + x >= g_map.x) {
                mvwaddch(mapWindow, y - reluy, x - relux, ' ');
                continue;
            }

            /*
            if (g_map.sight[absy + y][absx + x] == 0) {
                mvwaddch(mapWindow, y - reluy, x - relux, '?');
                continue;
            }
            */

            switch (g_map.field[absy + y][absx + x]) {
                case FIELD_TYPE_NONE:
                default:
                    mvwaddch(mapWindow, y - reluy, x - relux, '.');
                    break;

                case FIELD_TYPE_ROAD:
                    mvwaddch(mapWindow, y - reluy, x - relux, ' ');
                    break;

                case FIELD_TYPE_LADDER_UP:
                    mvwaddch(mapWindow, y - reluy, x - relux, '>');
                    break;

                case FIELD_TYPE_LADDER_DOWN:
                    mvwaddch(mapWindow, y - reluy, x - relux, '<');
                    break;
            }
        }

    for (t = g_players; t != NULL && (c = t->data) != NULL; t = t->next) {
        /*
        if (g_map.sight[c->y][c->x] == 0)
            continue;
        */

        if (c->uuid == g_character.uuid &&
                c->y >= absy + reluy && c->y <= absy + relby &&
                c->x >= absx + relux && c->x <= absx + relbx) {
            mvwaddch(mapWindow, (c->y - absy) - reluy, (c->x - absx) - relux, COLOR_PAIR(1) | '@');
        } else {
            mvwaddch(mapWindow, (c->y - absy) - reluy, (c->x - absx) - relux, utils_gen_enemy_char(&g_character, c));
        }
    }
}

void cui_refresh() {
    pthread_mutex_lock(&mutex_cui);

    cui_draw_map();

    wnoutrefresh(mapWindow);

    if (show_logWindow == 1) {
        werase(logWindow);
        box(logWindow, 0, 0);
        wnoutrefresh(logWindow);
    }

    if (show_statusWindow == 1) {
        werase(statusWindow);
        box(statusWindow, 0, 0);
        cui_set_profile(&g_character);
        wnoutrefresh(statusWindow);
    }

    if (show_playerWindow == 1) {
        werase(playerWindow);
        box(playerWindow, 0, 0);
        cui_set_player();
        wnoutrefresh(playerWindow);
    }

    doupdate();

    pthread_mutex_unlock(&mutex_cui);
}

void do_login() {
    const char logo[8][52] = {
            "   (`-')                                  (`-')  _ ",
            "<-.(OO )      .->       .->        .->    ( OO).-/ ",
            ",------,)(`-')----.  ,---(`-'),--.(,--.  (,------. ",
            "|   /`. '( OO).-.  ''  .-(OO )|  | |(`-') |  .---' ",
            "|  |_.' |( _) | |  ||  | .-, \\|  | |(OO )(|  '--.  ",
            "|  .   .' \\|  |)|  ||  | '.(_/|  | | |  \\ |  .--'  ",
            "|  |\\  \\   '  '-'  '|  '-'  | \\  '-'(_ .' |  `---. ",
            "`--' '--'   `-----'  `-----'   `-----'    `------' "
    };

    int i;
    char buf[MAX_BUF];
    Message_Login message_login;

    memset(&message_login, 0, sizeof(message_login));
    message_login.type = MES_LOGIN;

    for (i = 0; i < 8; ++i) {
        printf("%s\n", logo[i]);
    }

    printf("\n[Login]\n");
    memcpy(message_login.username, inputStrings(buf, "Username", 1, 10), 11);
    memcpy(message_login.password, inputStrings(buf, "Password", 1, 32), 33);

    if (packet_send(sock, &message_login, sizeof(message_login)) < 0) running = 0;
}

void do_move(int key) {
    Message_Move message_move;
    Message_Attack message_attack;

    uint16_t x = g_character.x, y = g_character.y;

    memset(&message_move, 0, sizeof(message_move));
    memset(&message_attack, 0, sizeof(message_attack));

    message_move.type = MES_MOVE;
    message_attack.type = MES_ATTACK;

    switch (key) {
        case KEY_UP:
            y--;
            break;

        case KEY_DOWN:
            y++;
            break;

        case KEY_LEFT:
            x--;
            break;

        case KEY_RIGHT:
            x++;
            break;

        default:
            break;
    }

    if (x >= g_map.x || y >= g_map.y) {
        return;
    }

    Character *tmp;

    pthread_mutex_lock(&mutex_players);

    for (int i = 0; (tmp = linkedList_getIndexOf(g_players, i)) != NULL; i++) {
        if (tmp->x == x && tmp->y == y) {
            message_attack.uuid = tmp->uuid;
            message_attack.by_uuid = g_character.uuid;
            packet_send(sock, &message_attack, sizeof(Message_Attack));

            pthread_mutex_unlock(&mutex_players);

            return;
        }
    }

    pthread_mutex_unlock(&mutex_players);

    message_move.uuid = g_character.uuid;
    message_move.x = x;
    message_move.y = y;

    packet_send(sock, &message_move, sizeof(Message_Move));
}

void handler_profile(Message_Profile *message_profile) {
    if (message_profile->status == STATUS_OK) {
        memcpy(&g_character, &message_profile->profile, sizeof(Character));
    } else {
        printf("Username or password is incorrect.\n");
        running = 0;
    }
}

void handler_map(Message_Map *message_map) {
    linkedList_free(g_players);
    g_players = linkedList_init();

    map_init(&g_map, message_map->x, message_map->y);
    map_generate(&g_map, message_map->seed);

    cui_refresh();
}

void handler_alive(Message_Alive *message_alive) {
    pthread_mutex_lock(&mutex_players);

    linkedList_push(g_players, &message_alive->character, sizeof(Character));

    pthread_mutex_unlock(&mutex_players);

    cui_refresh();
}

void handler_dead(Message_Dead *message_dead) {
    Character *c;
    LinkedList *t;
    int i;

    pthread_mutex_lock(&mutex_players);

    for (t = g_players, i = 0; t != NULL && (c = t->data) != NULL; t = t->next, i++) {
        if (c->uuid == message_dead->uuid) {
            linkedList_removeIndexOf(g_players, i);
            break;
        }
    }

    pthread_mutex_unlock(&mutex_players);

    if (message_dead->uuid == g_character.uuid) {
        running = 0;
        socket_close(sock);
        return;
    }

    cui_refresh();
}

void handler_move(Message_Move *message_move) {
    Character *c;
    LinkedList *t;

    pthread_mutex_lock(&mutex_players);

    for (t = g_players; t != NULL && (c = t->data) != NULL; t = t->next)
        if (c->uuid == message_move->uuid) {
            c->x = message_move->x;
            c->y = message_move->y;

            if (g_character.uuid == message_move->uuid) {
                g_character.x = message_move->x;
                g_character.y = message_move->y;
            }

            break;
        }

    pthread_mutex_unlock(&mutex_players);

    cui_refresh();
}

void handler_attack(Message_Attack *buf) {
    // ToDo : Implement attack log.
    return;
}

void *thread_network(void *args) {
    char buf[MAX_BUF];

    while (running) {
        memset(buf, 0, MAX_BUF);

        if (packet_receive(sock, buf) < 0) {
            printf("Disconnected from server.\n");
            running = 0;

            break;
        }

        switch (((Message *) &buf)->type) {
            case MES_QUIT:
                running = 0;
                break;

            case MES_PROFILE:
                handler_profile((Message_Profile *) buf);
                break;

            case MES_MAP:
                handler_map((Message_Map *) buf);
                break;

            case MES_ALIVE:
                handler_alive((Message_Alive *) buf);
                break;

            case MES_DEAD:
                handler_dead((Message_Dead *) buf);
                break;

            case MES_MOVE:
                handler_move((Message_Move *) buf);
                break;

            case MES_ATTACK:
                handler_attack((Message_Attack *) buf);
                break;

            case MES_LOGIN:
            default:
                printf("thread_network(): Unknown packet recieved. [%d]\n", ((Message *) &buf)->type);
                break;
        }
    }

    socket_close(sock);

    return NULL;
}

void set_color_pair() {
    init_pair(1, COLOR_WHITE, COLOR_GREEN);
    init_pair(2, COLOR_WHITE, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_RED);
}

void *thread_cui(void *args) {
    int key;

    initscr();
    start_color();
    set_color_pair();

    cbreak(); // 入力バッファを使用しない
    noecho(); // キー入力された文字を表示しない
    keypad(stdscr, TRUE); // カーソルキーを有効にする
    curs_set(0); // カーソルを表示しない

    mapWindow = stdscr;
    logWindow = newwin(LINES / 4, COLS / 2 - 1, 3 * LINES / 4, 1);
    statusWindow = newwin(LINES / 4, COLS / 3 - 2, 3 * LINES / 4, COLS / 2 + 1);
    playerWindow = newwin(LINES / 4, COLS / 6 - 1, 3 * LINES / 4, 5 * COLS / 6 + 1);

    while (running) {
        switch ((key = getch())) {
            case 'q':
                running = 0;
                socket_close(sock);
                break;

            case 'L':
                show_logWindow = show_logWindow == 1 ? 0 : 1;
                pthread_mutex_lock(&mutex_players);
                cui_refresh();
                pthread_mutex_unlock(&mutex_players);
                break;

            case 'S':
                show_statusWindow = show_statusWindow == 1 ? 0 : 1;
                pthread_mutex_lock(&mutex_players);
                cui_refresh();
                pthread_mutex_unlock(&mutex_players);
                break;

            case 'P':
                show_playerWindow = show_playerWindow == 1 ? 0 : 1;
                pthread_mutex_lock(&mutex_players);
                cui_refresh();
                pthread_mutex_unlock(&mutex_players);
                break;

            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                do_move(key);
                break;

            default:
                break;
        }
    }

    refresh();

    sleep(1);

    endwin();

    return NULL;
}

int main(int argc, char *argv[]) {
    int i;
    pthread_t threads[2];

    printf("Rogue\n");

    if (argc != 3) {
        printf("Usage : %s host port\n", argv[0]);
        return 0;
    }

    if ((sock = socket_connect(argv[1], argv[2])) < 0)
        return -1;

    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("signal()");
        return -1;
    }

    pthread_mutex_init(&mutex_cui, NULL);
    pthread_mutex_init(&mutex_players, NULL);

    g_players = linkedList_init();

    show_logWindow = 0;
    show_statusWindow = 1;
    show_playerWindow = 1;

    do_login();

    packet_no_printf(1);

    pthread_create(&threads[0], NULL, thread_network, NULL);
    pthread_create(&threads[1], NULL, thread_cui, NULL);

    for (i = 0; i < 2; ++i) {
        pthread_join(threads[i], NULL);
    }

    linkedList_free(g_players);

    map_free(&g_map);

    return 0;
}
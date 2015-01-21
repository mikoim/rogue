#ifndef HEADER_AUTH_H
#define HEADER_AUTH_H

#include <stdint.h>
#include "message.h"

typedef struct {
    char username[11];
    char password[33];

    Character character;
} UserData;

int auth_init();

int auth_free();

int auth_login(const char *username, const char *password, Character **character);

void auth_dump();

#endif
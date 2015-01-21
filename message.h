#ifndef HEADER_MESSAGE_H
#define HEADER_MESSAGE_H

#include <stdint.h>
#include <time.h>

enum {
    MES_QUIT, // Message
    MES_LOGIN, // Message_Login
    MES_PROFILE, // Message_Profile
    MES_MAP,
    MES_ALIVE,
    MES_DEAD,
    MES_MOVE,
    MES_ATTACK
};

enum {
    STATUS_NG,
    STATUS_OK
};

enum {
    CHARACTER_TYPE_PLAYER,
    CHARACTER_TYPE_MOB
};

#pragma pack(1)

typedef struct {
    int16_t uuid;
    uint8_t type;

    uint8_t depth;
    uint16_t x, y;

    char name[11];
    uint8_t lv;

    struct {
        int64_t base, cur, max;
    } hp;

    struct {
        int64_t cur, max;
    } exp;

    struct {
        int64_t base, cur;
    } atk;

    struct {
        int64_t base, cur;
    } def;

    time_t delay;
} Character;

typedef struct {
    uint8_t type;
    uint8_t status;
    char data[1]; // Dummy
} Message;

typedef struct {
    uint8_t type; // This field must be MES_LOGIN.
    uint8_t status;

    char username[11];
    char password[33];
} Message_Login;

typedef struct {
    uint8_t type; // This field must be MES_PROFILE.
    uint8_t status;

    Character profile;
} Message_Profile;

typedef struct {
    uint8_t type; // This field must be MES_MAP.
    uint8_t status;

    uint32_t seed;
    uint16_t x, y;
} Message_Map;

typedef struct {
    uint8_t type; // This field must be MES_ALIVE.
    uint8_t status;

    Character character;
} Message_Alive;

typedef struct {
    uint8_t type; // This field must be MES_DEAD.
    uint8_t status;

    int16_t uuid;
    int16_t by_uuid;
} Message_Dead;

typedef struct {
    uint8_t type; // This field must be MES_MOVE.
    int16_t status;

    int16_t uuid;
    uint16_t x, y;
} Message_Move;

typedef struct {
    uint8_t type; // This field must be MES_ATTACK.
    uint8_t status;

    int16_t uuid;
    int16_t by_uuid;

    uint64_t damage;
    uint64_t exp;
} Message_Attack;

#pragma pack(0)

#endif
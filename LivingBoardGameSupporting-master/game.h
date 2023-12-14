#ifndef __GAME_H__
#define __GAME_H__

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include <string.h>

// Token Types
#define PLAYER  0
#define EQUIP   1

// Player Classes
#define MUTANT  5
#define ROBOT   4
#define DOG   1
#define STRAW   6
#define GOBLIN   3
#define SCIENTIST   2
#define STARTING_CLASS DOG
#define NUM_CLASSES 6+1

// Equipment Classes
#define SPECIAL 1
#define HELMET 2
#define FIREARM 3
#define WEAPON 4
#define ARMOR 5

typedef struct{
    uint8_t type : 1;
    uint8_t class : 3;
    uint8_t na : 4;
    uint8_t reserved;
    uint8_t unknown;
    uint8_t last;
} message_pramble;

typedef union{
    uint8_t stat[16];
    struct{
        int8_t speed;
        int8_t view;
        int8_t health;
        int16_t gold;
        int8_t armor;
        int8_t attack;
        uint8_t special;
        int8_t reserved[8];
    };
}stat_block;

typedef struct{
    uint8_t uid[7];
    message_pramble preamble;
    uint8_t text_line[8][14];
    stat_block stats;
    uint8_t reserved[28];
    union{
        struct{
            uint8_t figurine_spacer[104];
            uint8_t portrait[240];
        };
        struct{
            uint8_t full_image[320];
            uint8_t equipment_image[24];
        };
    };
} NTAG215;

typedef struct{
    uint8_t *title;
    uint8_t *body;
    stat_block stats;
    uint8_t portrait;
} class_entry;

extern class_entry classes[NUM_CLASSES];
void fill_stats(uint8_t *text_buff, stat_block stats);
void add_stats(stat_block *end_stats, stat_block *changing_stats);
void sub_stats(stat_block *end_stats, stat_block *changing_stats);
#endif
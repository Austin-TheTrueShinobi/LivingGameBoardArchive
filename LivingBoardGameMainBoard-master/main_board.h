#ifndef MAIN_BOARD_H
#define MAIN_BOARD_H

#include "pico/stdlib.h"

#define PN532_CS_0  5   // Slot 1
#define PN532_CS_1  6   // Slot 2
#define PN532_CS_2  7   // Slot 3
#define PN532_CS_3  8   // Slot 4
#define PN532_CS_4  9   // Final Boss Slot
#define PN532_CS_5  10  // Trash One
#define PN532_CS_6  11  // Write Token

#define ERROR_NFC 0xAA

#define NUM_PN532 7


typedef union{
    struct{
        uint8_t type;
        uint8_t NFC_ID;
        uint8_t uid[7];
        uint8_t class;
        uint8_t stats[16];
    };
    uint8_t message_buffer[26];
} message_structure;

typedef union{
    struct{
        uint8_t NFC_ID;
        uint8_t class;
        uint8_t stats[16];
        uint8_t text[112];
        uint8_t full_image;
        uint8_t equip_image;
    };
    uint8_t message_buffer[132];
} incoming_message;

#endif
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "player_main.h"
#include "PN532.h"
#include "PN532_SPI.h"

uint8_t pn532_packetbuffer[64];


bool readPassiveTargetID(uint cs_gpio, uint8_t cardbaud, uint8_t *uid, uint8_t *uid_len, uint16_t timeout){
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1; // Max cards 1
    pn532_packetbuffer[2] = cardbaud;
    if(writeCommand(cs_gpio, pn532_packetbuffer, 3, NULL, 0)){
        printf("Read Command Failed\n");
        return false;
    }
    if(readResponse(cs_gpio, pn532_packetbuffer, 64, timeout) < 0){
        return 0;
    }

    if(pn532_packetbuffer[0] != 1){
        return 0;
    }

    uint16_t sens_res = pn532_packetbuffer[2];
    sens_res <<= 8;
    sens_res |= pn532_packetbuffer[3];

    //printf("ATQA: 0x%04x ", sens_res);
    //printf("SAK: 0x%02x", pn532_packetbuffer[4]);
    //printf("\n");

    /* Card appears to be Mifare Classic */
    *uid_len = pn532_packetbuffer[5];

    for (uint8_t i = 0; i < pn532_packetbuffer[5]; i++) {
        uid[i] = pn532_packetbuffer[6 + i];
    }

    return 1;
}

bool SAMconfig(uint cs_gpio){
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // Normal
    pn532_packetbuffer[2] = 0x14; // Timeout 50*20ms
    pn532_packetbuffer[3] = 0x01; // USE IRQ

    if(writeCommand(cs_gpio, pn532_packetbuffer, 4, NULL, 0)){
        printf("Failed write");
        return false;
    }
    int16_t result = readResponse(cs_gpio, pn532_packetbuffer, 64, 1000);
    printf("Result of read: %d", result);
    return (0 <= result);
}
/**
 * @brief Reads 4 pages starting at supplied page
 * 
 * @param cs_gpio 
 * @param page 
 * @param buffer 
 * @return 
 */
uint8_t ntag215_Read4Page(uint cs_gpio, uint8_t page, uint8_t *buffer){
    // User Bytes in 4-129
    if(page > 129){
        return 0;
    }
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;               /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ; /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page; /* Page Number (0..63 in most cases) */
    if(writeCommand(cs_gpio, pn532_packetbuffer, 4, NULL, 0)){
        printf("ReadPage Command Failed\n");
        return false;
    }
    int16_t result = readResponse(cs_gpio, pn532_packetbuffer, 64, 1000);
    if(pn532_packetbuffer[0] == 0x00){
        memcpy(buffer, pn532_packetbuffer + 1, 16);
    }
    else{
        return 0;
    }
    return 1;
}

/**
 * @brief Reads one page. (Although it actually reads 4 and discards rest)
 * @param cs_gpio 
 * @param page 
 * @param buffer 
 * @return 
 */
uint8_t ntag215_ReadPage(uint cs_gpio, uint8_t page, uint8_t *buffer){
    // User Bytes in 4-129
    if(page > 129){
        return 0;
    }
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;               /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ; /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page; /* Page Number (0..63 in most cases) */
    if(writeCommand(cs_gpio, pn532_packetbuffer, 4, NULL, 0)){
        printf("ReadPage Command Failed\n");
        return false;
    }
    int16_t result = readResponse(cs_gpio, pn532_packetbuffer, 64, 1000);
    if(pn532_packetbuffer[0] == 0x00){
        memcpy(buffer, pn532_packetbuffer + 1, 4);
    }
    else{
        return 0;
    }
    return 1;
}


uint8_t ntag215_WritePage(uint cs_gpio, uint8_t page, uint8_t *buffer){
    // User Bytes in 4-129
    if(page > 129){
        return 0;
    }
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;               /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE_ULTRALIGHT; /* Mifare UL Write cmd = 0xA2 */
    pn532_packetbuffer[3] = page; /* Page Number (0..63 in most cases) */
    if(writeCommand(cs_gpio, pn532_packetbuffer, 4, buffer, 4)){
        printf("WritePage Command Failed\n");
        return false;
    }
    return (0 <= readResponse(cs_gpio, pn532_packetbuffer, 64, 1000));
}
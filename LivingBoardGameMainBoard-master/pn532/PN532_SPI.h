#ifndef PN532_SPI_H
#define PN532_SPI_H

/*
    Adapting Code and structures from https://github.com/elechouse/PN532
    However, certain features are not needed for out implementation
        and this is in CPP
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "player_main.h"
#include "PN532.h"

#define STATUS_READ 2
#define DATA_WRITE  1
#define DATA_READ   3
// #define STATUS_READ 0x40
// #define DATA_WRITE  0x80
// #define DATA_READ   0xC0
void wakeup(uint cs_gpio);
void setupPN532CS(uint sck_pin);
int8_t readAckFrame(uint cs_gpio);
int spi_write_blocking_lsb(spi_inst_t *spi, uint8_t *src, size_t len);
bool isReady(uint cs_gpio);
void writeFrame(uint cs_gpio, uint8_t *header, uint8_t header_len, uint8_t *body, uint8_t body_len);
int8_t writeCommand(uint cs_gpio, uint8_t *header, uint8_t header_len, uint8_t *body, uint8_t body_len);
int16_t readResponse(uint cs_gpio, uint8_t *buf, uint8_t len, uint16_t timeout);
void setupSPI(spi_inst_t *spi, uint sck_pin, uint mosi_pin, uint miso_pin);

#endif
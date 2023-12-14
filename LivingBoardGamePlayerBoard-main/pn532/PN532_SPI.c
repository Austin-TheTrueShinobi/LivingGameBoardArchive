#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "player_main.h"
#include "misc.h"
#include "PN532.h"
#include "PN532_SPI.h"


uint8_t data_read = DATA_READ;
uint8_t data_write = DATA_WRITE;
uint8_t status_read = STATUS_READ;

uint8_t command = 0;

uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

uint8_t message_start[] = {PN532_PREAMBLE, PN532_STARTCODE1, PN532_STARTCODE2};

/**
 * \brief Sets up the SPI periperial interface with requested pins
 * Sets parameters to what is expected by PN532
 * 
 * \param spi The spi interface to configure. Should not be configured already
 * \param sck_pin The clock pin. Should be a valid pin based on the pinout
 * \param mosi_pin The mosi pin. Should be a valid pin based on pinout
 * \param miso_pin The miso pin. Should be a valid pin based on the pinout
*/
void setupSPI(spi_inst_t *spi, uint sck_pin, uint mosi_pin, uint miso_pin){
    // Initialize the SPI module
    spi_init(SPI0, SPI0_BAUD);
    // Note, Pico requires MSB, PN532 requires LSB
    // Setting the format to 8 bits and Mode 0
    spi_set_format(SPI0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    // Setting up the SPI pins
    gpio_set_function(sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(miso_pin, GPIO_FUNC_SPI);
}

// Sets up a CS pin and sets it to high automatically
void setupPN532CS(uint sck_pin){
    gpio_init(sck_pin);
    gpio_set_dir(sck_pin, true);
    gpio_put(sck_pin, 1);
    wakeup(sck_pin);
}

void wakeup(uint cs_gpio){
    gpio_put(cs_gpio, false);
    sleep_ms(2);
    gpio_put(cs_gpio, true);
}

int8_t readAckFrame(uint cs_gpio){
    uint8_t ackBuff[sizeof(PN532_ACK)];
    gpio_put(cs_gpio, false);
    // May decrease depending on performance
    sleep_ms(1);
    spi_write_blocking_lsb(SPI0, &data_read, 1);
    spi_read_blocking(SPI0, 0xFF, ackBuff, sizeof(PN532_ACK));
    swap_bits(ackBuff, sizeof(PN532_ACK));
    gpio_put(cs_gpio, true);
    
    return (0 == memcmp(PN532_ACK, ackBuff, sizeof(PN532_ACK)));
}

int8_t writeCommand(uint cs_gpio, uint8_t *header, uint8_t header_len, uint8_t *body, uint8_t body_len){
    command = header[0];
    writeFrame(cs_gpio, header, header_len, body, body_len);

    uint8_t timeout = PN532_ACK_WAIT_TIME;
    while(!isReady(cs_gpio)){
        sleep_ms(1);
        timeout--;
        if(0 == timeout){
            printf("Timeout waiting for ACK");
            return -2;
        }
    }
    if(!readAckFrame(cs_gpio)){
        printf("Invalid ACK\n");
        return PN532_INVALID_ACK;
    }
    return 0;
}
void writeFrame(uint cs_gpio, uint8_t *header, uint8_t header_len, uint8_t *body, uint8_t body_len){
    gpio_put(cs_gpio, false);
    sleep_ms(2);
    // Declaring a write
    spi_write_blocking_lsb(SPI0, &data_write, 1);
    // Writing preamble/header
    spi_write_blocking_lsb(SPI0, message_start, 3);
    // Length of data
    uint8_t len = header_len + body_len + 1;
    spi_write_blocking_lsb(SPI0, &len, 1);
    // Checksum
    len = ~len + 1;
    spi_write_blocking_lsb(SPI0, &len, 1);
    // Writing Rest
    len = PN532_HOSTTOPN532;
    spi_write_blocking_lsb(SPI0, &len, 1);
    // Summing the data 
    uint8_t sum = PN532_HOSTTOPN532;
    // Sending Header
    spi_write_blocking_lsb(SPI0, header, header_len);
    // Sending Body
    spi_write_blocking_lsb(SPI0, body, body_len);

    for(int i = 0; i < header_len; i++){
        sum += header[i];
    }
    for(int i = 0; i < body_len; i++){
        sum += body[i];
    }
    // Checksum
    sum = ~sum + 1;
    spi_write_blocking_lsb(SPI0, &sum, 1);
    len = PN532_POSTAMBLE;
    spi_write_blocking_lsb(SPI0, &len, 1);
    gpio_put(cs_gpio, true);
}

bool isReady(uint cs_gpio){
    uint8_t temp = STATUS_READ;
    gpio_put(cs_gpio, false);
    // May decrease depending on performance
    sleep_ms(1);
    spi_write_blocking_lsb(SPI0, &temp, 1);
    spi_read_blocking(SPI0, 0xFF, &temp, 1);
    swap_bits(&temp, 1);
    gpio_put(cs_gpio, true);
    return temp & 1;
}


int spi_write_blocking_lsb(spi_inst_t *spi, uint8_t *src, size_t len){
    swap_bits(src, len);
    spi_write_blocking(spi, src, len);
    swap_bits(src, len);
}

int16_t readResponse(uint cs_gpio, uint8_t *buf, uint8_t len, uint16_t timeout){
    uint16_t timeout_attempts = 0;
    uint8_t cmp_buf[3], excess_buf;
    while(!isReady(cs_gpio)){
        sleep_ms(1);
        timeout_attempts++;
        if(timeout > 0 && timeout_attempts > timeout){
            printf("Timeout waiting for response\n");
            return PN532_TIMEOUT;
        }
    }

    gpio_put(cs_gpio, false);
    sleep_ms(1);

    int16_t result;

    do{
        // Declaring a write
        spi_write_blocking_lsb(SPI0, &data_read, 1);

        spi_read_blocking(SPI0, 0xFF,cmp_buf, 3);
        swap_bits(cmp_buf, 3);
        if(0 != memcmp(cmp_buf, message_start, 3)){
            printf("Invalid Start\n");
            result = PN532_INVALID_FRAME;
            break; 
        }
        // Both length and length checksum
        spi_read_blocking(SPI0, 0xFF, cmp_buf, 2);
        swap_bits(cmp_buf, 2);
        uint8_t length = cmp_buf[0];
        if((uint8_t)(cmp_buf[0] + cmp_buf[1]) != 0){
            printf("Bad Length");
            result = PN532_INVALID_FRAME;
            break;
        }
        // Getting both command and direction
        uint8_t cmd = command + 1;
        spi_read_blocking(SPI0, 0xFF, cmp_buf, 2);
        swap_bits(cmp_buf, 2);
        if(PN532_PN532TOHOST != cmp_buf[0] || cmd != cmp_buf[1]){
            printf("Bad command/return");
            result = PN532_INVALID_FRAME;
            break;
        }
        // Printing the rest of the message
        length -= 2;
        // printf("Command %x Length %d\n :", cmd, length);
        // Checking if the provided buffer is large enough
        if(length > len){
            for(uint8_t i = 0; i < length; i++){
                spi_read_blocking(SPI0, 0xFF, &excess_buf, 1);
                swap_bits(&excess_buf, 1);
                // printf("%02x ", excess_buf);
            }
            printf("\nNot enough space\n");
            spi_read_blocking(SPI0, 0xFF, cmp_buf, 2);
            result = PN532_NO_SPACE;
            break;
        }
        // Retreiving the message and checksum
        uint8_t sum = PN532_PN532TOHOST + cmd;
        for(uint8_t i = 0; i < length; i++){
            spi_read_blocking(SPI0, 0xFF, &(buf[i]), 1);
            swap_bits(&(buf[i]), 1);
            sum += buf[i];
            // printf("%02x ", buf[i]);
        }
        printf("\n");
        // Read Message Checksum
        spi_read_blocking(SPI0, 0xFF, &excess_buf, 1);
        swap_bits(&excess_buf, 1);
        if(0 != (uint8_t)(sum + excess_buf)){
            printf("Checksum not correct\n");
            result = PN532_INVALID_FRAME;
            break;
        }
        
        spi_read_blocking(SPI0, 0xFF, &excess_buf, 1);
        result = length;

    }while(0);
    gpio_put(cs_gpio, true);
    return result;
}
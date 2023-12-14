#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "misc.h"
#include "PN532_SPI.h"
#include "PN532.h"
#include "SSD1306.h"
#include "main_board.h"
#include "images.h"
#include "game.h"
#include "servo.h"

void check_w();

uint8_t uids[NUM_PN532][7] = {0};
uint8_t uid[7] = {};
uint8_t uid_length = 7;
uint8_t image = 0;
uint8_t equipment_image = 0;
uint8_t page_buffer[4] = {};
uint cs_list[NUM_PN532] = {PN532_CS_0, PN532_CS_1, PN532_CS_2, PN532_CS_3, PN532_CS_4, PN532_CS_5, PN532_CS_6};
uint valid_cs[NUM_PN532] = {0}, num_cs = 0;
message_structure out_message_buff;
incoming_message in_message_buff;
message_pramble preamb;
bool can_write = false;

int main() {


    motor_init(MOTOR5_GPIO);
    motor_init(MOTOR6_GPIO);
    // turn_on_motor(MOTOR_GPIO);
    // while(1){
    //     //turn_on_motor(MOTOR_GPIO);
    //     set_motor_angle(MOTOR_GPIO, 0xFF);
    //     sleep_ms(600);
    //     //turn_off_motor(MOTOR_GPIO);
    //     sleep_ms(2000);
    //     //turn_on_motor(MOTOR_GPIO);
    //     set_motor_angle(MOTOR_GPIO, 0x7F);
    //     sleep_ms(600);
    //     //turn_off_motor(MOTOR_GPIO);
    //     sleep_ms(2000);
    //     //turn_on_motor(MOTOR_GPIO);
    //     set_motor_angle(MOTOR_GPIO, 0x00);
    //     sleep_ms(600);
    //     //turn_off_motor(MOTOR_GPIO);
    //     sleep_ms(2000);
    // }




    // Setting STDIO
    // Allows printouts
    stdio_init_all();

    // Wait for the start message
    while(getchar_timeout_us(0) != 'S');
    // printf("Test1");
    // Initializing PN532s
    setupSPI(SPI0, SPI0_SCK, SPI0_MOSI, SPI0_MISO);
    for(uint8_t i = 0; i < NUM_PN532; i++){
        setupPN532CS(cs_list[i]);
        if(!SAMconfig(cs_list[i])){
            //printf("%d SAM Failed\n", i);
            continue;
        }
        if(cs_list[i] == PN532_CS_6){
            can_write = true;
            continue;
        }
        valid_cs[num_cs] = cs_list[i];
        num_cs++;
    }
    
    //printf("Setup %d and can read? %d:\n", num_cs, can_write);
    // Starting the Progra
    while (1) {
        // Go throuhg each NFC reader on the board
        for(uint8_t i = 0; i < num_cs; i++){
            // If it finds a chip
            if(readPassiveTargetID(valid_cs[i], PN532_MIFARE_ISO14443A, uid, &uid_length, 300)){
                // If this chip has already been read and sent
                if(!memcmp(uid, uids[i], 7)) continue;
                // If it is a new chip, read the header
                if(ntag215_ReadPage(valid_cs[i], 4, (uint8_t *) &preamb)){
                    // If it is a player on the 4 slots or an equipment in the trash
                    if(preamb.type == PLAYER || (valid_cs[i] == PN532_CS_5)){
                        // Set up the writing message
                        out_message_buff.type = 'R';
                        // Read out the stats
                        ntag215_Read4Page(valid_cs[i], 33, out_message_buff.stats);
                        // Save the UID
                        memcpy(uids[i], uid, 7);
                        // Set the NFC Pad
                        out_message_buff.NFC_ID = valid_cs[i] - PN532_CS_0;
                        // Send which class the object is
                        out_message_buff.class = preamb.class;
                        // Send the UID
                        memcpy(out_message_buff.uid, uids[i], 7);
                        // Write out the whole message
                        send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
                        // If this is the trash, erase header at least, then move it
                        if(valid_cs[i] == PN532_CS_5){
                            ntag215_WritePage(valid_cs[i], 4, page_buffer);
                            turn_on_motor(MOTOR5_GPIO);
                            set_motor_angle(MOTOR5_GPIO, 0x00);
                            sleep_ms(500);
                            set_motor_angle(MOTOR5_GPIO, 0xFF);
                            sleep_ms(500);
                            turn_off_motor(MOTOR5_GPIO);
                            // MOVE THE MOTOR TO STACK THE CHIP
                            // move_stack_motor();[TODO]
                        }
                    }
                }
            }
            // If not chip was detected
            else{
                // If there was supposed to be a chip present
                if(uids[i][0] != 0 || uids[i][1] != 0 || uids[i][2] != 0 || uids[i][3] != 0){ 
                    // Send the UID of the missing chip
                    // Zero everything else
                    if(valid_cs[i] != PN532_CS_5){
                        memcpy(out_message_buff.uid, uids[i], 7);        
                        memset(out_message_buff.stats, 0, 16);
                        out_message_buff.type = 'R';
                        out_message_buff.class = 0;
                        out_message_buff.NFC_ID = valid_cs[i] - PN532_CS_0;
                        send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
                    }
                    // Blank the stored UID for the slot
                    memset(uids[i], 0, 7);
                }
            }
            check_w();
        }
        check_w();
    }
}


void check_w(){
     // Check for a Write command from the PI
    if(getchar_timeout_us(0) == 'W'){
        // printf("Test");
        // Get the NFC reader it is meant for
        in_message_buff.NFC_ID = getchar_timeout_us(0);
        // If it is for the dispenser
        if(in_message_buff.NFC_ID == PN532_CS_6-PN532_CS_0 && can_write){
            // Get full data from Host
            // Get Token Class
            in_message_buff.class = getchar_timeout_us(1000);
            // Grab Stats
            for(uint i = 0; i < 16; i++){
                in_message_buff.stats[i] = getchar_timeout_us(1000);
            }
            // Get text
            for(uint i = 0; i < 112; i++){
                in_message_buff.text[i] = getchar_timeout_us(1000);
            }
            // Get full image
            in_message_buff.full_image = getchar_timeout_us(1000);
            // Get equip image
            in_message_buff.equip_image = getchar_timeout_us(1000);
            //send_bytes(in_message_buff.message_buffer, sizeof(in_message_buff));
            // Move the token to the reader
            
            turn_on_motor(MOTOR6_GPIO);
            set_motor_angle(MOTOR6_GPIO, 0x00);
            sleep_ms(500);

            // Verify That token is on reader
            if(!readPassiveTargetID(PN532_CS_6, PN532_MIFARE_ISO14443A, uid, &uid_length, 300)){
                out_message_buff.type = 'R';
                // Error Byte
                out_message_buff.NFC_ID = ERROR_NFC;
                // Rest of message doesn't matter
                send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
            }

            // Write out data
            preamb.type = EQUIP;
            preamb.class = in_message_buff.class;
            if(!ntag215_WritePage(PN532_CS_6, 4, (uint8_t *) &preamb)){
                out_message_buff.type = 'R';
                // Error Byte
                out_message_buff.NFC_ID = ERROR_NFC;
                // Rest of message doesn't matter
                send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
            }
            for(uint8_t i = 0; i < 28; i++){
                if(!ntag215_WritePage(PN532_CS_6, i + 5, &in_message_buff.text[i*4])){
                    out_message_buff.type = 'R';
                    // Error Byte
                    out_message_buff.NFC_ID = ERROR_NFC;
                    // Rest of message doesn't matter
                    send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
                }
            }
            for(uint8_t i = 0; i < 4; i++){
                if(!ntag215_WritePage(PN532_CS_6, i + 33, &in_message_buff.stats[i*4])){
                    out_message_buff.type = 'R';
                    // Error Byte
                    out_message_buff.NFC_ID = ERROR_NFC;
                    // Rest of message doesn't matter
                    send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
                }
            }
            for(uint8_t i = 0; i < 6; i++){
                if(!ntag215_WritePage(PN532_CS_6, (129-5)+i, &images_24[in_message_buff.equip_image][i*4])){
                    out_message_buff.type = 'R';
                    // Error Byte
                    out_message_buff.NFC_ID = ERROR_NFC;
                    // Rest of message doesn't matter
                    send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
                }
            }
            for(uint8_t i = 0; i < 80; i++){
                if(!ntag215_WritePage(PN532_CS_6, (129-5-80)+i, &images_320[in_message_buff.full_image][i*4])){
                    out_message_buff.type = 'R';
                    // Error Byte
                    out_message_buff.NFC_ID = ERROR_NFC;
                    // Rest of message doesn't matter
                    send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
                }
            }

            // Move motor further
            set_motor_angle(MOTOR6_GPIO, 0xFF);
            sleep_ms(500);
            turn_off_motor(MOTOR6_GPIO);
        }
        // If it is for the slots
        else if(in_message_buff.NFC_ID < PN532_CS_5 - PN532_CS_0){
            // Only read up to text, ignore image, text, and equip
            // Get the stats
            for(uint i = 0; i < 16; i++){
                in_message_buff.stats[i] = getchar_timeout_us(1000);
            }
            // Verify That token is on reader
            if(!readPassiveTargetID(in_message_buff.NFC_ID + PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 300)){
                out_message_buff.type = 'R';
                // Error Byte
                out_message_buff.NFC_ID = ERROR_NFC;
                // Rest of message doesn't matter
                send_bytes(out_message_buff.message_buffer, sizeof(out_message_buff.message_buffer));
            }
            for(uint i = 0; i < 4; i++){
                ntag215_WritePage(in_message_buff.NFC_ID + PN532_CS_0, 33+i, &in_message_buff.stats[i*4]);
            }
        }
    }
}
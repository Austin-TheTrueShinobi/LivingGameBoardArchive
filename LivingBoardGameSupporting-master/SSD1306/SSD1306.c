#include <string.h>
#include "SSD1306.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "font.h"

uint8_t text_line[8][CHARS_PER_LINE] = {0x20};
uint8_t *text_array = (uint8_t*)text_line;

// Addressing mode will still be Per Page
uint8_t init_cmds[] = {
    SSD1306_COMMAND_STREAM, // Setting up the stream for all commands
    //0xA8, 0x3F,   // Setting mux ratio to 64 (Set by default)
    // 0xD3, 0x00,  // Starting at 0x00 (Set by default)
    // 0x40,        // Set Display Start Line (Set by default)
    // 0xA0,        // Segment Remap
    // 0xC0,        // COM direction
    0xDA, 0x12,  // COM pin hardware config
    // 0x81, 0x7F,  // Set Contrast
    // 0xA4,        // Use RAM contents
    // 0xA6,        // Normal/Not Inverted
    0xD5, 0x80,     // Set Oscillator Freq
    0x8D, 0x14,     // Set Chargepump
    0xAF,            // Turn Display on
};
// Blank the line of characters
uint8_t blank_line[CHARS_PER_LINE * CHARS_COLS_LENGTH] = {0};

// Set Page
uint8_t cmd_buf[8] = {0};

// UI Line
// Also used for equipment
uint8_t text_buff[7] = {
    SSD1306_DATA_STREAM,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
};

uint8_t image_buff[11] = {0};

uint8_t write_text(i2c_inst_t *i2c){
    bool terminated = false;
    // Set to Page Addressing
    cmd_buf[1] = 0x20;
    cmd_buf[2] = 0b00000010;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
    // Set from COLUMN 0-127
    cmd_buf[1] = 0x21;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x7F;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    // Set from PAGE0-7
    cmd_buf[1] = 0x22;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x07;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    for(uint8_t i = 0; i < 8; i++){
        //Setting the Page to i
        cmd_buf[1] = 0xB0 | i;
        i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 2, false);
        // Set to Column Start
        cmd_buf[1] = 0x00;
        cmd_buf[2] = 0x10;
        i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
        // Clearing the Line
        i2c_write_blocking(i2c, SSD1306_ADDR, blank_line, sizeof(blank_line), false);
        // Resetting to column Start
        cmd_buf[1] = 0x00;
        cmd_buf[2] = 0x10;
        i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
        // Printing each character
        if(!terminated){
            for(uint8_t j = 0; j  < CHARS_PER_LINE; j++){
                if(text_line[i][j] == '\0'){
                    terminated = true;
                }
                for(uint8_t k = 0; k < CHARS_COLS_LENGTH; k++){
                    if(text_line[i][j] - 0x20 >= 0 && !terminated){
                        text_buff[k+1] = FONTS[text_line[i][j] - 0x20][k];
                    }
                    else{
                        text_buff[k+1] = 0x00;
                    }
                }
                i2c_write_blocking(i2c, SSD1306_ADDR, text_buff, 7, false);
            }
        }
    }
}

void I2C_init(i2c_inst_t *i2c, uint baudrate, uint SDA_pin, uint SCL_pin){
    i2c_init(i2c, baudrate);
    gpio_set_function(SDA_pin, GPIO_FUNC_I2C);
    gpio_set_function(SCL_pin, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_pin);
    gpio_pull_up(SCL_pin);
}

void clear_display(i2c_inst_t *i2c){
    // Set to Page Addressing
    cmd_buf[1] = 0x20;
    cmd_buf[2] = 0x02;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
    // Set from all Columns
    cmd_buf[1] = 0x21;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x7F;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    // Set from PAGE0-7
    cmd_buf[1] = 0x22;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x07;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    for(uint8_t i = 0; i < 8; i++){
        // Setting the Page to i
        cmd_buf[1] = 0xB0 | i;
        i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 2, false);
        // Clearing the pages
        i2c_write_blocking(i2c, SSD1306_ADDR, blank_line, sizeof(blank_line), false);
        i2c_write_blocking(i2c, SSD1306_ADDR, blank_line, sizeof(blank_line), false);
    }
}

// Sets up columns and etc
void draw_character_ui(i2c_inst_t *i2c){
    // Set to Horizontal Addressing
    cmd_buf[1] = 0x20;
    cmd_buf[2] = 0x00;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
    // Set from PAGE0-7
    cmd_buf[1] = 0x22;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x07;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    // Set from COLUMN 14*6->14*6+4
    cmd_buf[1] = 0x21;
    cmd_buf[2] = CHARS_PER_LINE*CHARS_COLS_LENGTH + 1;
    cmd_buf[3] = CHARS_PER_LINE*CHARS_COLS_LENGTH + 2;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    memset(&text_buff[1], 0xFF, 2);
    for(uint8_t i = 0; i < 8; i++){
        i2c_write_blocking(i2c, SSD1306_ADDR, text_buff, 3, false);
    }
}

void display_error(i2c_inst_t *i2c, uint8_t *string, uint8_t len){
    clear_display(i2c);
    // Set to Horizontal Addressing
    cmd_buf[1] = 0x20;
    cmd_buf[2] = 0x00;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
    // Set from PAGE0-7
    cmd_buf[1] = 0x22;
    cmd_buf[2] = 0x03;
    cmd_buf[3] = 0x05;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    // Set from COLUMN 14*6->14*6+4
    cmd_buf[1] = 0x21;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x7F;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    for(uint8_t j = 0; j < len; j++){
        for(uint8_t k = 0; k < CHARS_COLS_LENGTH; k++){
            if(string[j] - 0x20 >= 0){
                text_buff[k+1] = FONTS[string[j] - 0x20][k];
            }
            else{
                text_buff[k+1] = 0x00;
            }
        }
        i2c_write_blocking(i2c, SSD1306_ADDR, text_buff, 7, false);
    }
}

void draw_image(i2c_inst_t *i2c, uint8_t *image, bool full_screen){
    // Set to Horizontal Addressing
    cmd_buf[1] = 0x20;
    cmd_buf[2] = 0x00;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
    // Set from PAGE0-7
    cmd_buf[1] = 0x22;
    cmd_buf[2] = full_screen? 0 : 2;
    cmd_buf[3] = 0x07;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    // Set from COLUMN 14*6->14*6+4
    cmd_buf[1] = 0x21;
    cmd_buf[2] = CHARS_PER_LINE * CHARS_COLS_LENGTH + 4;
    cmd_buf[3] = 0x7F;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
    for(uint8_t i = full_screen? 0 : 2; i < 8; i++){
        for(uint8_t j = 0; j < 4; j++){
            if(full_screen){
                memcpy(&image_buff[1], &image[((i)*40) + (j * 10)], 10);
            }
            else{
                memcpy(&image_buff[1], &image[((i-2)*40) + (j * 10)], 10);
            }
            i2c_write_blocking(i2c, SSD1306_ADDR, image_buff, 11, false);
        }
    }
}
void draw_equip(i2c_inst_t *i2c, uint8_t equipments[3][24], uint8_t occupied_spots){
    // Set to Horizontal Addressing
    cmd_buf[1] = 0x20;
    cmd_buf[2] = 0x00;
    i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 3, false);
    // For each equipment
    for(uint8_t equipment = 0; equipment < 3; equipment++){
        // Set from COLUMN
        cmd_buf[1] = 0x21;
        cmd_buf[2] = (CHARS_PER_LINE * CHARS_COLS_LENGTH) + 4 + (equipment * 14);
        cmd_buf[3] = (CHARS_PER_LINE * CHARS_COLS_LENGTH) + 4 + (equipment * 14) + 11;
        i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
        // Set from PAGE0-1
        cmd_buf[1] = 0x22;
        cmd_buf[2] = 0x00;
        cmd_buf[3] = 0x01;
        i2c_write_blocking(i2c, SSD1306_ADDR, cmd_buf, 4, false);
        // Per Page
        for(uint8_t i = 0; i < 2; i++){
            // Per Page, write out two bytes
            for(uint8_t j = 0; j < 2; j++){
                if((occupied_spots & 0x01 << equipment) > 0){
                    memcpy(&text_buff[1], &equipments[equipment][(i*12) + (j * 6)], 6);
                    i2c_write_blocking(i2c, SSD1306_ADDR, text_buff, 7, false);
                }
                else{
                    memset(&text_buff[1],0x00, 6);
                    i2c_write_blocking(i2c, SSD1306_ADDR, text_buff, 7, false);
                }
            }
        }
    }

}
// Sets up the display and clears it
void SSD1306_init(i2c_inst_t *i2c){
    i2c_write_blocking(i2c, SSD1306_ADDR, init_cmds, sizeof(init_cmds), false);
    blank_line[0] = SSD1306_DATA_STREAM;
    cmd_buf[0] = SSD1306_COMMAND_STREAM;
    image_buff[0] = SSD1306_DATA_STREAM;
    clear_display(i2c);
}
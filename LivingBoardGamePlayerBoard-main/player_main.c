
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "misc.h"
#include "PN532_SPI.h"
#include "PN532.h"
#include "SSD1306.h"
#include "player_main.h"
#include "pico/multicore.h"
#include "images.h"
#include "game.h"

#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define MAIN_PN PN532_CS_1
#define UDP_PORT 4444 ///< UDP port to send packets to.
#define BEACON_MSG_LEN_MAX 127 ///< Maximum length of the beacon message.
#define BEACON_TARGET "255.255.255.255" ///< Target IP address for broadcasting UDP packets. THIS IS THE STATIC METHOD
#define BEACON_INTERVAL_MS 1000 ///< Interval between UDP packets in milliseconds.


uint8_t uids[NUM_PN532][7] = {0};
uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
uint8_t uid_length = 0;
uint8_t board_uid[] = {0, 0, 0, 0, 0, 0, 0};
uint cs_list[NUM_PN532] = {PN532_CS_0, PN532_CS_1, PN532_CS_2, PN532_CS_3};
uint valid_cs[NUM_PN532] = {0}, num_cs = 0;
NTAG215 player_information;
NTAG215 temp_chip;
uint8_t page_buffer[4];
uint8_t current_equip = 0;
struct udp_pcb* pcb;
struct pbuf *p;
ip_addr_t addr;
uint8_t equipment_images[3][24];
uint8_t up_prev, down_prev, left_prev, right_prev;
uint8_t add_special, remove_special;
stat_block pending_stat_changes, null_stats = {0}, temp_stats, equiped[3];
message_pramble preamb;

volatile uint8_t start_up_select = 0, character_index = STARTING_CLASS, prev_index = STARTING_CLASS;
volatile bool current_text = false, buttons = false, char_select = false, figure_present = false, prev_text = false, player_present = false, debounce = true; 

void main2();

void text_toggle(uint gpio, uint32_t event_mask){
    if(gpio == TEXT){
        if(debounce){
            current_text = !current_text;
            debounce = false;
        }
    }
    else if(debounce && (gpio == LEFT || gpio == RIGHT || gpio == UP)){
        // If Selecting new or existing player
        if(buttons){
            if(gpio == LEFT){
                start_up_select = LEFT;
            }
            else if(gpio == RIGHT){
                start_up_select = RIGHT;
            }
            buttons = false;
        }
        // Selecting character
        if(char_select){
            if(gpio == LEFT){
                if(character_index - 1 >= STARTING_CLASS){
                    character_index--;
                }
            }
            if(gpio == RIGHT){
                if(character_index + 1 < NUM_CLASSES){
                    character_index++;
                }
            }
            if(gpio == UP){
                char_select = false;
            }
        }
    }
    gpio_acknowledge_irq(gpio, event_mask);
}

int main() {
    // stdio_init_all();
    // Zeroing out internal memory on startup
    memset(&player_information, 0, sizeof(NTAG215));
    current_equip = 0;
    // Initialize each SPI module
    setupSPI(SPI0, SPI0_SCK, SPI0_MOSI, SPI0_MISO);
    for(uint8_t i = 0; i < NUM_PN532; i++){
        setupPN532CS(cs_list[i]);
        if(!SAMconfig(cs_list[i])){
            //printf("%d SAM Failed\n", i);
            continue;
        }
        valid_cs[num_cs] = cs_list[i];
        num_cs++;
    }

    // Initialize the Display
    I2C_init(I2C, I2C_BAUD, SDA, SCL);
    SSD1306_init(I2C);

    // Initializing GPIO and Starting Callbacks
    gpio_init(UP);
    gpio_init(DOWN);
    gpio_init(LEFT);
    gpio_init(RIGHT);
    gpio_init(TEXT);
    gpio_pull_up(UP);
    gpio_pull_up(DOWN);
    gpio_pull_up(LEFT);
    gpio_pull_up(RIGHT);
    gpio_pull_up(TEXT);
    gpio_set_irq_enabled_with_callback(UP, GPIO_IRQ_EDGE_FALL, true, &text_toggle);
    gpio_set_irq_enabled_with_callback(LEFT, GPIO_IRQ_EDGE_FALL, true, &text_toggle);
    gpio_set_irq_enabled_with_callback(RIGHT, GPIO_IRQ_EDGE_FALL, true, &text_toggle);
    gpio_set_irq_enabled_with_callback(TEXT, GPIO_IRQ_EDGE_FALL, true, &text_toggle);

    // Check for new or existing player
    do{
        // Ask for Figure
        display_error(I2C, "Present Figure Lower  Reader", 29);
        // Wait for figure to be present (or at least NFC token)
        while(!readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000));
        display_error(I2C, "New Player <-          Existing ->", 35);
        buttons = true;
        while(start_up_select == 0);
        // New Player
        if(start_up_select == LEFT){
            // Show text about T Button
            display_error(I2C, "Press \"T\" to toggle  text Select with UP", 40);
            sleep_ms(4000);
            // Allow Left and Right to move between classes
            clear_display(I2C);
            draw_character_ui(I2C);
            char_select = true;
            prev_index = 10;
            // Display each class
            while(char_select){
                debounce = true;
                // Do we have a new selection
                if(current_text != prev_text){
                    if(current_text){
                        memcpy(text_array, classes[character_index].title, 14);
                        memcpy(&text_array[14], classes[character_index].body, 7*14);
                        write_text(I2C);
                    }
                    else{
                        memcpy(text_array, classes[character_index].title, 14);
                        fill_stats(&text_array[14], classes[character_index].stats);
                        write_text(I2C);
                    }
                    prev_text = current_text;
                }
                if(prev_index == character_index){
                    continue;
                }
                else{
                    prev_index = character_index;
                    draw_image(I2C, images_240[classes[character_index].portrait], false);
                    if(current_text){
                        memcpy(text_array, classes[character_index].title, 14);
                        memcpy(&text_array[14], classes[character_index].body, 7*14);
                        write_text(I2C);
                    }
                    else{
                        memcpy(text_array, classes[character_index].title, 14);
                        fill_stats(&text_array[14], classes[character_index].stats);
                        write_text(I2C);
                    }
                }
            }
            // Set up the player information
            player_information.preamble.type = PLAYER;
            player_information.preamble.class = character_index;
            // Set the image and the text
            memcpy(player_information.portrait, images_240[classes[character_index].portrait], 240);
            memcpy(player_information.stats.stat, classes[character_index].stats.stat, 16);
            memcpy(&player_information.text_line[0], classes[character_index].title, 14);
            memcpy(&player_information.text_line[1], classes[character_index].body, 7*14);
            // Make sure chip is present still
            if(!readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000)){
                display_error(I2C, "Return Figure", 14);
            }
            while(!readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000));
            // Finalize board UID and Player NTAG UID
            memcpy(board_uid, uid, uid_length);
            memcpy(player_information.uid, uid, uid_length);
            // Write Preamble
            while(!ntag215_WritePage(PN532_CS_0, 4, (uint8_t *) &player_information.preamble)){
                readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
            }
            // Write Flavor Text
            for(uint8_t i = 0; i < 28; i++){
                while(!ntag215_WritePage(PN532_CS_0, i + 5, &((uint8_t *) player_information.text_line)[i*4])){
                    readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
                }
            }
            // Write Stats
            for(uint8_t i = 0; i < 4; i++){
                while(!ntag215_WritePage(PN532_CS_0, i + 33, &player_information.stats.stat[i*4])){
                    readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
                }
            }
            // Write Portrait
            for(uint8_t i = 0; i < 60; i++){
                while(!ntag215_WritePage(PN532_CS_0, i + 70, &player_information.portrait[i*4])){
                    readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
                }
            }
            break;
        }
        // Existing Player
        else{
            // Read Preamble
            while(!ntag215_ReadPage(PN532_CS_0, 4, (uint8_t *) &player_information.preamble)){
                display_error(I2C, "Return Figure", 14);
                sleep_ms(2000);
                continue;
            }
            // Check if is a figurine
            if(player_information.preamble.type != PLAYER){
                display_error(I2C, "Not a Figurine", 15);
                sleep_ms(2000);
                continue;
            }
            // Set board UI and player uid
            memcpy(board_uid, uid, uid_length);
            memcpy(player_information.uid, uid, uid_length);
            // Read Flavor Text
            for(uint8_t i = 0; i < 7; i++){
                while(!ntag215_Read4Page(PN532_CS_0, (i*4) + 5, &((uint8_t *) player_information.text_line)[i*16])){
                    readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
                }
            }
            // Write Stats
            while(!ntag215_Read4Page(PN532_CS_0, 33, player_information.stats.stat)){
                readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
            }
            // Read Portrait
            for(uint8_t i = 0; i < 15; i++){
                while(!ntag215_Read4Page(PN532_CS_0, (i*4) + 70, &player_information.portrait[i*16])){
                    readPassiveTargetID(PN532_CS_0, PN532_MIFARE_ISO14443A, uid, &uid_length, 1000);
                }
            }
            break;
        }
    }while(1);

    // Clear Stat changes
    memset(pending_stat_changes.stat, 0, sizeof(pending_stat_changes.stat));

    // At this point, we should have a populated player_information and a 
    // known board ID. This is the starting point of the game and should kick off 
    // Austins input system.

    multicore_launch_core1(main2);

    draw_character_ui(I2C);

    // Making sure booleans set
    buttons = false;
    char_select = false; 

    gpio_set_irq_enabled_with_callback(TEXT, GPIO_IRQ_EDGE_FALL, true, &text_toggle);

    // Kick off core with austins system

        

    // Wait a little
    // sleep_ms(1);
    while (1) {
        // Go through each entry
        for(uint8_t reader = 0; reader < num_cs; reader++){
start:
            debounce = true;
            // If the text was toggled
            if(current_text != prev_text){
                // If the toggling occured while a figure was present
                if((figure_present && player_present) || (!figure_present)){
                    // Swap player text
                    if(current_text){
                        memcpy(text_array,(uint8_t *)player_information.text_line, 8*14);
                        write_text(I2C);
                    }
                    else{
                        memcpy(text_array, player_information.text_line[0], 14);
                        fill_stats(&text_array[14], player_information.stats);
                        write_text(I2C);
                    }
                }
                    // Swap item text
                else if(figure_present){
                    if(current_text){
                        memcpy(text_array,(uint8_t *)temp_chip.text_line, 8*14);
                        write_text(I2C);
                    }
                    else{
                        memcpy(text_array, temp_chip.text_line[0], 14);
                        fill_stats(&text_array[14], temp_chip.stats);
                        write_text(I2C);
                    }
                }
                prev_text = current_text;
            }
            // If it finds a chip
            if(readPassiveTargetID(valid_cs[reader], PN532_MIFARE_ISO14443A, uid, &uid_length, 400)){
                // If this chip has already been read and sent
                if(!memcmp(uid, uids[reader], 7)) continue;
                // If it is a new chip, read the header
                if(ntag215_ReadPage(valid_cs[reader], 4, (uint8_t *) &preamb)){
                    // If this is the main reader
                    if(valid_cs[reader] == PN532_CS_0){
                        // If this is the main player
                        if(!memcmp(uid, player_information.uid, 7)){
                            memcpy(uids[reader], uid, 7);
                            // Notify that a figure is present and player is present
                            figure_present = true;
                            player_present = true;
                            // Get and modify Player Stats
                            if(!ntag215_Read4Page(valid_cs[reader], 33, player_information.stats.stat)){
                                // display_error(I2C, "Reader Fail", 12);
                                // sleep_ms(200);
                                memset(uids[reader], 0, 7);
                                figure_present = false;
                                player_present = false;
                                if(reader != 0)
                                    reader--;
                                goto start;
                            }
                            add_stats(&player_information.stats, &pending_stat_changes);
                            player_information.stats.special &= ~remove_special;
                            player_information.stats.special |= add_special;
                            add_special = 0;
                            remove_special = 0;
                            memset(pending_stat_changes.stat, 0, 16);
                            // Write Stats
                            for(uint8_t i = 0; i < 4; i++){
                                ntag215_WritePage(PN532_CS_0, i + 33, &player_information.stats.stat[i*4]);
                            }
                            draw_image(I2C, player_information.portrait, false);
                            // If toggled to flavor text
                            if(current_text){
                                memcpy(text_array,(uint8_t *)player_information.text_line, 8*14);
                                write_text(I2C);
                            }
                            else{
                                memcpy(text_array, player_information.text_line[0], 14);
                                fill_stats(&text_array[14], player_information.stats);
                                write_text(I2C);
                            }
                            draw_character_ui(I2C);
                        }
                        // Or is an equipment (ignore other players)
                        else if(preamb.type == EQUIP){
                            memcpy(uids[reader], uid, 7);
                            figure_present = true;
                            // Get the equipment stats
                            if(!ntag215_Read4Page(valid_cs[reader], 33, temp_chip.stats.stat)){
                                // display_error(I2C, "Reader Fail", 12);
                                // sleep_ms(200);
                                memset(uids[reader], 0, 7);
                                figure_present = false;
                                if(reader != 0)
                                    reader--;
                                draw_character_ui(I2C);
                                draw_image(I2C, player_information.portrait, false);
                                goto start;
                            }
                            // Read Flavor Text
                            for(uint8_t i = 0; i < 7; i++){
                                if(!ntag215_Read4Page(PN532_CS_0, (i*4) + 5, &((uint8_t *) temp_chip.text_line)[i*16])){
                                    // display_error(I2C, "Reader Fail", 12);
                                    // sleep_ms(200);
                                    memset(uids[reader], 0, 7);
                                    figure_present = false;
                                    if(reader != 0)
                                        reader--;
                                    draw_character_ui(I2C);
                                    draw_image(I2C, player_information.portrait, false);
                                    goto start;
                                }
                            }
                            // Read in the full image
                            for(uint8_t i = 0; i < 20; i++){
                                if(!ntag215_Read4Page(PN532_CS_0, (129-5-80)+(i*4), &temp_chip.full_image[i*16])){
                                    // display_error(I2C, "Reader Fail", 12);
                                    // sleep_ms(200);
                                    memset(uids[reader], 0, 7);
                                    figure_present = false;
                                    if(reader != 0)
                                        reader--;
                                    draw_character_ui(I2C);
                                    draw_image(I2C, player_information.portrait, false);
                                    goto start;
                                }
                            }
                            // If toggled to flavor text
                            if(current_text){
                                memcpy(text_array,(uint8_t *)temp_chip.text_line, 8*14);
                                write_text(I2C);
                            }
                            else{
                                memcpy(text_array, temp_chip.text_line[0], 14);
                                fill_stats(&text_array[14], temp_chip.stats);
                                write_text(I2C);
                            }
                            draw_image(I2C, temp_chip.full_image, true);
                            draw_character_ui(I2C);
                        }
                    }
                    // If this is an equipment reader, check for equipment
                    else if(preamb.type == EQUIP){
                        memcpy(uids[reader], uid, 7);
                        // Set Bit for equipment
                        current_equip |= 0x1 << (valid_cs[reader]-PN532_CS_1);
                        // Read in equipment image
                        for(uint8_t i = 0; i < 6; i++){
                            if(!ntag215_ReadPage(valid_cs[reader], (129-5)+i, &equipment_images[valid_cs[reader]-PN532_CS_1][i*4])){
                                // display_error(I2C, "Reader Fail", 12);
                                // sleep_ms(100);
                                memset(uids[reader], 0, 7);
                                if(reader != 0)
                                    reader--;
                                current_equip &= ~(0x1 << (valid_cs[reader]-PN532_CS_1));
                                draw_character_ui(I2C);
                                draw_image(I2C, player_information.portrait, false);
                                goto start;
                            }
                        }
                        // Draw Currently equipt items
                        draw_equip(I2C, equipment_images, current_equip);

                        // Get stats of item
                        if(!ntag215_Read4Page(valid_cs[reader], 33, temp_stats.stat)){
                            // display_error(I2C, "Reader Fail", 12);
                            // sleep_ms(100);
                            memset(uids[reader], 0, 7);
                            if(reader != 0)
                                reader--;
                            goto start;
                        }
                        // Modify the player if present
                        if(player_present){
                            add_stats(&player_information.stats, &temp_stats);
                            player_information.stats.special |= temp_stats.special;
                            // Write Stats
                            for(uint8_t i = 0; i < 4; i++){
                                ntag215_WritePage(PN532_CS_0, i + 33, &player_information.stats.stat[i*4]);
                            }
                            if(current_text){
                                memcpy(text_array,(uint8_t *)player_information.text_line, 8*14);
                                write_text(I2C);
                            }
                            else{
                                memcpy(text_array, player_information.text_line[0], 14);
                                fill_stats(&text_array[14], player_information.stats);
                                write_text(I2C);
                            }
                        }
                        // If player not present, queue changes
                        else{
                            add_stats(&pending_stat_changes, &temp_stats);
                            add_special |= temp_stats.special;
                        }
                        // Store stats for later removal
                        memcpy(equiped[valid_cs[reader]-PN532_CS_1].stat, temp_stats.stat, 16);
                    }
                }
            }// If not chip was detected
            else{
                // If there was supposed to be a chip present
                if(uids[reader][0] != 0 || uids[reader][1] != 0 || uids[reader][2] != 0 || uids[reader][3] != 0){
                    memset(uids[reader], 0, 7);
                    // If nothing on main reader, mark nothing present and display main player info
                    if(valid_cs[reader] == PN532_CS_0){
                        clear_display(I2C);
                        draw_character_ui(I2C);
                        player_present = false;
                        figure_present = false;
                        draw_image(I2C, player_information.portrait, false);
                        draw_equip(I2C, equipment_images, current_equip);
                        // If toggled to flavor text
                        if(current_text){
                            memcpy(text_array,(uint8_t *)player_information.text_line, 8*14);
                            write_text(I2C);
                        }
                        else{
                            memcpy(text_array, player_information.text_line[0], 14);
                            fill_stats(&text_array[14], player_information.stats);
                            write_text(I2C);
                        }
                    }
                    // If an equipment was removed
                    else{
                        // Remove it from the equipted slot
                        current_equip &= ~(0x1 << (valid_cs[reader]-PN532_CS_1));
                        memset(equipment_images[valid_cs[reader]-PN532_CS_1], 0, 24);
                        draw_equip(I2C, equipment_images, current_equip);
                        // Modify the player if present
                        if(player_present){
                            sub_stats(&player_information.stats, &equiped[valid_cs[reader]-PN532_CS_1]);
                            player_information.stats.special |= equiped[valid_cs[reader]-PN532_CS_1].special;
                            // Write Stats
                            for(uint8_t i = 0; i < 4; i++){
                                ntag215_WritePage(PN532_CS_0, i + 33, &player_information.stats.stat[i*4]);
                            }
                            if(current_text){
                                memcpy(text_array,(uint8_t *)player_information.text_line, 8*14);
                                write_text(I2C);
                            }
                            else{
                                memcpy(text_array, player_information.text_line[0], 14);
                                fill_stats(&text_array[14], player_information.stats);
                                write_text(I2C);
                            }
                        }
                        // If player not present, queue changes
                        else{
                            sub_stats(&pending_stat_changes, &equiped[valid_cs[reader]-PN532_CS_1]);
                            remove_special |= equiped[valid_cs[reader]-PN532_CS_1].special;
                        }
                    }
                }
            }
        }
    }
}

void run_udp_beacon() {

     // Attempt to connect to the specified Wi-Fi network with WPA2 security.
    if (cyw43_arch_wifi_connect_timeout_ms("raspi-webgui", "ChangeMe", CYW43_AUTH_WPA2_AES_PSK, 5000)) {
        printf("Failed to connect to Wi-Fi\n");
        return;
    }
    pcb = udp_new(); // Create a new UDP control block.
    
    ipaddr_aton(BEACON_TARGET, &addr); // Convert target IP address from string to IP format.
    up_prev = 1;
    down_prev = 1;
    left_prev = 1;
    right_prev = 1;
    uint8_t current_up, current_down, current_left, current_right;
    while(1){
        current_up = gpio_get(UP);
        current_down = gpio_get(DOWN);
        current_left = gpio_get(LEFT);
        current_right = gpio_get(RIGHT);

        if((up_prev != current_up && current_up == 0) || 
        (down_prev != current_down && current_down == 0)  ||
        (left_prev != current_left && current_left == 0)  ||
        (right_prev != current_right && current_right == 0) ){
            p = pbuf_alloc(PBUF_TRANSPORT, 11, PBUF_RAM);
            char *req = (char *)p->payload; // Get pointer to the payload of the pbuf.
            memcpy(req, board_uid, 7);
            up_prev = req[7] = current_up;
            down_prev = req[8] = current_down;
            left_prev = req[9] = current_left;
            right_prev = req[10] = current_right;


            // Send the UDP packet to the specified address and port.
            err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);

            pbuf_free(p);
        }
        else if((up_prev != current_up) || 
        (down_prev != current_down)  ||
        (left_prev != current_left)  ||
        (right_prev != current_right)){
            up_prev = current_up;
            down_prev = current_down;
            left_prev = current_left;
            right_prev = current_right;
        }
    }
}


void main2() {
    // stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43 Wi-Fi module\n");
        return;
    }

    cyw43_arch_enable_sta_mode(); // Enable station mode for Wi-Fi module.

    printf("Connecting to Wi-Fi...\n");

    while(1)
        run_udp_beacon(); // Start sending UDP beacon packets.
    
    cyw43_arch_deinit(); // Deinitialize the Wi-Fi module.
    return;
}

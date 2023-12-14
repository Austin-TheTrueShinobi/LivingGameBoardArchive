#include "misc.h"

void swap_bits(uint8_t *buf, size_t size){
    if(buf == NULL){
        return;
    }
    for(int i = 0; i < size; i++){
        // Code borrowed from https://www.wyzant.com/resources/answers/661501/most-efficient-algorithm-for-bit-reversal-from-msb-lsb-to-lsb-msb-in-c
        // Slight modification required
        buf[i] = ((buf[i] & 0b11110000) >> 4) | ((buf[i] & 0b00001111) << 4); 
        buf[i] = ((buf[i] & 0b11001100) >> 2) | ((buf[i] & 0b00110011) << 2);
        buf[i] = ((buf[i] & 0b10101010) >> 1) | ((buf[i] & 0b01010101) << 1);
    }
}
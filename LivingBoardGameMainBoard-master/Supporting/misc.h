#ifndef MISC_H
#define MISC_H

#include "pico/stdlib.h"

/**
 * \brief swaps bits from lsb->msb or msb->lsb on a per byte basis
 *  I.E order of bytes is not changed, but bits are fliped
 * 
 * \param buf buffer of bytes to swap around. Performs inplace
 * \param size Number of bytes to swap in buffer
 * 
*/
void swap_bits(uint8_t *buf, size_t size);

#endif
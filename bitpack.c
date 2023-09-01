/*
 * bitpack.c
 * Nick Dresens 
 * Sam Miller
 * 03/09/2023
 *
 * COMP40 HW4 - Arith 
 * 
 * Purpose: The program is a C implementation of bit-packing functions used 
 *          for encoding and decoding values within a larger 64-bit word. These
 *          functions include checking if an integer value can fit within a 
 *          certain number of bits, retrieving integer values from a word, and
 *          updating a word with a new integer value at a specific bit position.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>a

#include "assert.h"
#include "except.h"
#include "bitpack.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

static const unsigned WORD_LENGTH = 64;


/*
* name:     bool Bitpack_fitsu(uint64_t n, unsigned width)
* purpose:  Determines whether an unsigned integer value can fit within a 
*           specified number of bits.
* returns:  Returns true if the integer value can be represented by the given
*           number of bits, false otherwise.
* inputs:   The unsigned integer value to check.
* output:   True or False depending on 
*/
bool Bitpack_fitsu(uint64_t n, unsigned width) {
        uint64_t mask = 1 << width;
        return n < mask;
}

/*
* name:     bool Bitpack_fitss(int64_t n, unsigned width)
* purpose:  Determines whether a signed integer value can fit within a 
*           specified number of bits.
* returns:  Returns true if the integer value can be represented by the given number
*           of bits, false otherwise.
* inputs:   The signed integer value to check.
* output:   True or false depending on  
*/
bool Bitpack_fitss(int64_t n, unsigned width) {
        int64_t mask_max = (1 << (width - 1)) - 1;
        int64_t mask_min = -(1 << (width - 1));

        return ((n <= mask_max) && (n >= mask_min));
}

/*
* name:     uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
* purpose:  Retrieves an unsigned integer value from a 64-bit word at a 
*           specific bit position and with a specific number of bits.
* returns:  The unsigned integer valuge extracted from the word
* inputs:   The 64 bit word to extract, the number of bits to extract, and the
*           index of the least significant bit    
* output:   The unsigned integer valuge extracted from the word 
*/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb) {
        assert(width <= WORD_LENGTH);
        assert(width + lsb <= WORD_LENGTH);
        
        if (width == 0) {
                return word;
        }
        /* shift left, then right to only grab the word */
        word = word << (WORD_LENGTH - (width + lsb));
        word = word >> (WORD_LENGTH - width);
        return word;
}

/*
* name:     int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
* purpose:  Returns the signed integer value extracted from the word.
* returns:  The 64-bit word to extract the signed integer value from.
* inputs:   The 64 bit word to extract, the number of bits to extract, and the
*           index of the least significant bit   
* output:   Retrieves signed integer value extracted from the word. 
*/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb) {
        assert(width <= WORD_LENGTH);
        assert(width + lsb <= WORD_LENGTH);

        if (width == 0) {
                return word;
        }

        int64_t final = word;
        final = final >> lsb;
        final = final << (WORD_LENGTH - width);
        final = final >> (WORD_LENGTH - width);

        return final;
}

/*
* name:     uint64_t Bitpack_newu(uint64_t word, unsigned width,
*           unsigned lsb, uint64_t value)
* purpose:  Updates a 64-bit word with a new unsigned integer value at a
*           specific bit position and with a specific numer of bits
* returns:  The updated word with a new value inserted at the specified bit 
*           position
* inputs:   The 64-bit word to update with the new unsigned integer value.
*           The number of bits to insert the value into. The least significant
*           bit position to insert into the word. 
* output:   Updated 64-bit word with un unsigned integer value. 
*/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value) {
        assert(width <= WORD_LENGTH);
        assert(width + lsb <= WORD_LENGTH);
        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t mask_h = ~0;
        uint64_t mask_l = ~0;
        mask_h = mask_h << (lsb + width);
        mask_l = mask_l >> (WORD_LENGTH - lsb);

        uint64_t mask = mask_h | mask_l;
        uint64_t final = word & mask;

        value = value << lsb;
        final = final | value;

        return final;
}

/*
* name:     uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned 
*           lsb, int64_t value)
* purpose:  Updates a 64-bit word with a new signed integer value a
*           specific bit position and with a specific number of bits.
* returns:  The updated word with new value inserted at the specified position.
* inputs:   The 64-bit word to update with the new unsigned integer value.
*           The number of bits to insert the value into. The least significant
*           bit position to insert into the word. 
* output:   Updated 64-bit word with un signed integer value.
*/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, int64_t value) {
        assert(width <= WORD_LENGTH);
        assert(width + lsb <= WORD_LENGTH);
        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t mask_h = ~0;
        uint64_t mask_l = ~0;
        mask_h = mask_h << (lsb + width);
        mask_l = mask_l >> (WORD_LENGTH - lsb);

        uint64_t mask = mask_h | mask_l;
        uint64_t final = word & mask;

        value = value << lsb;
        final = final | value;

        return final;
}
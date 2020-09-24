/* 
 * bitpack.c
 * Jun Sun Tak (jtak01), Eric Duanmu (eduanm01), 3/5/20
 * hw4: Arith
 * 
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "assert.h"
#include "except.h"
#include "bitpack.h"

#define MAX_BIT_WIDTH 64

Except_T Bitpack_Overflow = { "Overflow packing bits" };

static uint64_t left_shift(uint64_t num, unsigned value);
static uint64_t create_mask(unsigned width, unsigned lsb);
static uint64_t u_right_shift(uint64_t num, unsigned value);
static int64_t s_right_shift(int64_t num, unsigned value);
static uint64_t flip(uint64_t word, int64_t value, unsigned width, 
												   unsigned lsb);

/* 
 * name    : create_mask
 * purpose : Helper function for creating a mask of inputted width 
 *           at inputted lsb
 * inputs  : unsigned width, unsigned lsb
 * outputs : binary mask that was created
 */
static uint64_t create_mask(unsigned width, unsigned lsb)
{
	uint64_t mask = left_shift(~0, MAX_BIT_WIDTH - width);
	mask = u_right_shift(mask, MAX_BIT_WIDTH - (width + lsb));

	return mask;
}

/* 
 * name    : left_shift
 * purpose : Helper function for performing left_shift
 * inputs  : uint64_t num, unsigned value
 * outputs : num that was left-shifted by the value
 */
static uint64_t left_shift(uint64_t num, unsigned value)
{
	assert(value <= MAX_BIT_WIDTH);

	if (value < MAX_BIT_WIDTH) {
		return num << value;
	}

	return 0;
}

/* 
 * name    : s_right_shift
 * purpose : Helper function for performing right-shift on singed integers
 * inputs  : int64_t num, unsigned value
 * outputs : num that was right-shifted by the value
 */
static int64_t s_right_shift(int64_t num, unsigned value)
{
	assert(value <= MAX_BIT_WIDTH);

	if (value < MAX_BIT_WIDTH) {
		return num >> value;
	} 
	if (num >= 0) {
		return 0;
	}

	return ~0;
}

/* 
 * name    : right_shift
 * purpose : Helper function for performing right-shift on unsigned integers
 * inputs  : uint64_t num, unsigned value
 * outputs : num that was right-shifted by the value
 */
static uint64_t u_right_shift(uint64_t num, unsigned value)
{
	assert(value <= 64);

	if (value <= 63) {
		return num >> value;
	}

	return 0;
}

/* 
 * name    : flip
 * purpose : helper function that takes in a word and inserts the given value
             at the approporiate width and lsb
 * inputs  : uint64_t num, int64_t value, unsigned width, unsigned lsb
 * outputs : A 64 bit word that now has the given value at the given lsb
 */
static uint64_t flip(uint64_t word, int64_t value, unsigned width, 
												   unsigned lsb)
{
	int64_t mask = create_mask(width, lsb);

	int64_t temp = left_shift(value, lsb) & mask;
	word &= ~mask;
	word |= temp;

	return word;
}

/* 
 * name    : Bitpack_fitsu
 * purpose : Function that checks whether a given value can be represented by
             width number of bits
 * inputs  : uint64_t n, unsigned width
 * outputs : Whether the given number can be stored in the given width
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
	assert(width <= 64);

    if (width == 0 || left_shift(1, width) - 1 < n) {
        return false;
	}

	return true;
}

/* 
 * name    : Bitpack_fitss
 * purpose : Function that checks whether a given value can be represented by 
             width number of bits
 * inputs  : int64_t n, unsigned width
 * outputs : Whether the given number can be stored in the given width
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
	assert(width <= 64);

	if (width < 1) {
    	return false;
    }
    if (n == 0) {
     	return true;
	}

    if (n < 0) {
        n  = ~n;
	}

    return Bitpack_fitsu(n, width - 1);
}

/* 
 * name    : Bitpack_getu
 * purpose : Function that extracts the a width-long value at lsb
 * inputs  : uint64_t word, unsigned width, unsigned lsb
 * outputs : The value at lsb that is width-long from word
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
	assert(width <= MAX_BIT_WIDTH);
	assert(width + lsb <= MAX_BIT_WIDTH);

	uint64_t mask = create_mask(width, lsb);

	word &= mask;
	word = u_right_shift(word, lsb);

	return word;
}

/* 
 * name    : Bitpack_gets
 * purpose : Function that extracts the a width-long value at lsb for signed
             values
 * inputs  : uint64_t word, unsigned width, unsigned lsb
 * outputs : The value at lsb that is width-long from word
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
	assert(width <= MAX_BIT_WIDTH);
	assert(width + lsb <= MAX_BIT_WIDTH);

	uint64_t mask = create_mask(width, lsb);
	
	word &= mask;
	word = left_shift(word, MAX_BIT_WIDTH - (width + lsb));
	word = s_right_shift(word, MAX_BIT_WIDTH - width);

	return (int64_t) word;
}

/* 
 * name    : Bitpack_newu
 * purpose : Function that puts the given value inside the word at lsb
 * inputs  : uint64_t word, unsigned width, unsigned lsb, uint64_t value
 * outputs : The 64-bit word with the given value inserted at lsb
 */
uint64_t Bitpack_newu(uint64_t word, 
					  unsigned width, 
					  unsigned lsb, 
					  uint64_t value)
{
	if (!Bitpack_fitsu(value, width)) {
		RAISE(Bitpack_Overflow);
	}
	assert(width <= MAX_BIT_WIDTH);
	assert(width + lsb <= MAX_BIT_WIDTH);

	return flip(word, value, width, lsb);
}

/* 
 * name    : Bitpack_news
 * purpose : Function that puts the given signed value inside the word at lsb
 * inputs  : uint64_t word, unsigned width, unsigned lsb, int64_t value
 * outputs : The 64-bit word with the given value inserted at lsb
 */
uint64_t Bitpack_news(uint64_t word, 
					  unsigned width, 
					  unsigned lsb,  
					  int64_t value)
{
	if (!Bitpack_fitss(value, width)) {
		RAISE(Bitpack_Overflow);
	}
	assert(width <= MAX_BIT_WIDTH);
	assert(width + lsb <= MAX_BIT_WIDTH);
	
	return flip(word, value, width, lsb);
}
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "assert.h"
#include "bitpack.h"

bool fitsu_test(uint64_t n, unsigned width);
bool fitss_test( int64_t n, unsigned width);
bool getu_test(uint64_t n, unsigned width, unsigned lsb);
bool gets_test(uint64_t n, unsigned width, unsigned lsb);
bool newu_test(uint64_t n, unsigned width, unsigned lsb, uint64_t value);
bool news_test(uint64_t n, unsigned width, unsigned lsb,  int64_t value);


int main(int argc, char *argv[])
{
	(void)argc; (void) argv;
	
	uint64_t a = 7;
	unsigned width = 3;
	fitsu_test(a, width);
	int64_t b = -5;
	width = 3;
	fitss_test(b, width);

	fitsu_test((uint64_t)5, (unsigned)3);
	fitss_test((int64_t)5, (unsigned)3);
	fitss_test((int64_t)0x1d, 5);

	fitss_test(16, 5);
	fitsu_test(0, 1);

	getu_test(0x3f4, 6, 2);
	gets_test(0x3f4, 6, 2);

	newu_test(0x3f4, 6, 2, (uint64_t)52);
	news_test(0x3f4, 6, 2, ( int64_t)20);

	newu_test(0x3f4, 4, 4, a);
	news_test(0x3f4, 6, 4, 0x1d);

	uint64_t test = ~0;
	int64_t test1 = ~0;

	printf("unsigned val of test: %lu\n", sizeof(test));
	printf("signed val: %ld\n", test1);
	//test1 << 3;
	printf("signed val after right shift 1: %ld\n", sizeof(test1));
}

bool fitsu_test(uint64_t n, unsigned width)
{
	if(Bitpack_fitsu(n, width)){
		printf("FITSU: the value of %" PRIu64 " fits in %u width\n", n, width);
		return true;
	} else {
		printf("FITSU: the value of %" PRIu64 " does not fit in %u width\n", n, width);
	}
	return false;
}

bool fitss_test(int64_t n, unsigned width)
{
	if(Bitpack_fitss(n, width)){
		printf("FITSS: the value of %ld fits in %u width\n", n, width);
		return true;
	}
	printf("FITSS: the value of %ld does not fit in %u width\n", n, width);
	return false;
}

bool getu_test(uint64_t n, unsigned width, unsigned lsb)
{
	if(Bitpack_getu(n, width, lsb) == 61){
		printf("GETU: value %ld, width %u, lsb %u extracted successfully!\n", n, width, lsb);
		return true;
	}
	printf("GETU: value %ld, width %u, lsb %u extracted unsuccessfully\n", n, width, lsb);
	return false;
}



bool gets_test(uint64_t n, unsigned width, unsigned lsb)
{
	if(Bitpack_gets(n, width, lsb) == -3){
		printf("GETS: value %ld, width %u, lsb %u extracted successfully!\n", n, width, lsb);
		return true;
	}
	printf("GETU: value %ld, width %u, lsb %u extracted unsuccessfully\n", n, width, lsb);
	return false;
}

bool newu_test(uint64_t n, unsigned width, unsigned lsb, uint64_t value)
{
	assert(width + lsb <= 50);
	unsigned lsb2 = 3;
	unsigned width2 = 2; 

	if(Bitpack_getu(Bitpack_newu(n, width, lsb, value), width, lsb) == value &&
		Bitpack_getu(Bitpack_newu(n, width, lsb, value), width2, lsb2) == Bitpack_getu(n, width2, lsb2)){ // Not sure about this cond
		printf("NEWU: works apparently\n");
		return true;
	}
	printf("NEWU: FUCKING FAILS\n");
	return false;
}



bool news_test(uint64_t n, unsigned width, unsigned lsb,  int64_t value)
{
	assert(width + lsb <= 50);
	unsigned lsb2 = 3;
	unsigned width2 = 2; 
		
	if(Bitpack_gets(Bitpack_news(n, width, lsb, value), width, lsb) == value &&
		Bitpack_gets(Bitpack_news(n, width, lsb, value), width2, lsb2) == Bitpack_gets(n, width2, lsb2)){ // not sure about this cond
		printf("NEWS: works apparently\n");
		return true;
	}
	printf("NEWS: FUCKING FAILS\n");
	return false;
}
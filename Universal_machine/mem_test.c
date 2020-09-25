#include <stdio.h>
#include "seg.h"


void test1();

int main(int argc, char *argv[]){
	(void)argc;(void)argv;
	test1();
}

void test1(){
	Seg_T mem = Seg_new();
	uint32_t id = Seg_map(mem, 4);
	uint32_t id1 = Seg_map(mem, 4);
	uint32_t id2 = Seg_map(mem, 4);
	uint32_t value = 10;
	uint32_t value1 = 11;
	uint32_t value2 = 12220;
	Seg_store(mem, value, id, 0);
	Seg_store(mem, value1, id1, 0);
	Seg_store(mem, value2, id2, 2);
	printf("values loaded\n");
	uint32_t load_value = Seg_load(mem, id, 0);
	uint32_t load_value1 = Seg_load(mem, id1, 0);
	uint32_t load_value2 = Seg_load(mem, id2, 2);

	printf("load value: %u load value1: %u load value2: %u\n", load_value, load_value1, load_value2);	

	printf("About to free\n");
	Seg_free(mem);
}
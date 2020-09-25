/*
 * Name: Jun Sung Tak (jtak) and Nilay Maity (nmaity01)
 * hw6
 */

#include <stdio.h>
#include "um_loader.h"
#include "um_executor.h"
#include <assert.h>
#include <except.h>

int main(int argc, char *argv[])
{
	if(argc < 2 || argc > 2){
		fprintf(stderr, "too few/many arguments\n");
		fprintf(stderr, "Usage: %s [.um filename]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(argv[1], "rb");

	if (fp == NULL){
		fprintf(stderr, "%s %s %s\n", "Could not open file ", argv[1],
			    "for reading.");
		exit(EXIT_FAILURE);
	}
	uint32_t regs[8] = {0};
	Seg_T prgm = program_loader(fp);
	assert(prgm != NULL); 
	fclose(fp);

	um_exec(prgm, regs);
	
	return 0;
} 

/*
 * Name: Jun Sung Tak (jtak) and Nilay Maity (nmaity01)
 * hw6
 */

#include <stdlib.h>
#include "um_loader.h"
#include "bitpack.h"

/*               
 * Parameters: FILE *fp
 *    Returns: int
 *       Does: Calculates number of words, thus instructions in the given file
 */
extern int word_count(FILE *fp)
{
	int temp = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, temp, SEEK_SET);
	return file_size / (sizeof(uint32_t));
}

/*               
 * Parameters: FILE *fp
 *    Returns: Seg_T
 *       Does: Loads the program that is in fp into a Segmented memory
 */
extern Seg_T program_loader(FILE *fp)
{
	Seg_T mem = Seg_new();
	int num_inst = word_count(fp);
	uint32_t loc = Seg_map(mem, num_inst);
	printf("numinst: %u\n", num_inst);
	for(int i = 0; i < num_inst; i++)
	{
		uint32_t inst = 0;
		for(int j = 3; j >= 0; j--)
		{
			uint32_t data = getc(fp);
			inst = Bitpack_newu(inst, 8, 8 * j, data);
		}
		Seg_store(mem, inst, loc, i);
	}
	return mem;
}
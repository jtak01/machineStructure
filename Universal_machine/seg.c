/*
 * Name: Jun Sung Tak (jtak) and Nilay Maity (nmaity01)
 * hw6
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <uarray.h>
#include <seq.h>
#include <stack.h>
#include "seg.h"

#define T Seg_T

struct T {
	Stack_T empty;  //unmapped memory
	Seq_T segments; // main memory
};

/*               
 * Parameters: None             
 *    Returns: Segmented memory 
 *       Does: Initializes a new segment of memory and returns
 */
extern T Seg_new()
{
	T mem = malloc(sizeof(*mem));
	assert(mem != NULL);

	mem -> segments = Seq_new(4);
	mem -> empty = Stack_new();
	assert(mem -> segments != NULL);
	assert(mem -> empty != NULL);

	return mem;
}

/*               
 * Parameters: Seg_T mem             
 *    Returns: Nothing
 *       Does: Frees any memory allocated for the given memory
 */
extern void Seg_free(T mem)
{
	UArray_T to_free;
	int length = Seq_length(mem -> segments);

	for(int i = 0; i < length; i++){
		to_free = Seq_remlo(mem -> segments);
		if(to_free != NULL)
			UArray_free(&to_free);
	}

	Seq_free(&(mem -> segments));
	Stack_free(&(mem -> empty));
	free(mem);
	return;
}

/*               
 * Parameters: Seg_T mem, int size             
 *    Returns: Address/ID of the newly mapped memory 
 *       Does: Allocates a space on the segmented memory
 */
extern uint32_t Seg_map(T mem, int size)
{
	UArray_T segment = UArray_new(size, sizeof(uint32_t));
	assert(segment != NULL);

	uint32_t id = 0;
	if(!Stack_empty(mem -> empty)){
		id = (uint32_t)(uintptr_t)Stack_pop(mem -> empty);
		Seq_put(mem -> segments, id, (void *)segment);
	} else {
		Seq_addhi(mem -> segments, (void *)segment);
		id = (uint32_t)Seq_length(mem -> segments) - 1;
	}
	return id;
}

/*               
 * Parameters: Seg_T mem, unsigned id             
 *    Returns: Nothing
 *       Does: Frees the specified memory from the given segmented memory
 */
extern void Seg_unmap(T mem, unsigned id)
{
	UArray_T to_free = Seq_put(mem -> segments, id, NULL);
	UArray_free(&to_free);
	Stack_push(mem -> empty, (void *)(uintptr_t)id);
}

/*               
 * Parameters: Seg_T mem, uint32_t val, unsigned id, unsigned offset
 *    Returns: None
 *       Does: Stores the given value into the specfied location in  memory
 */
extern void Seg_store(T mem, uint32_t val, unsigned id, unsigned offset)
{
	assert(sizeof(val) <= sizeof(uint32_t));
	UArray_T seg = Seq_get(mem -> segments, id);
	*(uint32_t *)UArray_at(seg, offset) = val;
}

/*               
 * Parameters: Seg_T mem, unsigned offset, unsigned id             
 *    Returns: uint32_t
 *       Does: The value in the given memory at the specfied location
 */
extern uint32_t Seg_load(T mem, unsigned offset, unsigned id)
{
	UArray_T seg = Seq_get(mem -> segments, id);
	uint32_t to_return = *(uint32_t *)UArray_at(seg, offset);
	return to_return;
}

/*               
 * Parameters: Seg_T mem, unsigned id             
 *    Returns: int
 *       Does: Returns the length of the given memory
 */
extern int Seg_length(T mem, unsigned id)
{
	UArray_T seg = Seq_get(mem -> segments, id);
	return UArray_length(seg);
}

/*               
 * Parameters: Seg_T mem, unsigned id             
 *    Returns: None
 *       Does: Duplicate the existing program and frees it 
 */
extern void Seg_load_prgm(T mem, unsigned id)
{
	UArray_T loaded_prgm = UArray_copy(Seq_get(mem -> segments, id), 
                                       UArray_length(Seq_get(mem -> segments, 
                                       id)));
	UArray_T old_prgm = Seq_put(mem -> segments, 0, loaded_prgm);
	UArray_free(&old_prgm);
	
}
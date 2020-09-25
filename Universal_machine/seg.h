/*
 * Name: Jun Sung Tak (jtak) and Nilay Maity (nmaity01)
 * hw6
 */

#ifndef SEG_H_INCLUDED
#define SEG_H_INCLUDED
#include <stdint.h>
#include <seq.h>
#include <stack.h>

#define T Seg_T
typedef struct T *T;

extern T Seg_new();
extern void Seg_free(T mem);
extern uint32_t Seg_map(T mem, int size);
extern void Seg_unmap(T mem, unsigned id);
extern void Seg_store(T mem, uint32_t val, unsigned id, unsigned offset);
extern uint32_t Seg_load(T mem, unsigned offset, unsigned id);
extern void Seg_load_prgm(T mem, unsigned id);
extern int Seg_length(T mem, unsigned id);

#undef T
#endif
/*
        Name: Jun Sung Tak (jtak01) Nilay Maity (nmaity01)
        HW2
*/

#include <uarray.h>
#include <bit.h>
#ifndef BIT2_H_INCLUDED
#define BIT2_H_INCLUDED
#define T Bit2_T
typedef struct T *T;


struct T {
        int width;
        int height; 
        Bit_T arr;
};

/*Declares and returns a new 2D bit array with the given dimensions. Function 
takes width and height of desired bit array with the expectation that none of
the parameters are less than or equal to 0.*/
T Bit2_new(int width, int height);

/*Frees the memory associated with the given bit array. Expects the pointer 
and the bit array itself to be NOT null.*/
void Bit2_free(T * array);

/*Returns a binary value (in integer) of the given column and row. Expects 
the given array to NOT be null. Expects col and row to be in bounds (greater 
than or equal to 0 less than the width and height)*/
int Bit2_get(T array, int col, int row);

/*Given the array and specified row and column, it takes the given bit and 
replaces the old bit at the specified location. Function also returns the 
value of the replaced old bit. Expects the given array to NOT be null. 
Expects col and row to be in bounds (greater than or equal to 0 less than 
the width and height)*/
int Bit2_put(T array, int col, int row, int bit);

/*-Returns the width of the given bit array. Expects the given array to NOT 
be null.*/
int Bit2_width(T array);

/*Returns the height of the given bit array. Expects the given array to NOT 
be null.*/
int Bit2_height(T array);

/*Maps the given function in the given bit array in row major fashion. 
Expects array to NOT be null.*/
void Bit2_map_col_major(T array, void apply(int col, int row, T array,
                                int bit, void *cl), void *cl);

/*-Maps the given function in the given bit array in column major fashion. 
-Expects array to NOT be null.*/
void Bit2_map_row_major(T array, void apply(int col, int row, T array,
                                int bit, void *cl), void *cl);

#undef T
#endif
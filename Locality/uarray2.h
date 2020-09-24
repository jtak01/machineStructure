/*
        Name: Jun Sung Tak (jtak01) Nilay Maity (nmaity01)
        HW2
*/
#include <uarray.h>
#ifndef UARRAY2_H_INCLUDED
#define UARRAY2_H_INCLUDED
#define T UArray2_T
typedef struct T *T;

struct T {
        int width;
        int height;
        int size;
        UArray_T arr;
};

/*Declares and returns a new 2D unboxed array with the given dimensions. 
Size indicates the bit-size of each element in the array.
Function takes width and height of desired array as well as a size for 
an element with that expectation that none of the parameters are less than 
or equal to 0.*/
T UArray2_new(int width, int height, int size);

/*Frees the memory associated with the given array. Expects the pointer and 
the array itself to be NOT null.*/
void UArray2_free(T *array);

/*Returns the void pointer of the element at the specified index of the 
given array. Expects given column and row to be within array bounds.Expects 
array to NOT be null.*/
void *UArray2_at(T array, int col, int row);

/*Returns the width of the given array. Expects array to NOT be null.*/
int UArray2_width(T array);

/*Returns the height of the given array. Expects array to NOT be null.*/
int UArray2_height(T array);

/*Returns the bit size of the elements stored in the given array. Expects 
array to NOT be null.*/
int UArray2_size(T array); 

/*Maps the given function in the given array in row major fashion. Expects 
array to NOT be null.*/
void UArray2_map_row_major(T array, void apply (int col, int row, 
                             T array, void *element, void *cl), void * cl);

/*Maps the given function in the given array in column major fashion. Expects 
array to NOT be null.*/
void UArray2_map_col_major(T array, void apply (int col, int row,
                             T array, void *element, void *cl), void * cl);

#undef T
#endif
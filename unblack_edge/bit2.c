/*
    Name: Jun Sung Tak (jtak01) Nilay Maity (nmaity01)
    HW2
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "assert.h"
#include "bit.h"
#include "bit2.h"
#define T Bit2_T

/*  Parameters: int width, int height
    Return: Bit2_T
    Purpose: Initializes a 2D bit array
*/  
T Bit2_new(int width, int height)
{
    assert(width > 0 && height > 0);
    int length = width * height;
    Bit2_T array2 = malloc(sizeof(*array2));
    assert(array2 != NULL);
    Bit_T array1 = Bit_new(length);
    array2->width = width;
    array2->height = height;
    array2->arr = array1;
    return array2;
}
/*  Parameters: T(UArray2_T)* array
    Return: void
    Purpose: Frees memory associated with Bit2 array
*/
void Bit2_free(T *array)
{
    assert(array != NULL && *array != NULL);
    Bit_free(&((*array)->arr));
    free(*array);       
}

/*  Parameters: T(Uarray2_T) array, int col, int row
    Return: int
    Purpose: Returns the value of the bit associated with the given column
             and row in the Bit2 array
*/      
int Bit2_get(T array, int col, int row)
{
    assert(array != NULL);
    assert(col >= 0 && col < (array->width));
    assert(row >= 0 && row < (array->height));
    int index = ((row * array->width) + col);
    int curr_bit = Bit_get(array->arr, index);
    return curr_bit;
}

/*  Parameters: T(Uarray2_T) array, int col, int row, int bit
    Return: int(value of old bit)
    Purpose: Puts the given bit into the given 2D bit array in the spot
             associated with the given column and row. Returns the old bit
             that was in the location
*/   
int Bit2_put(T array, int col, int row, int bit)
{
    assert(array != NULL);    
    assert(col >= 0 && col < (array->width));
    assert(row >= 0 && row < (array->height));
    int index = ((row * array->width) + col);
    int old_bit = Bit_get(array->arr, index);
    Bit_put(array->arr, index, bit);
    return old_bit;
}

/*  Parameters: T(Bit2_T) array
    Return: int
    Purpose: Returns the width of the given 2D Bit array
*/    
int Bit2_width(T array)
{
    assert(array != NULL);
    return array->width;
}

/*  Parameters: T(Bit2_T) array
    Return: int
    Purpose: Returns the height of the given 2D Bit array
*/ 
int Bit2_height(T array)
{
    assert(array != NULL);
    return array->height;
}

/*  Parameters: T(Bit2_T) array, function pointer, void *cl
    Return: void
    Purpose: Maps the given function pointer to every element in the given
             2D Bit array in row major fashion
*/   
void Bit2_map_row_major(T array, void apply (int col, int row, 
                             T array, int bit, void *cl), void * cl)
{
    assert(array != NULL);
    int width = array->width;
    int height = array->height;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                    apply(j, i, array, Bit2_get(array, j, i), cl);
            }
        }
}

/*  Parameters: T(Bit2_T) array, function pointer, void *cl
    Return: void
    Purpose: Maps the given function pointer to every element in the given
             2D Bit array in column major fashion
*/   
void Bit2_map_col_major(T array, void apply (int col, int row,
                             T array, int bit, void *cl), void * cl)
{
    assert(array != NULL);
    int width = array->width;
    int height = array->height;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            apply(i, j, array, Bit2_get(array, i, j), cl);
        }
    }
}

#undef T
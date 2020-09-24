/*
    Name: Jun Sung Tak (jtak01) Nilay Maity (nmaity01)
    HW2
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "assert.h"
#include "uarray.h"
#include "uarray2.h" 
#define T UArray2_T

// Parameters: int width, int height, int size
// Return: UArray2_T
// Purpose: Initializes a new 2D Uarray
T UArray2_new(int width, int height, int size)
{
    assert(width != 0 && height != 0 && size != 0);
    
    T arr2 = malloc(sizeof(*arr2));
    assert(arr2 != NULL);

    int length = width * height;
    UArray_T arr1 = UArray_new(length, size);

    arr2->size = size;
    arr2->arr = arr1;
    arr2->height = height;
    arr2->width = width;

    return arr2;
}

// Parameters: T(UArray2_T)* array
// Return: void
// Purpose: Frees memory associated with UArray2 array
void UArray2_free(T *array)
{
    assert(array != NULL && *array != NULL);
    
    UArray_free(&((*array)->arr));
    free(*array);       
}

/*  Parameters: T(Uarray2_T) array, int col, int row
    Return: void pointer
    Purpose: Returns the void pointer associated with given column and row
*/          
void *UArray2_at(T array, int col, int row)
{
    assert(array != NULL);
    assert(col >= 0 && col < (array->width));
    assert(row >= 0 && row < (array->height));

    int index = row * array->width + col;
    void *temp = UArray_at(array->arr, index);
    return temp;
}

/*  Parameters: T(Uarray2_T) array
    Return: int
    Purpose: Returns the width of the given 2D UArray
*/    
int UArray2_width (T array)
{
    assert(array != NULL);
    return array->width;
}

/*  Parameters: T(Uarray2_T) array
    Return: int
    Purpose: Returns the height of the given 2D UArray
*/    
int UArray2_height (T array)
{
    assert(array != NULL);
    return array->height;
}

/*  Parameters: T(Uarray2_T) array
    Return: int
    Purpose: Returns the size of the given 2D UArray
*/    
int UArray2_size (T array)
{
    assert(array != NULL);
    return array->size;
}

/*  Parameters: T(Uarray2_T) array, function pointer, void *cl
    Return: void
    Purpose: Maps the given function pointer to every element in the given
             2D UArray in row major fashion
*/    
void UArray2_map_row_major (T array, void apply (int col, int row, 
                            T array, void *elem, void *cl), void *cl)
{
    assert(array != NULL);

    for (int i = 0; i < array->height; i++){
        for (int j = 0; j < array->width; j++){
            apply(j, i, array, UArray2_at(array, j, i), cl);
        }
    }
}

/*  Parameters: T(Uarray2_T) array, function pointer, void *cl
    Return: void
    Purpose: Maps the given function pointer to every element in the given
             2D UArray in column major fashion
*/    
void UArray2_map_col_major (T array, void apply (int col, int row,
                            T array, void *elem, void *cl), void *cl)
{
    assert(array != NULL);
        
    for (int i = 0; i < array->width; i++){
        for (int j = 0; j < array->height; j++){
            apply(i, j, array, UArray2_at(array, i, j), cl);
        }
    }
}

#undef T
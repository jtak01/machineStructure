/* 
 * a2plain.c
 * Jun Sun Tak, Zoe Hsieh, 2/20/20
 * hw3 : locality
 * 
 */

#include <string.h>

#include <a2plain.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/


/* 
 * name    : new
 * purpose : creates new UArray2
 * inputs  : int width, int height, int size
 * outputs : A2Methods UArray2 array
 */
static A2Methods_UArray2 new(int width, int height, int size)
{
  return UArray2_new(width, height, size);
}


/* 
 * name    : new_with_blocksize
 * purpose : creates new UArray 2 with default blocksize
 * inputs  : int width, int height, int size, int blocksize
 * outputs : A2Methods UArray2 array
 */
static A2Methods_UArray2 new_with_blocksize(int width, int height, 
                                            int size, int blocksize)
{
  (void) blocksize;
  return UArray2_new(width, height, size);
}


/* 
 * name    : a2free
 * purpose : frees the UArray2
 * inputs  : A2Methods_UArray2 *arr
 * outputs : none
 */
static void a2free(A2Methods_UArray2 *arr)
{
  UArray2_free((UArray2_T *)arr);
}


/* 
 * name    : width
 * purpose : gets the width of the array
 * inputs  : A2Methods_UArray2 arr
 * outputs : int width
 */
static int width(A2Methods_UArray2 arr)
{
  return UArray2_width(arr);
}


/* 
 * name    : height
 * purpose : gets height of the array
 * inputs  : A2Methods_UArray2 arr
 * outputs : in height
 */
static int height(A2Methods_UArray2 arr)
{
  return UArray2_height(arr);
}


/* 
 * name    : size
 * purpose : gets size of the array
 * inputs  : A2Methods_UArray2 arr
 * outputs : int size
 */
static int size(A2Methods_UArray2 arr)
{
  return UArray2_size(arr);
}


/* 
 * name    : blocksize
 * purpose : returns blocksize
 * inputs  : A2Methods_UArray2 arr
 * outputs : int blocksize
 */
static int blocksize(A2Methods_UArray2 arr)
{
  (void)arr;
  return 1;
}


/* 
 * name    : at
 * purpose : gets the element at the given position
 * inputs  : A2Methods_UArray2 arr, int i, int j
 * outputs : pointer to element
 */
static A2Methods_Object *at(A2Methods_UArray2 arr, int i, int j)
{
  return UArray2_at(arr, i, j);
}


typedef void applyfun(int i, int j, UArray2_T array2b, void *elem, void *cl);


/* 
 * name    : map_row_major
 * purpose : calls map function that iterates through rows
 * inputs  : A2Methods_UArray2 uarray2, A2Methods_applyfun apply,
 *           void *cl
 * outputs : none
 */
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
  UArray2_map_row_major(uarray2, (applyfun*)apply, cl);
}


/* 
 * name    : map_col_major
 * purpose : calls map function that iterates through columns
 * inputs  : A2Methods_UArray2 uarray2, A2Methods_applyfun apply,
 *           void *cl
 * outputs : none
 */
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
  UArray2_map_col_major(uarray2, (applyfun*)apply, cl);
}


struct small_closure {
  A2Methods_smallapplyfun *apply; 
  void                    *cl;
};


/* 
 * name    : apply_small
 * purpose : apply function for map functions
 * inputs  : int i, int j, UArray2_T uarray2, void *elem, void *vcl
 * outputs : none
 */
static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
  struct small_closure *cl = vcl;
  (void)i;
  (void)j;
  (void)uarray2;
  cl->apply(elem, cl->cl);
}


/* 
 * name    : small_map_row_major
 * purpose : modified version of map_row_major function
 * inputs  : A2Methods_UArray2 a2, A2Methods_applyfun apply,
 *           void *cl
 * outputs : none
 */
static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
  struct small_closure mycl = { apply, cl };
  UArray2_map_row_major(a2, apply_small, &mycl);
}


/* 
 * name    : small_map_col_major
 * purpose : modified version of map_col_major function
 * inputs  : A2Methods_UArray2 a2, A2Methods_applyfun apply,
 *           void *cl
 * outputs : none
 */
static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
  struct small_closure mycl = { apply, cl };
  UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
  new,
  new_with_blocksize,
  a2free,
  width, 
  height, 
  size,
  blocksize,
  at,
  map_row_major,
  map_col_major,
  NULL,
  map_row_major,
  small_map_row_major,
  small_map_col_major,
  NULL,
  small_map_row_major,
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;

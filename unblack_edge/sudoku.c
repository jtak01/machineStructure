/*
        Name: Jun Sung Tak (jtak01) Nilay Maity (nmaity01)
        HW2
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "uarray.h"
#include "uarray2.h"
#include "pnmrdr.h"
#include "assert.h"

/*Process inputs and determines whether to read from standard in or
  a passed in file*/
FILE *process_input(int, char *argv[]);

/*General error function. Exits with exit(1)*/
void error(char *);

/*Reads in sudoku in pgm format with Pnmrdr*/
void read_in_sol(Pnmrdr_T, UArray2_T);

/*Initializes a UArray2 with the contents of the sudoku pgm*/
void initialize_arr(int, int, UArray2_T, void*, void*);

/*Main function that checks the sudoku to make sure that rows, columns, and
  boxes all contain unique 1's~9's*/
bool check_sudoku(UArray2_T);
/*Resets an UArray_T array (used for checking for duplicates)*/
void reset_aux(UArray_T);

/*Feeds in coordinates of a 3x3 box to its helper function*/
bool check_box(UArray_T, UArray2_T);

/*Helper function for check_box. With the 3x3 box coordinate, it checks
  for duplicate numbers in each 3x3 box*/
bool check_box_helper(UArray_T, UArray2_T, int, int);

/*Before checking for duplicates, this functions makes sure that all values
  in the array are valid sudoku values(1~9)*/
bool validate_sudoku(UArray2_T, Pnmrdr_T);

/*Helper function for validating sudoku*/
void check_sol_arr(int, int, UArray2_T, void*, void*);

/*Checks row for duplicates. Exits if it finds a duplicate(exit(1))*/
void row_check(int col, int row, UArray2_T arr, void *element, void *cl);

/*Checks column for duplicates. Exits if it finds a duplicate(exit(1))*/
void col_check(int col, int row, UArray2_T arr, void *element, void *cl);

const int SUDOKU_DIM = 9;

int main(int argc, char *argv[])
{
        FILE *fp = process_input(argc, argv);
        Pnmrdr_T rdr;
        TRY                                  
        rdr = Pnmrdr_new(fp); 
    EXCEPT(Pnmrdr_Badformat)                               
        fclose(fp);
        error("Bad file format (.pgm required)");                  
    END_TRY;

    UArray2_T sol_arr = UArray2_new(SUDOKU_DIM, SUDOKU_DIM, sizeof(int));
        assert(sol_arr != NULL);
        read_in_sol(rdr, sol_arr);
        fclose(fp);

        bool is_valid_sudoku = validate_sudoku(sol_arr, rdr);
        if(!is_valid_sudoku){
                UArray2_free(&sol_arr);
                Pnmrdr_free(&rdr);
                exit(1);
        }
        Pnmrdr_free(&rdr);
    bool is_sol = check_sudoku(sol_arr);

    UArray2_free(&sol_arr);
    if(!is_sol)
        exit(1);
    else
        exit(0);
}

/*
        Parameters: UArray2_T sol_arr
        Return: bool
        Purpose: Determines whether the given sol_arr is a correctly 
                         solved sudoku
*/
bool check_sudoku(UArray2_T sol_arr)
{
        assert(sol_arr != NULL);
        UArray_T aux = UArray_new(SUDOKU_DIM, sizeof(int));
        assert(aux != NULL);
        reset_aux(aux); //Set entire array to 0's
        UArray2_map_row_major(sol_arr, col_check, &aux); 
        
        UArray2_map_col_major(sol_arr, row_check, &aux);
                                                                             
        
        bool box_checker = check_box(aux, sol_arr); 
        UArray_free(&aux);
        if(!box_checker){
                return false;
        }
        return true;
}

/*
        Parameters: UArray_T aux, UArray2_T sol_arr
        Returns: bool
        Purpose: Checks the given array for 9 valid sudoku boxes 
                 (each box is a 3x3 portion of UArray2)
*/
bool check_box(UArray_T aux, UArray2_T sol_arr)
{
        assert(aux != NULL && sol_arr != NULL);
        reset_aux(aux); 
        for(int i = 0; i < 3; i++)
        {
                for(int j = 0; j < 3; j++)
                {
                        if(!(check_box_helper(aux, sol_arr, i * 3, j * 3))){
                                return false;
                        } else {
                                reset_aux(aux);
                        }
                }
        }
        return true;
}

/*
        Parameters: UArray_T aux, UArray2_T sol_arr, int i, int j
        Returns: bool
        Purpose: Helper function for the check_box function. Given the 
                     coordinates to 3x3 boxes it checks for any duplicates 
                     within the box
*/
bool check_box_helper(UArray_T aux, UArray2_T sol_arr, int i, int j)
{
        assert(i >= 0 && i < 7 && j >= 0 && j < 7);
        int start_row = i;
        int start_col = j;
        int end_row = i + 3;
        int end_col = j + 3;
        
        for(int x = start_row; x < end_row; x++)
        {
                for(int y = start_col; y < end_col; y++)
                {
                        if(*(int *)UArray_at(aux, 
                           *(int *)UArray2_at(sol_arr, y, x) - 1) != 0){
                                return false;
                        } else {
                                *(int *)UArray_at(aux, 
                                *(int *)UArray2_at(sol_arr, y, x) - 1) = 1;
                        } 
                }
        }
        return true;
}

/*
        Parameters: int col, int row, UArray2_T arr, void *element, void *cl
        Returns: void
        Purpose: Function that is mapped to the given UArray2. Checks row by 
                 row for duplicates. If there is a duplicate the program 
                 is terminated from this function with exit(1)
*/
void row_check(int col, int row, UArray2_T arr, void *element, void *cl) {
        (void) element;
        if (row == 0) {
                reset_aux(*(UArray_T *) cl);
        }
        if(*(int *)UArray_at(*(UArray_T *) cl,
           *(int *)UArray2_at(arr, row, col) - 1) != 0) {
                UArray2_free(&arr);
                UArray_free((UArray_T*)cl);
                exit(1);
        } else {
                *(int *)UArray_at(*(UArray_T *) cl,
                *(int *)UArray2_at(arr, row, col) - 1) = 1;
        }       
}

/*
        Parameters: int col, int row, UArray2_T arr, void *element, void *cl
        Returns: void
        Purpose: Function that is mapped to the given UArray2. Checks column
                 by column for duplicates. If there is a duplicate the 
                 program is terminated from this function with exit(1)
*/
void col_check(int col, int row, UArray2_T arr, void *element, void *cl) {
        (void) element;
        if (col == 0) {
                reset_aux(*(UArray_T *) cl);
        }
        if(*(int *)UArray_at(*(UArray_T *) cl,
           *(int *)UArray2_at(arr, row, col) - 1) != 0) {
                UArray2_free(&arr);
                UArray_free((UArray_T*)cl);
            exit(1);
        } else {
                *(int *)UArray_at(*(UArray_T *) cl,
                *(int *)UArray2_at(arr, row, col) - 1) = 1;
        }       
}

/*
        Parameters: char *error_message
        Returns: void
        Purpose: Generic error function that prints the error message. 
                 Terminates with exit(1)
*/
void error(char *error_message)
{
        fprintf(stderr, "Error: %s\n", error_message);
        exit(1);
}

/*
        Parameters: UArray_T aux
        Returns: void
        Purpose: Resets the given UArray so that all the elements are 0's
*/
void reset_aux(UArray_T aux)
{
        for(int i = 0; i < 9; i++){
                *(int *)UArray_at(aux, i) = 0;
        }
}

/*
        Parameters: Pnmrdr_T rdr, UArray2_T sol_arr
        Returns: void
        Purpose: Given a Pnmrdr reader it calls initialize_arr to put the pgm
                 values in the sol_arr
*/
void read_in_sol(Pnmrdr_T rdr, UArray2_T sol_arr)
{
        Pnmrdr_mapdata data = Pnmrdr_data(rdr); 
        assert(data.type == Pnmrdr_gray);
        UArray2_map_row_major(sol_arr, initialize_arr, rdr);
}

/*
        Parameters: int col, int row, UArray2_T sol_arr, void *elem, void *cl
        Returns: void
        Purpose: Puts each pgm values in the sol_arr
*/
void initialize_arr(int col, int row, UArray2_T sol_arr, void *elem, void *cl)
{
        (void) col;
        (void) row;
        (void) sol_arr;
        assert(elem != NULL && cl != NULL);
        assert(sizeof(*(int*)elem) == UArray2_size(sol_arr));
        int pgm_val = Pnmrdr_get(cl);
        *(int *)elem = pgm_val;
} 

/*
        Parameters: int argc, char *argv[]
        Returns: FILE *
        Purpose: Given argc and argv this function determines which input the
                         the program(stdin or argv[1]) should read from
*/
FILE *process_input(int argc, char *argv[])
{
        FILE *f = NULL;
    if (argc > 2) {
        error("Too many arguments");
    } else if (argc < 2) {
        f = stdin;
        if (f == NULL) { //FIX: when there is no stdin
            error("No file provided");
        }
    } else {
        f = fopen(argv[1], "rb");
        if (!f) {
                error("Couldn't open file");
        } 
    }
    return f;
}

/*
        Parameters: UArray2_T sol_arr, Pnmrdr_T rdr
        Returns: bool
        Purpose: Goes through the initialized sol_arr and checks that 
                 intensity, width, height, are valid values
*/
bool validate_sudoku(UArray2_T sol_arr, Pnmrdr_T rdr)
{
        Pnmrdr_mapdata data = Pnmrdr_data(rdr);
        int intensity = data.denominator;
        int width = data.width;
        int height = data.height;
        if(intensity != SUDOKU_DIM || width != SUDOKU_DIM || 
           height != SUDOKU_DIM){
                return false;
        }
        bool valid_val = true;
        UArray2_map_row_major(sol_arr, check_sol_arr, &valid_val);
        if(!valid_val){
                return false;
        }
        return true;
}

/*
        Parameters: int col, int row, UArray2_T sol_arr, void *elem, void *cl
        Returns: void
        Purpose: Function goes through the sol_arr and makes sure each value
                 is within 1~9
*/
void check_sol_arr(int col, int row, UArray2_T sol_arr, void *elem, void *cl)
{
        (void)elem;
        if(*(int *)UArray2_at(sol_arr, col, row) < 1 || 
           *(int *)UArray2_at(sol_arr, col, row) > 9){
                *((bool *)cl) &= false;
        }
}
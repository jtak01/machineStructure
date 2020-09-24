/*
    Name: Jun Sung Tak (jtak01) Nilay Maity (nmaity01)
    HW2
*/ 

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <seq.h>
#include "bit2.h"
#include "except.h"
#include "assert.h"
#include "pnmrdr.h"

typedef struct Coordinate T;
struct Coordinate {
        int row;
        int col;
};

const int MAX_LINE_LENGTH = 70;

/*Reads pbm image*/
void read_img(Bit2_T*, FILE*);

/*From the pbm image, bit array is filled with its data*/
void populate_arr(int, int, Bit2_T, int, void*);

/*Finds the bits that corresponds to edges in the pbm image*/
void find_edges(int, int, Bit2_T, int, void *);

/*From bits that corresponds to edges, looks for any neighboring 
  black pixels and change those to white*/
void find_neighbors(int, int, Bit2_T);

/*Starts printing onto standard output. Prints P1 for plain pbm 
  format and the header (dimensions of the image) and calls 
  print_char to print the contents of bit array*/
void pbmwrite(Bit2_T);

/*Prints the contents of the bit array in plain pbm format. No
  line exceeds 70 characters in length*/
void print_char(int, int, Bit2_T, int, void*);


int main(int argc, char *argv[]){
    FILE *fp;
    Bit2_T bit_arr = NULL;

    if(argc > 2){
        fprintf(stderr, "Error: too many inputs\n");
        exit(EXIT_FAILURE);
    } else if(argc == 1) {
        fp = stdin;
    } else {
        fp = fopen(argv[1], "rb");
    }
    assert(fp != NULL);

    read_img(&bit_arr, fp); // Array filled
    // Main functionality here

    Bit2_map_row_major(bit_arr, find_edges, NULL);
    pbmwrite(bit_arr);

    Bit2_free(&bit_arr);
}

/*
    Parameters: int col, int row, Bit2_T arr, int bit, void *cl
    Return: void
    Purpose: Goes through the Bit2_T array and identifies the bits that
             corresponds to the edges of the given pbm file
*/
void find_edges(int col, int row, Bit2_T arr, int bit, void *cl)
{
    (void) cl;
    int width = Bit2_width(arr);
    int height = Bit2_height(arr);

    if ((bit == 1) && (row == 0 || row == (height - 1) || col == 0 || 
        col == (width - 1))) {
        find_neighbors(row, col, arr);
    }
}

/*
    Parameters: int row, int col, Bit2_T arr
    Return: void
    Purpose: Looks for pixels in Bit2_T arr that are next to the given pixel
             which corresponds to the row, col in the arr. If the identified 
             pixel is black (1 in value), it changes it to a white pixel
             (0 in value)
*/
void find_neighbors(int row, int col, Bit2_T arr)
{
    int curr_col = col;
    int curr_row = row;
    int height = Bit2_height(arr);
    int width = Bit2_width(arr);
    Seq_T seq = Seq_new(1000000);
    assert(seq != NULL);
    while (1) {
        T *coord_data = malloc(sizeof(T)); 
        assert(coord_data != NULL);
        bool cont_bool = false;
        coord_data -> row = curr_row;
        coord_data -> col = curr_col;
        Bit2_put(arr, curr_col, curr_row, 0);    
        if ((curr_row + 1) < height && 
            (Bit2_get(arr, curr_col, curr_row + 1) == 1)) {
            cont_bool = true;
            curr_row++;
            Seq_addlo(seq, coord_data);
        } else if ((curr_row - 1) >= 0 && 
                  (Bit2_get(arr, curr_col, curr_row - 1) == 1)) {
            cont_bool = true;
            curr_row--;
            Seq_addlo(seq, coord_data);
        } else if ((curr_col + 1) < width && 
                  (Bit2_get(arr, curr_col + 1, curr_row) == 1)) {
            cont_bool = true;
            curr_col++;
            Seq_addlo(seq, coord_data);
        } else if ((curr_col - 1) >= 0 && 
                  (Bit2_get(arr, curr_col - 1, curr_row) == 1)) {
            cont_bool = true;
            curr_col--;
            Seq_addlo(seq, coord_data);
        } else if (curr_row == row && curr_col == col) {
            free(coord_data);
            break;
        }
        if (cont_bool){
            continue;
        }
        free(coord_data);
        coord_data = (T*)Seq_remlo(seq);
        assert(coord_data != NULL && &coord_data != NULL);
        curr_row = coord_data -> row;
        curr_col = coord_data -> col;
        free(coord_data);        
    }
    Seq_free(&seq);
}

/*
    Parameters: Bit2_T *bit_arr, FILE *fp
    Return: void
    Purpose: Given the pointer to the bit array, the function uses *fp (file
             pointer) to read in the bit values to the bit_arr 
*/
void read_img(Bit2_T *bit_arr, FILE *fp)
{
    assert(bit_arr != NULL); // fp checked in main
    int height = 0;
    int width = 0;

    Pnmrdr_T rdr = Pnmrdr_new(fp);
    assert(rdr != NULL);
    Pnmrdr_mapdata data = Pnmrdr_data(rdr);
    assert(data.type == Pnmrdr_bit);

    height = data.height;
    width = data.width;
    assert(height > 0 && width > 0);

    *bit_arr = Bit2_new(width, height);
    assert(bit_arr != NULL);

    Bit2_map_row_major(*bit_arr, populate_arr, rdr);

    Pnmrdr_free(&rdr);
    fclose(fp);
}

/*
    Parameters: int col, int row, Bit2_T arr, int bit, void *cl
    Return: void
    Purpose: Reads in bit value of the pbm image then populates the bit array
             with it
*/
void populate_arr(int col, int row, Bit2_T arr, int bit, void *cl)
{
    assert(arr != NULL);
    (void)bit;
    int pixel = Pnmrdr_get(cl);
    Bit2_put(arr, col, row, pixel);
}

/*
    Parameters: Bit2_T arr
    Return: void
    Purpose: Given the Bit2_T arr, this function writes its content out to 
             standard output (stdout)
*/
void pbmwrite(Bit2_T arr)
{
    int width = Bit2_width(arr);
    int height = Bit2_height(arr);
    
    printf("P1\n");
    printf("%u %u\n", width, height);
    Bit2_map_row_major(arr, print_char, NULL);
}

/*
    Parameters: int col, int row, Bit2_T arr, int bit, void *cl
    Return: void
    Purpose: This function is mapped for every element of Bit2_T arr so that
             its contents are printed out to standard output (stdout)
*/
void print_char(int col, int row, Bit2_T arr, int bit, void *cl)
{
    (void) cl;
    (void) row;
    
    int width = Bit2_width(arr);
    assert(fprintf(stdout, "%d", bit)); /*makes sure that only a singular 
                                          character is printed onto stdout*/
    if (col == width - 1 || (col + 1) % (MAX_LINE_LENGTH / 2) == 0) {
        /*MAX_LINE_LENGTH is divided by 2 to account for spaces*/
        printf("\n");
        return;
    }
    printf(" ");
    return;
}
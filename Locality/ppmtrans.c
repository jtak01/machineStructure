/* 
 * ppmtrans.c
 * Jun Sun Tak, Zoe Hsieh, 2/20/20
 * hw3 : locality
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"



#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (0)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

FILE *open_file(int, char *argv[], int);
Pnm_ppm transform(int, A2Methods_T, Pnm_ppm, A2Methods_mapfun *, char*);
Pnm_ppm change_orientation(int rotation, Pnm_ppm pix_arr, 
    A2Methods_UArray2 arr, A2Methods_T methods, A2Methods_mapfun *map, char*);
Pnm_ppm same_orientation(int rotation, Pnm_ppm pix_arr, 
    A2Methods_UArray2 arr, A2Methods_T methods, A2Methods_mapfun *map, char*);

void rotate_90(int i, int j, A2Methods_UArray2 arr, 
    A2Methods_Object *ptr, void *cl);
void rotate_180(int i, int j, A2Methods_UArray2 arr, 
    A2Methods_Object *ptr, void *cl);
void rotate_270(int i, int j, A2Methods_UArray2 arr, 
    A2Methods_Object *ptr, void *cl);
void flip_hori(int i, int j, A2Methods_UArray2 arr, 
    A2Methods_Object *ptr, void *cl);
void flip_vert(int i, int j, A2Methods_UArray2 arr, 
    A2Methods_Object *ptr, void *cl);
void flip_transpose(int i, int j, A2Methods_UArray2 arr, 
    A2Methods_Object *ptr, void *cl);

void print_time_data(double, char *, Pnm_ppm, int, char *);

int main(int argc, char *argv[]) 
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                } else if (strcmp(argv[i], "-flip") == 0) {
                        if(!(i + 1 < argc)) {
                            usage(argv[0]);
                        }
                        if(strcmp(argv[++i], "horizontal") == 0){
                            rotation = 1;
                        } else if(strcmp(argv[i], "vertical") == 0) {
                            rotation = 2;
                        } else {
                            fprintf(stderr, 
                    "Flipping must be horizontal or vertical\n");
                            usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        rotation = 3;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }
        FILE *fp = open_file(argc, argv, i);
        assert(fp);
        Pnm_ppm pix_arr = Pnm_ppmread(fp, methods);
        pix_arr = transform(rotation, methods, pix_arr, map, time_file_name);
        Pnm_ppmwrite(stdout, pix_arr);
        fclose(fp);
        Pnm_ppmfree(&pix_arr);
        exit(EXIT_SUCCESS);
}

/* 
 * name    : transform
 * purpose : calls functions to be ready to be transformed
 * inputs  : int rotation, A2Methods_T methods, Pnm_ppm pix_arr, 
 *           A2Methods_mapfun *map, char *time_file
 * outputs : Pnm_ppm pix_arr
 */
Pnm_ppm transform(int rotation, A2Methods_T methods, Pnm_ppm pix_arr, 
                  A2Methods_mapfun *map, char *time_file)
{
    A2Methods_UArray2 arr = pix_arr -> pixels;
    if(rotation == 0){
        return pix_arr;
    } else if(rotation == 90 || rotation == 270) {
        return change_orientation(rotation, pix_arr, arr, methods, map, 
            time_file);
    } else if(rotation == 180 || rotation == 1 || rotation == 2 || 
              rotation == 3) {
        return same_orientation(rotation, pix_arr, arr, methods, map, 
            time_file);
    }
    return pix_arr;
}

/* 
 * name    : same_orientation
 * purpose : calls the specific transformation, and starts the map and timer
 * inputs  : int rotation, Pnm_ppm pix_arr, 
             A2Methods_UArray2 arr, A2Methods_T methods, 
             A2Methods_mapfun *map, char *time_file
 * outputs : Pnm_ppm pix_arr
 */
Pnm_ppm same_orientation(int rotation, Pnm_ppm pix_arr, 
                             A2Methods_UArray2 arr, A2Methods_T methods, 
                             A2Methods_mapfun *map, char *time_file)
{
    CPUTime_T timer;
    int width = methods -> width(arr);
    int height = methods -> height(arr);
    int size = methods -> size(arr);
    int blocksize = methods -> blocksize(arr);

    pix_arr -> pixels = methods -> new_with_blocksize(width, height, size, 
                                                      blocksize);
    timer = CPUTime_New();
    if(rotation == 180){
        CPUTime_Start(timer);
        map(arr, rotate_180, pix_arr);
    } else if(rotation == 1) {
        CPUTime_Start(timer);
        map(arr, flip_hori, pix_arr);
    } else if(rotation == 2) {
        CPUTime_Start(timer);
        map(arr, flip_vert, pix_arr);
    } else if(rotation == 3){
        CPUTime_Start(timer);
        map(arr, flip_transpose, pix_arr);
    }
    double time_elapsed = CPUTime_Stop(timer);
    char *operation;
    if(rotation == 180){
        operation = "rotated";
    } else if(rotation == 3){
        operation = "transposed";
    } else {
        operation = "flipped";
    }

    if(time_file != NULL){
        print_time_data(time_elapsed, time_file, pix_arr, rotation, operation);
    }


    CPUTime_Free(&timer);

    methods -> free(&arr);
    return pix_arr;
}

/* 
 * name    : change_orientation
 * purpose : calls the specific transformation, and starts the map and timer
 * inputs  : int rotation, Pnm_ppm pix_arr, A2Methods_UArray2 arr, 
 *           A2Methods_T methods, A2Methods_mapfun *map, char *time_file
 * outputs : Pnm_ppm pix_arr
 */
Pnm_ppm change_orientation(int rotation, Pnm_ppm pix_arr, 
                 A2Methods_UArray2 arr, A2Methods_T methods,
                 A2Methods_mapfun *map, char *time_file)
{
    int width = methods -> height(arr);
    int height = methods -> width(arr);
    int size = methods -> size(arr);
    int blocksize = methods -> blocksize(arr);
    CPUTime_T timer;
    double time_elapsed;

    pix_arr -> pixels = methods -> new_with_blocksize(width, height, size,
                                                      blocksize);
    timer = CPUTime_New();

    if(rotation == 90){
        CPUTime_Start(timer);
        map(arr, rotate_90, pix_arr);
    } else if (rotation == 270) {
        CPUTime_Start(timer);
        map(arr, rotate_270, pix_arr);
    }
    time_elapsed = CPUTime_Stop(timer);
    int temp = pix_arr -> width;
    pix_arr -> width = pix_arr -> height;
    pix_arr -> height = temp;
    char *operation = "rotated";
    if(time_file != NULL){
        print_time_data(time_elapsed, time_file, pix_arr, rotation, operation);
    }

    methods -> free(&arr);
    CPUTime_Free(&timer);
    return pix_arr;
}

/* 
 * name    : print_time_data
 * purpose : prints time data onto the given time file
 * inputs  : double time_elapsed, char *file_name, Pnm_ppm pix_arr, 
             int rotation, char *operation
 * outputs : none
 */
void print_time_data(double time_elapsed, char *file_name, Pnm_ppm pix_arr, 
                    int rotation, char *operation)
{
    FILE *timing_file = fopen(file_name, "a");
    assert(timing_file != NULL);
    int total_pixel = pix_arr -> width * pix_arr -> height;
    double nanosec_per_pix = time_elapsed / total_pixel;
    fprintf(timing_file, "Manipulating %d pixels\n", total_pixel);
    if(rotation == 1 || rotation == 2){
        char *orientation;
        if(rotation == 2){
            orientation = "vertically";
        } else {
            orientation = "horizontally";
        }
        fprintf(timing_file, "Image was %s %s in %.0f nanoseconds\n"
            ,operation, orientation, time_elapsed);
        fprintf(timing_file, "It took %.0f nanoseconds per pixel\n\n", 
            nanosec_per_pix);
    } else if(rotation == 3){
        fprintf(timing_file, "Image was %s in %.0f nanoseconds\n"
            ,operation, time_elapsed);
        fprintf(timing_file, "It took %.0f nanoseconds per pixel\n\n", 
            nanosec_per_pix);
    } else {
        fprintf(timing_file, "Image was %s by %d degrees in %.0f nanoseconds\n"
            ,operation, rotation, time_elapsed);
        fprintf(timing_file, "It took %.0f nanoseconds per pixel\n\n", 
            nanosec_per_pix);
    }
    fclose(timing_file);
}

/* 
 * name    : rotate_90
 * purpose : rotates the given image 90 degrees clockwise
 * inputs  : int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
 *           void *cl
 * outputs : none
 */
void rotate_90(int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
    void *cl)
{
    (void)arr;
    Pnm_ppm pix_arr = (Pnm_ppm)cl;
    int row = i;
    int col = pix_arr -> height - j - 1;
    *(Pnm_rgb)(pix_arr -> methods -> at(pix_arr -> pixels, col, row)) = 
    *(Pnm_rgb)ptr;
}

/* 
 * name    : rotate_180
 * purpose : rotates the given image 180 degrees 
 * inputs  : int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
 *           void *cl
 * outputs : none
 */
void rotate_180(int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
    void *cl)
{
    (void)arr;
    Pnm_ppm pix_arr = (Pnm_ppm)cl;
    int row = pix_arr -> height - j - 1;
    int col = pix_arr -> width - i - 1;
    *(Pnm_rgb)(pix_arr -> methods -> at(pix_arr -> pixels, col, row)) = 
    *(Pnm_rgb)ptr;
}

/* 
 * name    : rotate_270
 * purpose : rotates the given image 90 degrees counter clockwise 
             (aka 270 degrees)
 * inputs  : int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
 *           void *cl
 * outputs : none
 */
void rotate_270(int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr,
    void *cl)
{   
    (void)arr;
    Pnm_ppm pix_arr = (Pnm_ppm)cl;
    int row = pix_arr -> width - i - 1;
    int col = j;
    *(Pnm_rgb)(pix_arr -> methods -> at(pix_arr -> pixels, col, row)) = 
    *(Pnm_rgb)ptr;
}

/* 
 * name    : flip_hori
 * purpose : flips the image horizontally 
 * inputs  : int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
 *           void *cl
 * outputs : none
 */
void flip_hori(int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
    void *cl)
{
    (void)arr; (void)j;
    Pnm_ppm pix_arr = (Pnm_ppm)cl;
    int col = pix_arr -> width - i - 1;
    *(Pnm_rgb)(pix_arr -> methods -> at(pix_arr -> pixels, col, j)) = 
    *(Pnm_rgb)ptr;
}

/* 
 * name    : flip_vert
 * purpose : flips the image vertically 
 * inputs  : int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
 *           void *cl
 * outputs : none
 */
void flip_vert(int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
    void *cl)
{
    (void)arr; (void)j;
    Pnm_ppm pix_arr = (Pnm_ppm)cl;
    int row = pix_arr -> height - j - 1;
    *(Pnm_rgb)(pix_arr -> methods -> at(pix_arr -> pixels, i, row)) = 
    *(Pnm_rgb)ptr;   
}

/* 
 * name    : flip_transpose
 * purpose : flips the image over the y=x line (transposed image)
 * inputs  : int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr, 
 *           void *cl
 * outputs : none
 */
void flip_transpose(int i, int j, A2Methods_UArray2 arr, A2Methods_Object *ptr,
 void *cl)
{
    (void)arr;
    Pnm_ppm pix_arr = (Pnm_ppm)cl;
    int col = pix_arr -> width - i - 1;
    int row = pix_arr -> height - j - 1;
    *(Pnm_rgb)(pix_arr -> methods -> at(pix_arr -> pixels, col, row)) = 
    *(Pnm_rgb)ptr;
}

/* 
 * name    : open_file
 * purpose : opens file, and returns pointer to the file
 * inputs  : int argc, char *argv[], int i
 * outputs : FILE pointer
 */
FILE *open_file(int argc, char *argv[], int i)
{
    FILE *fp = stdin;
    if(argc > i){
        fp = fopen(argv[i], "r");
        if(fp == NULL){
            fprintf(stderr, "File cannot be opened. Exitting...\n");
            exit(EXIT_FAILURE);
        }
    }
    return fp;

}
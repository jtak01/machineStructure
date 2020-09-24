/*
 * brightness.c
 * by Eric Duanmu, Jun Sung Tak
 * 01/31/2020
 * HW 1
 *
 * Reads in command line arguments or stdin for .pgm files. Prints the average
 * brightness of the file
 */

#include <stdlib.h>
#include <stdio.h>
#include "pnmrdr.h"
#include "except.h"
#include <stdbool.h>

double average_brightness(void *rdr, Pnmrdr_mapdata data);
void error(char *errorMessage);
bool is_valid_data(Pnmrdr_mapdata data, char **err);
FILE *process_input(int argc, char *argv[]);

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

    Pnmrdr_mapdata data = Pnmrdr_data(rdr);
    char *err_msg;
    if (is_valid_data(data, &err_msg) == false) {
        Pnmrdr_free(&rdr);
        fclose(fp);
        error(err_msg);
    }
    printf("Average brightness: %f\n", average_brightness(rdr, data));
    Pnmrdr_free(&rdr);
    fclose(fp);
}

FILE *process_input(int argc, char *argv[]) {
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

bool is_valid_data(Pnmrdr_mapdata data, char **err){
    if (data.type != Pnmrdr_gray) {
        *err = "Bad file format (.pgm required)";
        return false;
    }
    if ((data.width == 0) || (data.height == 0)) {
        *err = "File has 0 pixels";
        return false;
    }
    return true;
}

double average_brightness(void *rdr, Pnmrdr_mapdata data) {  
    double averageBrightness;
    unsigned pixelCount = data.width * data.height;
    unsigned numeratorSum = 0;
    unsigned pixelsCounted = 0;
    TRY
        for (unsigned i = 0; i < pixelCount; i++) {
            pixelsCounted++;
            //fprintf(stderr, "in get\n");
            numeratorSum += Pnmrdr_get(rdr);
            //fprintf(stderr, "after get\n");
        }
        if (pixelsCounted < pixelCount) {
            Pnmrdr_free(rdr);          
            error("Bad pixel count, not all pixels read");
        }
        averageBrightness = (double) (numeratorSum/(data.denominator))/pixelCount;
    EXCEPT(Pnmrdr_Count)
        error("Bad pixel count, pixel read exhausted");
    END_TRY;

    return averageBrightness;
  }

void error(char *errorMessage) {
    fprintf(stderr, "Error: %s\n", errorMessage);
    exit(EXIT_FAILURE);
}

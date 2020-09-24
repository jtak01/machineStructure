#include "readaline.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <seq.h>
#include <stdbool.h>
#include "assert.h"

size_t readaline(FILE *, char **);
size_t sizeofarray(char *);
char *seq_to_string(Seq_T, int);
bool file_to_seq(Seq_T, FILE *);

// Make sure the file is open and does error when running this function
size_t readaline(FILE *inputfd, char **datapp)
{
    if (*datapp == NULL){
    	fprintf(stderr, "File is null\n");
    	return 0;
    }
    Seq_T char_seq = Seq_new(200);
    if (file_to_seq(char_seq, inputfd) == false) {
        Seq_free(&char_seq);
		*datapp = NULL;
		return 0;
    }
    int len = Seq_length(char_seq);    
    char *char_array = seq_to_string(char_seq, len);
    Seq_free(&char_seq);
    *datapp = char_array;
    return len;
}

bool file_to_seq(Seq_T seq, FILE *file) 
{
    int i = 0;
    while (true) {
        char *tempchar = NULL;
	tempchar = malloc(1);
	if(tempchar == NULL){
	    free(tempchar);
	    Seq_free(&seq);
	    fclose(file);
	    fprintf(stderr, "Error allocating memory\n");
	    exit(0);
	}
	*tempchar = fgetc(file);
	if (ferror(file)) {
	    free(tempchar);
	    fclose(file);
	    fprintf(stderr, "Error reading file\n");
	}
	if (*tempchar == EOF) {
	    free(tempchar);
	    if (i == 0) {
	        return false;
	    } else {
	        return true;
	    }
	}
	Seq_addhi(seq, tempchar);
	i++;
	if ((int)*tempchar == 10) {
	    return true;
	}
    }
    return false;
}

size_t sizeofarray(char *arr) 
{
    size_t size = 0;
    for (unsigned i = 0; i < strlen(arr); i++) {
        printf("%c", arr[i]);
	size += sizeof(arr[i]);
    }
    printf("sizeof whole array: %lu\n", sizeof(arr));
    return size;
}

char *seq_to_string(Seq_T seq, int size)
{ 
    char *char_array = NULL;            
    char_array = malloc(size + 1);
    assert(char_array != NULL);
    
    for (int j = 0; j < size; j++) {
        char *copy_char = Seq_get(seq, j);
	char_array[j] = *copy_char;
	free(Seq_get(seq, j));
    }
    char_array[size] = '\0';
    return char_array;
}

#include "readaline.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <seq.h>
#include "assert.h"

size_t readalineTest(FILE *, char **);
void seq_print(Seq_T);
void *seq_to_arr(Seq_T);

int main(int argc, char *argv[]){
	printf("size of a nl char: %ld\n", sizeof('\n'));
	printf("size of a e char: %ld\n", sizeof(char));
	printf("size of a space char: %ld\n", sizeof(' '));
	if (argc == 1)
		return 0;
	else if (argc > 2)
		return 0;

	FILE *file = NULL;
	file = fopen(argv[1], "r");
	if (file == NULL){
		fprintf(stderr, "error opening file\n");
		return 0;
	}
	char *data;

	readalineTest(file, &data);
	//printf("%s", data);

}


size_t readalineTest(FILE *inputfd, char **data){
	assert(inputfd != NULL);
	assert(data != NULL);

	int i = 0;
	Seq_T char_seq = Seq_new(200);

    do {
    	char *temp_str = NULL;
    	temp_str = malloc(1);
    	*temp_str = fgetc(inputfd);

    	Seq_addhi(char_seq, temp_str);
    	i++;
    	if((int)*temp_str == 10) {
    		//printf("new line");

        	break ;
    	}
   	} while(1);


   	printf("count: %u\n", i);
   	seq_print(char_seq);

   	char *final = seq_to_arr(char_seq);
   	
   	printf("Printing from array \n");
   	for (unsigned j = 0; j < 32; j++){
   		printf("%c", final[j]);
   	}
   	printf("\n");
    return(0);
}

void seq_print(Seq_T seq)
{
	(printf("Seq_length = %u\n", Seq_length(seq)));
	printf("Starting to print from seq... \n");
	for (int i = 0; i < Seq_length(seq); i++){
		char *temp = Seq_get(seq, i);
		printf("%s", temp);
	}
}



void *seq_to_arr(Seq_T seq){
	unsigned length = Seq_length(seq) - 2;
	char *arr[length];
	for (unsigned i = 0; i < length; i++){
		arr[i] = Seq_get(seq, i);
	}

	return *arr;
}















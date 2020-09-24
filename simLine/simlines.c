#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <seq.h>
#include <table.h>
#include <atom.h>
#include "readaline.h"

typedef struct lineNode {
    char *filename;
    int linenum;
} lineNode;

char *clean_string(char *);
bool is_correct_char(char); // clean_string helper
char *seq_to_str(Seq_T); // clean_string helper
int next_correct_char_i(char *, int);

void check_inputs(int argc, char *argv[]);
void error(char *);

void read_all_files(int argc, char *argv[], Table_T *);
void read_from_file(FILE *, Table_T **, char *); // read_all_files helper

// to be mapped on table:
void print_outputgroup(const void *key, void **value, void *cl);
void print_lineNode(lineNode); // print_outputgroup helper
void free_table_element(const void *key, void **value, void *cl);

int main(int argc, char *argv[]) 
{
  check_inputs(argc, argv);

  int first_group = 1;
  int *cl = &first_group;

  Table_T table = Table_new(200, NULL, NULL);
  Table_T *table_p = &table;
  
  read_all_files(argc, argv, table_p);

  Table_map(table, print_outputgroup, cl);
  Table_map(table, free_table_element, cl);
  Table_free(table_p);
}

// Arguments: int, char * []
// Returns: none
// Notes: Given argc and argv from main, checks if files are valid to read
///       If any of the inputted files cause errors, exit and print stderr
void check_inputs(int argc, char *argv[]) 
{
    if (argc < 2) {                                                 
        exit(EXIT_SUCCESS);      
    } else {                                    
        bool all_files_valid = true;   
	for (int i = 1; i < argc; i++) {               
	    FILE *fp = NULL;           
	    fp = fopen(argv[i], "r");                                
	    if (fp == NULL) {  
	        all_files_valid = false;                                  
	    } else {
	        fclose(fp);
	    }
	}                                                                   
	if (!all_files_valid) {                                          
	    error("One or more files could not be opened");                   
	}                                                       
    }        
}

// Arguments: int, char * [], Table_T *
// Returns: None
// Notes: Loops through all files and calls read_from_file for each one
void read_all_files(int argc, char *argv[], Table_T *table_p) 
{
    for (int i = 1; i < argc; i++) {
        FILE *fp = NULL;
	fp = fopen(argv[i], "rb");
	read_from_file(fp, &table_p, argv[i]);
	fclose(fp);
    }
}

// Helper function for read_all_files
// Arguments: FILE *, Table_T **, char *
// Returns: None
// Notes: Loops through every line of a given file and inserts them into table
//        key: atom string of a line of text
//        value: sequence of lineNodes to describe the location of each line
//        If the key already exists, add the lineNode to the sequence there
//        If the key doesn't exist, table_put a new key 
void read_from_file(FILE *fp, Table_T **table_p, char *filename) 
{
    int linenum = 1;
    char *data = "";
    Seq_T value_seq;
    
    while (true) {
        int num_chars = readaline(fp, &data);    
	if (num_chars == 0) {
	    free(data);
	    break;
	}
	data = clean_string(data); 
               
	const char *key_atom_string = data;
	const char *key_atom = Atom_string(key_atom_string);
	if (key_atom != Atom_string("\n")) {	

	    lineNode *new_lineNode = NULL;
	    new_lineNode = malloc(sizeof(lineNode));
	    assert(new_lineNode != NULL);
	    (*new_lineNode).filename = filename;
	    (*new_lineNode).linenum = linenum;

	    value_seq = Table_get(**table_p, key_atom);
	    if (value_seq == NULL) {
	        Seq_T new_seq = Seq_new(1);
		Seq_addhi(new_seq, new_lineNode);
		Table_put(**table_p, key_atom, new_seq);               
	    }
	    else {
	        Seq_addhi(value_seq, new_lineNode);
	    }
	}
	free(data);
	linenum++;
    }
    return;
}

// Called by Table_map
// Arguments: const void *, void **, void *
// Returns: none
// Notes: For each element in the table, if the value sequence has more than
//        one element, prints the whole sequence according to the hw spec
void print_outputgroup(const void *key, void **value, void *cl)
{
    const char *key_atom = key;
    Seq_T value_seq = *((Seq_T *)value);
    int length = Seq_length(value_seq);
    if (length < 2) {
        return;
    }
    if (*(int *)cl == 1) {
        printf("%s", key_atom);
	*(int *)cl = 0;
    } else {
        printf("\n%s", key_atom);
    }
    for (int i = 0; i < length; i++) {
        lineNode *curr_lineNode = Seq_get(value_seq, i);
        print_lineNode(*curr_lineNode);
    }
}

// Helper function for print_outputgroup
// Arguments: lineNode
// Returns: none
// Notes: Correctly prints one line for an output group (with padding)
void print_lineNode(lineNode node) 
{
    printf("%-20s %7d\n", node.filename, node.linenum);
}

// Called by Table_map
// Arguments: const void *, void **, void *
// Notes: Deallocates all memory for every element in the table
void free_table_element(const void *key, void **value, void *cl)
{
    (void) key;
    (void) cl;
    Seq_T *value_seq_p = (Seq_T *)value;
    int length = Seq_length(*value_seq_p);
    lineNode *info = NULL;
    
    for (int i = 0; i < length; i++) {
        info = Seq_get(*value_seq_p, i);
	free(info);
    }
    Seq_free(value_seq_p);
    free(*value);
    return;
}

// Arguments: char *
// Returns: char *
// Notes: Converts inputted string to a sequence to remove unnecessary chars
//        (based on hw spec) then calls seq_to_str to deallocate the sequence
//        and returns the cleaned string
char *clean_string(char *str) 
{
    int str_len = (int) strlen(str);
    Seq_T char_seq = Seq_new(200);
    int i = 0;
    while (i < str_len) {
        if (str[i] == '\n') { 
	    char *add_char = NULL;      
	    add_char = malloc(1);                       
	    *add_char = str[i];                                   
	    Seq_addhi(char_seq, add_char);             
	    i++;     
	    break;
	} else if (!is_correct_char(str[i]) && Seq_length(char_seq) == 0) {
	    i++;
	    continue;
	} else if (!is_correct_char(str[i]) && Seq_length(char_seq) > 0) {
	    i = next_correct_char_i(str, i);
	    if (i < 0) {
	        break;
	    }
	    char *add_char = NULL;        
            add_char = malloc(1);                
            *add_char = ' ';           
            Seq_addhi(char_seq, add_char);
	    i++;                      
	} else {
	    char *add_char = NULL;
	    add_char = malloc(1);
	    *add_char = str[i];
	    Seq_addhi(char_seq, add_char);
	    i++;
	}
    }
    char *s = seq_to_str(char_seq);
    free(str);
    return s;
}

// Helper function for clean_string
// Arguments: char
// Returns: bool
// Notes: Checks if inputted char is alphanumeric or ' ' or '\0' or '\n'
//        If the char is none of the above, returns false, else returns true
bool is_correct_char(char c) 
{
    if ((c == '\0') || (c == '\n') || (c == '_')) {
        return true;
    }
    if ((c >= 48 && c <= 57) || isalpha(c)) {
        return true;
    }
    return false;
}

int next_correct_char_i(char *str, int start_i) {
    int i = start_i;
    while (!is_correct_char(str[i + 1])) {
        if (str[i + 1] == '\0' || str[i + 1] == '\n') {
            return -1;
	}
	i++;
    }
    return i;
}

// Helper function for clean_string
// Arguments: Sequence
// Returns: char *
// Notes: Converts a sequence of chars to a string (char *) and returns it
//        Deallocates all memory of the sequence
char *seq_to_str(Seq_T seq)     
{                          
    int length = Seq_length(seq);                                       
    char *char_array = NULL;                       
    char_array = malloc(length + 1);                                   
    assert(char_array != NULL);                                               
    for (int j = 0; j < length; j++) {                                
        char *temp_char = Seq_get(seq, j); 
        char_array[j] = *temp_char;
	free(Seq_get(seq, j));
    }                                                               
    Seq_free(&seq);
    char_array[length] = '\0';         
    return char_array;                                             
}      

void error(char *errorMessage) 
{
    fprintf(stderr, "Error: %s\n", errorMessage);
    exit(EXIT_FAILURE);
}

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdio.h>

void error(char *errorMessage)
{
  fprintf(stderr, "Error: %s\n", errorMessage);
  exit(EXIT_FAILURE);
}

#endif

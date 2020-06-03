#include "constant.h"

void parsingInput(int argc, char **argv, char **filename, char **scheduling_algorithm, char **memory_allocation, int *memory_size, int *quantum);
void readFile(char *filename, int ***input_array, int *input_lines);
void swapArray( int *a, int *b, size_t n );
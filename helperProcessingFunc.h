#include "constant.h"

int count_loadTime(int *process_info, int **page_array, int page_array_size);
int count_pageValue(int **page_array, int array_size, int match_value);
int smallestSpecified_pageIndex(int **page_array, int page_array_size, int process_id, int specifiedIndex);
int largestSpecified_pageIndex(int **page_array, int page_array_size, int process_id, int specifiedIndex);
int smallestJobTime_Index(int **input_array, int array_size, int currentTime);
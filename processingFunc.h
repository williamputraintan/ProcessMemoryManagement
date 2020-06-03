#include "constant.h"

int processing_job( int *process_info, int currentTime, char *memory_allocation, int **page_array, int page_array_size );
void finish_processing(int currentTime, int *process_info, int num_in_queue, int **page_array, int no_pages);
void print_statistic(int **input_array, int array_size, int currentTime);
void print_running_stats(int currentTime, int *process_info, char *memory_allocation, int loadTime, int **page_array, int page_array_size);

void check_and_evict(int **page_array, int page_array_size, int required_page, int currentTime, int eviction_type, int process_id);
void evict_pid(int **page_array, int array_size, int process_id, int currentTime);
int process_scheduler(int currentTime, char * scheduling_algorithm, int quantum, int *process_info);
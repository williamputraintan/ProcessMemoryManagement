/* Constants */
#define DEFAULT_QUANTUM 10      //in seconds
#define SIZE_PER_PAGE 4         //in KB
#define VIRTUAL_MEMORY 16       //in KB
#define LOAD_TIME_PER_PAGE 2    //in seconds
#define TIME_INDEX 0
#define PID_INDEX 1
#define MEM_SIZE_INDEX 2
#define JOB_TIME_INDEX 3
#define FINISH_TIME_INDEX 4
#define REMAINING_JOB_TIME_INDEX 5
#define EMPTY -1
#define EVICTION_TYPE_P 0
#define EVICTION_TYPE_V 1
#define EVICTION_TYPE_CM 2
#define COUNTER_INDEX 2

/* Include libraries */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "helperProcessingFunc.h"

/**
* Calculate amount of loadtime to load based on the process information input.
* The function will ignore if page is already loaded
*/
int count_loadTime(int *process_info, int **page_array, int page_array_size){
    int process_id = process_info[PID_INDEX];
    int memory_size = process_info[MEM_SIZE_INDEX];

    int numPage_needed = ceil((float)memory_size/SIZE_PER_PAGE);
    int numPage_loaded = count_pageValue(page_array, page_array_size, process_id);

    int numPage_toLoad = numPage_needed - numPage_loaded;

    int loadTime = numPage_toLoad * LOAD_TIME_PER_PAGE;

    return loadTime;
}


/**
* THe function will count PID in the page array value based on the matched value int the array
*/
int count_pageValue(int **page_array, int array_size, int match_value){
    int count = 0;

    for (int i=0; i < array_size; i++){
        if (page_array[PID_INDEX][i] == match_value){
            count += 1;
        }
    }
    return count;
}
/**
* The function will return the index lines based on the smallest index column
*/
int smallestSpecified_pageIndex(int **page_array, int page_array_size, int process_id, int specifiedIndex){
    int smallestValue;
    int smallestValue_Index = 0;

    int i = 0;
    //find non empty time
    while ( i < page_array_size){
        if(page_array[specifiedIndex][i] != EMPTY && page_array[PID_INDEX][i] != process_id){
            smallestValue = page_array[specifiedIndex][i];
            smallestValue_Index = i;
            break;
        }
        i += 1;
    }
    //Loop until the end to find the smallest value
    while ( i < page_array_size){
        int specified_val = page_array[specifiedIndex][i];
        if(specified_val != EMPTY && specified_val < smallestValue && page_array[PID_INDEX][i] != process_id){
            smallestValue = specified_val;
            smallestValue_Index = i;
        }
        i += 1;
    }
    return smallestValue_Index;
}
/**
* The function will return the index lines based on the largest index column
*/
int largestSpecified_pageIndex(int **page_array, int page_array_size, int process_id, int specifiedIndex){
    int largestValue;
    int largestValue_Index = 0;

    int i = 0;
    //find non empty time
    while ( i < page_array_size){
        if(page_array[specifiedIndex][i] != EMPTY && page_array[PID_INDEX][i] != process_id){
            largestValue = page_array[specifiedIndex][i];
            largestValue_Index = i;
            break;
        }
        i += 1;
    }
    //Loop until the end to find the smallest value
    while ( i < page_array_size){
        int specified_val = page_array[specifiedIndex][i];
        if(specified_val != EMPTY && specified_val > largestValue && page_array[PID_INDEX][i] != process_id){
            largestValue = specified_val;
            largestValue_Index = i;
        }
        i += 1;
    }
    return largestValue_Index;
}

/**
* The function will return the smallest index lines based on the smallest JobTime_Index
*/
int smallestJobTime_Index(int **input_array, int array_size, int currentTime){
    int lowestJobTime;
    int lowestJobTime_Index = EMPTY;
    int i = 0;
    //find non empty time
    while (input_array[i][TIME_INDEX] <= currentTime){

        if(input_array[i][REMAINING_JOB_TIME_INDEX] > 0){
            lowestJobTime = input_array[i][REMAINING_JOB_TIME_INDEX];
            lowestJobTime_Index = i;
            break;
        }
        i += 1;
                
        if(i >= array_size) return lowestJobTime_Index;
    }
    //Loop to find the smallest job Time
    while (input_array[i][TIME_INDEX] <= currentTime){

        int index_remainingJobTime = input_array[i][REMAINING_JOB_TIME_INDEX];

        if(index_remainingJobTime > 0 && index_remainingJobTime < lowestJobTime){
              
            lowestJobTime = index_remainingJobTime;
            lowestJobTime_Index = i;

        }
        i += 1;

        if(i >= array_size) return lowestJobTime_Index;
    }
       
    return lowestJobTime_Index;
}
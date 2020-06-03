#include "processingFunc.h"
#include "helperProcessingFunc.h"

/**
* The function will process the prosses given to this function.
* It will return the LoadTime needed for the process to run.
*/
int processing_job( int *process_info, int currentTime, char *memory_allocation, int **page_array, int page_array_size ){
    int process_id = process_info[PID_INDEX];

    //Calculate initial loadTIme && page needed remaining
    int loadTime = count_loadTime(process_info, page_array, page_array_size);
    int page_needed_remaining = loadTime / LOAD_TIME_PER_PAGE;  

    //If page eviction required && based on its algorithm chosen
    if(page_needed_remaining>0){

        if( strcmp(memory_allocation, "p") == 0) {
            
            //Will check and evict page when needed
            check_and_evict(page_array, page_array_size, page_needed_remaining, currentTime, EVICTION_TYPE_P, process_id);

            //Assign pageArray with the processIDs
            for (int i = 0; i < page_array_size; i++){

                if(page_array[PID_INDEX][i] == EMPTY){
                    page_array[TIME_INDEX][i] = currentTime;
                    page_array[PID_INDEX][i] = process_id;
                    page_needed_remaining -= 1;
                    if(page_needed_remaining == 0){
                        break;
                    }
                }

            }
            
        }else if( strcmp(memory_allocation, "v") == 0) {
            //Calculate paged 
            int no_emptyPage = count_pageValue(page_array, page_array_size, EMPTY);
            int noPage_virtualMemory = VIRTUAL_MEMORY/SIZE_PER_PAGE;

            int noPage_evict = noPage_virtualMemory - no_emptyPage - count_pageValue(page_array, page_array_size, process_id);

            //Evicting to meet the minimum requirement
            if (noPage_evict > 0){
                check_and_evict(page_array, page_array_size, noPage_evict, currentTime, EVICTION_TYPE_V, process_id);   
            }
            
            for(int i = 0; i < page_array_size; i++){

                if(page_array[PID_INDEX][i] == EMPTY && page_needed_remaining > 0) {
                    
                    //Assign the memory size array with its PID
                    page_array[PID_INDEX][i] = process_id;
                    page_array[TIME_INDEX][i] = currentTime;
                    page_needed_remaining -= 1;

                }
                
            }
            
            
        }else if( strcmp(memory_allocation, "cm") == 0) {
            //Calculate variable for pageNeeded
            int no_emptyPage = count_pageValue(page_array, page_array_size, EMPTY);
            int noPage_virtualMemory = VIRTUAL_MEMORY/SIZE_PER_PAGE;
            int noPage_evict = noPage_virtualMemory - no_emptyPage - count_pageValue(page_array, page_array_size, process_id);

            //Evicting to meet the minimum requirement
            if (noPage_evict > 0){
                check_and_evict(page_array, page_array_size, noPage_evict, currentTime, EVICTION_TYPE_CM, process_id);   
            }
            
            for(int i = 0; i < page_array_size; i++){
                if(page_array[PID_INDEX][i] == EMPTY && page_needed_remaining > 0) {
                    
                    //Assign the memory size array with its PID 
                    page_array[PID_INDEX][i] = process_id;
                    page_array[TIME_INDEX][i] = currentTime;
                    page_array[COUNTER_INDEX][i] += 1;
                    page_needed_remaining -= 1;

                }
                
            }


        }
    }

    //Adding reading page fault for all unsucessfull load
    process_info[REMAINING_JOB_TIME_INDEX] += page_needed_remaining;

    //Reducing loadTime when it is not successfully loaded
    loadTime -= (page_needed_remaining * LOAD_TIME_PER_PAGE);
    
    return loadTime;
}

/**
* The function will print the running process at the beginning of the processing
*/
void print_running_stats(int currentTime, int *process_info, char *memory_allocation, int loadTime, int **page_array, int page_array_size){

    int process_id = process_info[PID_INDEX];
    int remainingTime = process_info[REMAINING_JOB_TIME_INDEX];

    printf("%d, RUNNING, id=%d, remaining-time=%d", currentTime, process_id, remainingTime);
    if(strcmp(memory_allocation, "u") != 0){
        
        int no_emptyPage = count_pageValue(page_array, page_array_size, EMPTY);

        int page_used = page_array_size - no_emptyPage;
        int mem_usage_percent = ceil((float) page_used / page_array_size*100);

        printf(", load-time=%d, mem-usage=%d%%, mem-addresses=[", loadTime, mem_usage_percent);
        bool firstPrint = true;
        for (int i=0; i < page_array_size; i++){
            if (page_array[PID_INDEX][i] == process_id){
                if(firstPrint){
                    printf("%d", i);
                    firstPrint = false;
                }else{
                    printf(",%d", i);
                }
            }
        }
        printf("]");
    }
    printf("\n");
}

/**
* The function will print the finished process
*/
void finish_processing(int currentTime, int *process_info, int num_in_queue, int** page_array, int no_pages){
    int process_id = process_info[PID_INDEX];

    evict_pid(page_array, no_pages, process_id, currentTime);
    
    process_info[FINISH_TIME_INDEX] = currentTime;
    printf("%d, FINISHED, id=%d, proc-remaining=%d\n", currentTime, process_id, num_in_queue);
}


/**
* The function will print the final statistic of the project
*/
void print_statistic(int **input_array, int array_size, int currentTime){
    int intervalNo = ceil((float)currentTime/60);

    int *interval_array = calloc((intervalNo+1), sizeof(int));
    assert(interval_array);

    //Throughput
    int min_throughput = intervalNo;
    int max_throughput = 0;
    int total_throughput = 0;

    //Turnaround Time
    int total_turnaround_time = 0;
    
    //Time Overhead
    float max_overhead = 0;
    float total_overhead = 0;

    for (int i=0 ; i < array_size; i++){

        int finishTime = input_array[i][FINISH_TIME_INDEX];
        int arrivalTime = input_array[i][TIME_INDEX];
        int jobTime = input_array[i][JOB_TIME_INDEX];

        //Calculate Finishing interval
        int currentInterval = ceil((float)finishTime/60);
        interval_array[currentInterval] += 1;

        //Turnaround Time
        total_turnaround_time += (finishTime - arrivalTime);

        //Time Overhead
        total_overhead += ((float)(finishTime-arrivalTime)/jobTime);
        if(max_overhead < ((float)(finishTime-arrivalTime)/jobTime)){
            max_overhead = ((float)(finishTime-arrivalTime)/jobTime);
        }
    }

    //Calculate the interval values
    for (int i = 1; i <= intervalNo; i ++){
        if (interval_array[i] <  min_throughput){
            min_throughput = interval_array[i];
        }
        if (interval_array[i] >  max_throughput){
            max_throughput = interval_array[i];
        }
        total_throughput += interval_array[i];
    }
    free(interval_array);

    //Calculate avg_throughput
    int avg_throughput = ceil((float)total_throughput/intervalNo);

    //Calculate turnaround time
    int avg_turnaround_time = ceil((float)(total_turnaround_time) / array_size);
    float time_overhead[2] = {max_overhead, total_overhead/array_size};
    int makespan = currentTime;

    //Printng values
    printf("Throughput %d, %d, %d\n", avg_throughput, min_throughput, max_throughput);
    printf("Turnaround time %d\n", avg_turnaround_time);
    printf("Time overhead %.2f %.2f\n", time_overhead[0], time_overhead[1]);
    printf("Makespan %d\n", makespan);
}


/**
* The will return finish time needed based on the algorithm chosen
*/
int process_scheduler(int currentTime, char *scheduling_algorithm, int quantum, int *process_info){
    int finishTime = currentTime;
    int remainingTime = process_info[REMAINING_JOB_TIME_INDEX];

    //Choose wich algortuihm used
    if(strcmp(scheduling_algorithm, "ff") == 0 || strcmp(scheduling_algorithm, "cs") == 0){
        finishTime += remainingTime;
    }else if (strcmp(scheduling_algorithm, "rr") == 0){

        if(remainingTime > quantum){
            finishTime += quantum; 
        }else{
            finishTime += remainingTime; 
        }
    }
    
    return finishTime;
}




/**
* The function will check if page needed are availabe and evict if it is not enough
*/
void check_and_evict(int **page_array, int page_array_size, int required_page, int currentTime, int eviction_type, int process_id){
    int no_emptyPage = count_pageValue(page_array, page_array_size, EMPTY);
    int page_needed = required_page;

    //Evict page when needed when it is required based on the algorithm
    if(no_emptyPage < required_page){

        if(eviction_type == EVICTION_TYPE_V){
            while(page_needed > 0){
                int index_to_evict = smallestSpecified_pageIndex(page_array, page_array_size, process_id, TIME_INDEX);
                page_array[PID_INDEX][index_to_evict] = EMPTY;
                page_array[TIME_INDEX][index_to_evict] = currentTime;
                page_needed -=1;
            }
        }else if (eviction_type == EVICTION_TYPE_P){
            while(page_needed > 0){
                int index_to_evict = smallestSpecified_pageIndex(page_array, page_array_size, process_id, TIME_INDEX);
                page_array[PID_INDEX][index_to_evict] = EMPTY;
                page_array[TIME_INDEX][index_to_evict] = currentTime;
                page_needed -=1;
            }
        }else if (eviction_type == EVICTION_TYPE_CM){
            while(page_needed > 0){
                int index_to_evict = largestSpecified_pageIndex(page_array, page_array_size, process_id, COUNTER_INDEX);
                
                page_array[PID_INDEX][index_to_evict] = EMPTY;
                page_array[TIME_INDEX][index_to_evict] = currentTime;
                page_array[COUNTER_INDEX][index_to_evict] = EMPTY;
                page_needed -=1;
            }
        }

        //Printing evicted page
        printf("%d, EVICTED, mem-addresses=[", currentTime);
        bool firstPrint = true;
        for (int i = 0; i < page_array_size; i++){
            if (page_array[PID_INDEX][i] == EMPTY && page_array[TIME_INDEX][i] == currentTime){

                if(firstPrint){
                    printf("%d", i);
                    firstPrint = false;
                }else{
                    printf(",%d", i);
                }                
            }   
        }

        printf("]\n");
    }

}

/**
* The function will evict page when the process is completely finish
*/
void evict_pid(int **page_array, int array_size, int process_id, int currentTime ){

    bool isEvicting = false;

    for(int i =0; i < array_size; i++){
        if(page_array[PID_INDEX][i] == process_id){

            page_array[PID_INDEX][i] = EMPTY;
            page_array[TIME_INDEX][i] = currentTime;
            isEvicting = true;

        }
    
    } 

    //Printing statistic of the evicted page
    if(isEvicting){
        bool firstPrint = true;
        for(int i =0; i < array_size; i++){
            if(page_array[PID_INDEX][i] == EMPTY && page_array[TIME_INDEX][i] == currentTime){
                if(firstPrint){
                    printf("%d, EVICTED, mem-addresses=[", currentTime);
                    printf("%d", i);
                    firstPrint = false;
                }else{
                    printf(",%d", i);
                }
            }
        }
        printf("]\n");
    }
}
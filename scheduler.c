/*
	COMP30023 Project 2 2020
	by William Putra Intan
*/

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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

typedef struct node_t {
    int value;
    struct node_t * next;
} *node;

//funct prototype

int count_loadTime(int *process_info, int **page_array, int page_array_size);
void parsingInput(int argc, char **argv, char **filename, char **scheduling_algorithm, char **memory_allocation, int *memory_size, int *quantum);
void readFile(char *filename, int ***input_array, int *input_lines);
void swapArray( int *a, int *b, size_t n );
node createNode(int value);
node push_node(node head, int value);
int pop(node * head);
int processing_job( int *process_info, int currentTime, char *memory_allocation, int **page_array, int page_array_size );
void finish_processing(int currentTime, int *process_info, int num_in_queue, int **page_array, int no_pages);
void print_statistic(int **input_array, int array_size, int currentTime);
void check_and_evict(int **page_array, int page_array_size, int required_page, int currentTime, int eviction_type, int process_id);
void evict_pid(int **page_array, int array_size, int process_id, int currentTime);
int count_pageValue(int **page_array, int array_size, int match_value);
void print_running_stats(int currentTime, int *process_info, char *memory_allocation, int loadTime, int **page_array, int page_array_size);
int process_scheduler(int currentTime, char * scheduling_algorithm, int quantum, int *process_info);
int smallestSpecified_pageIndex(int **page_array, int page_array_size, int process_id, int specifiedIndex);
int largestSpecified_pageIndex(int **page_array, int page_array_size, int process_id, int specifiedIndex);
int smallestJobTime_Index(int **input_array, int array_size, int currentTime);

/*Main Function of the program*/
int main (int argc, char **argv)
{
    //Reading value 
    char *filename = NULL;
    char *scheduling_algorithm = NULL;
    char *memory_allocation = NULL;
    int memory_size = 0;
    int quantum = DEFAULT_QUANTUM;

    //Parsing STDIN values
    parsingInput(argc, argv, &filename, &scheduling_algorithm, &memory_allocation, &memory_size, &quantum);

    //Reading input file
    int **input_array = malloc(sizeof(int*));
    assert(input_array);
    int input_lines = 0;

    //Reading each line file
    readFile(filename, &input_array, &input_lines);


    // creating index queue list
    node listHead = NULL;
    
    // Memory Managment declaring an aray
    int no_pages = memory_size/SIZE_PER_PAGE; 
    int page_array_row = 3;

    int *page_array[page_array_row];
    for(int i=0; i< page_array_row; i++) {
        page_array[i] = malloc(no_pages * sizeof(int));
        assert(page_array[i]);
        for(int j=0; j<no_pages; j++){
            page_array[i][j] = EMPTY;
        }
    }


    //Declaring variables for the process
    int currentTime = 0;
    int process_completed = 0;
    int numberInQueue = 0;

    int inputArrayIndex = 0;
    bool isProcessing = false;
    int processing_index = 0;
    int finishTime = 0;
    int loadTime = 0;

    //Looping every second until no more input
    while(true){
        
        //accessing all input at current time
        while(inputArrayIndex < input_lines){
            if (input_array[inputArrayIndex][TIME_INDEX] != currentTime){
                break;
            }
            if(strcmp(scheduling_algorithm, "cs") != 0){

                //Putting process into the jobList
                listHead = push_node(listHead, inputArrayIndex);
                
            }
            //Adding number of queue
            numberInQueue += 1;
            inputArrayIndex += 1;
        }

        //Reduce remaining job time each second after loading is done
        if(isProcessing){
            if(loadTime <= 0){
                input_array[processing_index][REMAINING_JOB_TIME_INDEX] -= 1;
            }else{
                loadTime -= 1;
            }
        }

        //Process finished job 
        if (isProcessing == true && currentTime == finishTime){
            //If job have not completed
            if (input_array[processing_index][REMAINING_JOB_TIME_INDEX] > 0){

                listHead = push_node(listHead, processing_index);
                numberInQueue += 1 ;
            } else {
                //Proccess Job completed
                finish_processing(currentTime, input_array[processing_index], numberInQueue, page_array, no_pages);
                process_completed += 1;

            }
            isProcessing =false;
        }

        //Processing new job when available
        if(isProcessing == false && numberInQueue > 0){
            
            //Assign next job depending on it algorithm
            if(strcmp(scheduling_algorithm, "cs") == 0){
                processing_index = smallestJobTime_Index(input_array, input_lines, currentTime);

            } else {
                processing_index = pop(&listHead);
            }
            numberInQueue -= 1;

            //Adding loadTime if the process have limited memory
            if(strcmp(memory_allocation, "u") != 0){
                loadTime = processing_job( input_array[processing_index], currentTime, memory_allocation, page_array, no_pages);
            }
            
            //Calculate finished time
            finishTime = process_scheduler(currentTime, scheduling_algorithm, quantum, input_array[processing_index]);
            finishTime += loadTime;

            //Printing statistic of processing job
            print_running_stats(currentTime, input_array[processing_index], memory_allocation, loadTime, page_array, no_pages);

            isProcessing = true;           
        }

        //If no more input are given to the program
        if (process_completed >= input_lines){
            break;
        }
        
        currentTime+=1;
    }
    
    print_statistic(input_array, input_lines,  currentTime);

    //Free all allocated memory
    for (int i=0; i<input_lines;i++){
        free(input_array[i]);
    }
    for (int i=0; i<page_array_row; i++){
        free(page_array[i]);
    }
    free(input_array);


    return 0;
}
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
* Parsing input from STDIN to their assign variable
*/
void parsingInput(int argc, char **argv, char **filename, char **scheduling_algorithm, char **memory_allocation, int *memory_size, int *quantum){
    
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "f:a:m:s:q:")) != -1){
        switch (c)
        {
            case 'f':
                *filename = optarg;
                break;
            case 'a':
                *scheduling_algorithm = optarg;
                break;
            case 'm':
                *memory_allocation = optarg;
                break;
            case 's':
                *memory_size = atoi(optarg);
                break;
            case 'q':
                *quantum = atoi(optarg);
                break;
            case '?':
                if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                exit(0);
            default:
                abort ();
        }
    }
    for (index = optind; index < argc; index++){
        printf ("Non-option argument %s\n", argv[index]);
    }


}

/**
* Reading .txt file and put it on a 2d array
*/
void readFile(char *filename, int ***input_array, int *input_lines){

    FILE *fp;
    //Open the current files
    fp = fopen(filename, "r"); // read mode
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    
    //Assign each input line to the array
    int val1, val2, val3, val4;
    while(fscanf(fp, "%d %d %d %d\n", &val1, &val2, &val3, &val4) != EOF){
        //Allocating memory for new line
        (*input_array) = realloc(*input_array, ( (*input_lines+1) * sizeof(int*)) );
        assert(input_array);

        //Allocating memory for the data each line
        (*input_array)[*input_lines]= malloc(6*sizeof(int));
        assert((*input_array)[*input_lines]);

        //Assign the data
        (*input_array)[*input_lines][TIME_INDEX] = val1;
        (*input_array)[*input_lines][PID_INDEX] = val2;
        (*input_array)[*input_lines][MEM_SIZE_INDEX] = val3;
        (*input_array)[*input_lines][JOB_TIME_INDEX] = val4;
        (*input_array)[*input_lines][FINISH_TIME_INDEX] = 0;
        (*input_array)[*input_lines][REMAINING_JOB_TIME_INDEX] = val4;

        //Swap between array so have the increasing process-id at the same time
        if (*input_lines > 0){
            for(int i = (*input_lines) ; i > 0; i-- ){
                if ((*input_array)[i][TIME_INDEX] != (*input_array)[i-1][TIME_INDEX]){
                    break;
                }
                if((*input_array)[i][PID_INDEX] < (*input_array)[i-1][PID_INDEX]){
                    swapArray((*input_array)[i] , (*input_array)[i-1], 6);
                }

            }
        }

        *input_lines+=1;
    }   
    
    fclose(fp);
 
}


/**
* This function will swap lines in the 2d array and is taken from 
* https://stackoverflow.com/questions/49751409/swapping-2-arrays-in-c
*/
void swapArray( int *a, int *b, size_t n )
{
    for ( size_t i = 0; i < n; i++ )
    {
        int tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

/**
* This function will create node and is taken and modified from 
* https://www.hackerearth.com/practice/data-structures/linked-list/singly-linked-list/tutorial/
*/
node createNode(int data){
    node temp;
    temp = (node)malloc(sizeof(struct node_t));
    assert(temp);
    temp->value = data;
    temp->next = NULL;
    return temp;
}

/**
* This function will add a new node in the lis and is taken and modified from 
* https://www.hackerearth.com/practice/data-structures/linked-list/singly-linked-list/tutorial/
*/
node push_node(node head, int value) {
    node new_node, temp ;
    new_node = createNode(value);
     
    if(head == NULL){
        head = new_node;
    }
    else{
        temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return head;
}

/**
* This function will remove the next queue from the list and is taken from 
* https://www.learn-c.org/en/Linked_lists
*/
int pop(node * head) {
    int retval = -1;
    node next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->value;
    free(*head);
    *head = next_node;

    return retval;
}
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
                check_and_evict(page_array, page_array_size, noPage_evict, currentTime, EVICTION_TYPE_V, process_id);   
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
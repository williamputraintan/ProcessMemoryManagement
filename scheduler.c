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
int smallestTime_Index(int **page_array, int page_array_size, int process_id);

/*Main Function of the program*/
int main (int argc, char **argv)
{
    //Reading value 
    char *filename = NULL;
    char *scheduling_algorithm = NULL;
    char *memory_allocation = NULL;
    int memory_size = 0;
    int quantum = DEFAULT_QUANTUM;

    parsingInput(argc, argv, &filename, &scheduling_algorithm, &memory_allocation, &memory_size, &quantum);

    // printf ("filename = %s, scheduling_algorithm = %s, memory_allocation = %s, memory_size = %d, quantum = %d\n\n", filename,scheduling_algorithm,memory_allocation,memory_size, quantum);

    //Reading input file
    int **input_array = malloc(sizeof(int*));
    int input_lines = 0;

    readFile(filename, &input_array, &input_lines);

    // creating index list
    node listHead = NULL;
    
    // Memory Managment 
    int no_pages = memory_size/SIZE_PER_PAGE; 

    int *page_array[2];
    for(int i=0; i< 2; i++) {
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
            listHead = push_node(listHead, inputArrayIndex);
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


        if(isProcessing == false && numberInQueue > 0){
            
            processing_index = pop(&listHead);

            if(strcmp(memory_allocation, "u") != 0){

                loadTime = processing_job( input_array[processing_index], currentTime, memory_allocation, page_array, no_pages);
                
            }

            finishTime = process_scheduler(currentTime, scheduling_algorithm, quantum, input_array[processing_index]);

            finishTime += loadTime;

            print_running_stats(currentTime, input_array[processing_index], memory_allocation, loadTime, page_array, no_pages);

            numberInQueue -= 1;
            isProcessing = true;           
        }

        //If no more input are given to the program
        if (process_completed >= input_lines){
            break;
        }
        
        currentTime+=1;
    }
    
    print_statistic(input_array, input_lines,  currentTime);

    //print and free input array
    // printf("\n\npcount = %d\n", input_lines);
    for (int i=0; i<input_lines;i++){
        
        // printf("%d %d %d %d\n", input_array[i][TIME_INDEX], input_array[i][PID_INDEX], input_array[i][MEM_SIZE_INDEX], input_array[i][JOB_TIME_INDEX]);
        free(input_array[i]);
    }

    for (int i=0; i<2; i++){
        free(page_array[i]);
    }

    free(input_array);
    // free(page_array);
    return 0;
}

int count_loadTime(int *process_info, int **page_array, int page_array_size){
    int process_id = process_info[PID_INDEX];
    int memory_size = process_info[MEM_SIZE_INDEX];

    int numPage_needed = ceil((float)memory_size/SIZE_PER_PAGE);
    int numPage_loaded = count_pageValue(page_array, page_array_size, process_id);

    int numPage_toLoad = numPage_needed - numPage_loaded;

    int loadTime = numPage_toLoad * LOAD_TIME_PER_PAGE;

    return loadTime;
}

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

    // printf ("filename = %s, scheduling_algorithm = %s, memory_allocation = %s, memory_size = %d, quantum = %d\n", filename,scheduling_algorithm,memory_allocation,memory_size, quantum);

    for (index = optind; index < argc; index++){
        printf ("Non-option argument %s\n", argv[index]);
    }


}


void readFile(char *filename, int ***input_array, int *input_lines){

    FILE *fp;

    fp = fopen(filename, "r"); // read mode
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    
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
                    swapArray((*input_array)[i] , (*input_array)[i-1], 5);
                }

            }
        }

        *input_lines+=1;
    }   
    
    fclose(fp);
 

}


/**
* This following function swapArray is taken from 
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
* This function is taken and modified from 
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
* This function is taken and modified from 
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
* This function is taken from 
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


int processing_job( int *process_info, int currentTime, char *memory_allocation, int **page_array, int page_array_size ){


    int loadTime = count_loadTime(process_info, page_array, page_array_size);
    
    int page_needed_remaining = loadTime / LOAD_TIME_PER_PAGE;

    int process_id = process_info[PID_INDEX];
// for(int i=0; i<page_array_size; i++) fprintf(stderr, "konidisi index %d = %d\n",i, page_array[PID_INDEX][i]);
    if(page_needed_remaining>0){

        if( strcmp(memory_allocation, "p") == 0) {
            
            check_and_evict(page_array, page_array_size, page_needed_remaining, currentTime, EVICTION_TYPE_P, process_id);

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
            //Adding reading page fault
            process_info[REMAINING_JOB_TIME_INDEX] += page_needed_remaining;

        }
    }
// fprintf(stderr, "FILLING\n");
// for(int i=0; i<page_array_size; i++) fprintf(stderr, "konidisi PENGISINAN index %d = %d\n",i, page_array[PID_INDEX][i]);
    loadTime -= (page_needed_remaining * LOAD_TIME_PER_PAGE);
    
    return loadTime;
}

int process_scheduler(int currentTime, char *scheduling_algorithm, int quantum, int *process_info){
    int finishTime = currentTime;
    int remainingTime = process_info[REMAINING_JOB_TIME_INDEX];
    if(strcmp(scheduling_algorithm, "ff") == 0){

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

void finish_processing(int currentTime, int *process_info, int num_in_queue, int** page_array, int no_pages){
    int process_id = process_info[PID_INDEX];

    evict_pid(page_array, no_pages, process_id, currentTime);
    
    process_info[FINISH_TIME_INDEX] = currentTime;
    printf("%d, FINISHED, id=%d, proc-remaining=%d\n", currentTime, process_id, num_in_queue);
}


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

    int avg_throughput = ceil((float)total_throughput/intervalNo);

    int avg_turnaround_time = ceil((float)(total_turnaround_time) / array_size);
    float time_overhead[2] = {max_overhead, total_overhead/array_size};
    int makespan = currentTime;

    printf("Throughput %d, %d, %d\n", avg_throughput, min_throughput, max_throughput);
    printf("Turnaround time %d\n", avg_turnaround_time);
    printf("Time overhead %.2f %.2f\n", time_overhead[0], time_overhead[1]);
    printf("Makespan %d\n", makespan);
}


void check_and_evict(int **page_array, int page_array_size, int required_page, int currentTime, int eviction_type, int process_id){
    int no_emptyPage = count_pageValue(page_array, page_array_size, EMPTY);

    int page_needed = required_page;
// fprintf(stderr, "no_emptyPage = %d\n", no_emptyPage);
// fprintf(stderr, "required_page = %d\n", required_page);
    if(no_emptyPage < required_page){
// for(int i=0; i<page_array_size; i++) fprintf(stderr, "BEFOREvicting index %d = %d\n",i, page_array[PID_INDEX][i]);
// fprintf(stderr, "PROCESS \n");
        if(eviction_type == EVICTION_TYPE_V){
            
            while(page_needed > 0){
                int index_to_evict = smallestTime_Index(page_array, page_array_size, process_id);
                page_array[PID_INDEX][index_to_evict] = EMPTY;
                page_array[TIME_INDEX][index_to_evict] = currentTime;
                page_needed -=1;
            }
        }else if (eviction_type == EVICTION_TYPE_P){
            while(page_needed > 0){
                int index_to_evict = smallestTime_Index(page_array, page_array_size, process_id);
                page_array[PID_INDEX][index_to_evict] = EMPTY;
                page_array[TIME_INDEX][index_to_evict] = currentTime;
                page_needed -=1;
            }
        }

        
// for(int i=0; i<page_array_size; i++) fprintf(stderr, "AFTERevicting index %d = %d\n",i, page_array[PID_INDEX][i]);
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

void evict_pid(int **page_array, int array_size, int process_id, int currentTime ){

    bool isEvicting = false;

    for(int i =0; i < array_size; i++){
        if(page_array[PID_INDEX][i] == process_id){

            page_array[PID_INDEX][i] = EMPTY;
            page_array[TIME_INDEX][i] = currentTime;
            isEvicting = true;

        }
    
    } 


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

        //Print ending if there eviction occurs
        printf("]\n");
    }
}

int count_pageValue(int **page_array, int array_size, int match_value){
    int count = 0;

    for (int i=0; i < array_size; i++){
        if (page_array[PID_INDEX][i] == match_value){
            count += 1;
        }
    }
    
    return count;
}

int smallestTime_Index(int **page_array, int page_array_size, int process_id){
    int smallestTime;
    int smallestTime_Index = 0;

    int i = 0;
    //find non empty time
    while ( i < page_array_size){
        if(page_array[TIME_INDEX][i] != EMPTY && page_array[PID_INDEX][i] != process_id){
            smallestTime = page_array[TIME_INDEX][i];
            smallestTime_Index = i;
            break;
        }
        i += 1;
    }

    while ( i < page_array_size){
        if(page_array[TIME_INDEX][i] != EMPTY && page_array[TIME_INDEX][i] < smallestTime && page_array[PID_INDEX][i] != process_id){
            smallestTime = page_array[TIME_INDEX][i];
            smallestTime_Index = i;
        }
        i += 1;
    }


    return smallestTime_Index;
}
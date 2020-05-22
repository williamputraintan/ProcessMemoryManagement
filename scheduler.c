/*
	COMP30023 Project 2 2020
	by William Putra Intan
*/

#define DEFAULT_QUANTUM 10      //in seconds
#define SIZE_PER_PAGE 4         //in KB
#define LOAD_TIME_PER_PAGE 2    //in seconds
#define TIME_INDEX 0
#define PID_INDEX 1
#define MEM_SIZE_INDEX 2
#define JOB_TIME_INDEX 3
#define FINISH_TIME_INDEX 4
#define REMAINING_JOB_TIME_INDEX 5
#define EMPTY -1


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

int count_loadTime(int memory_size);
void parsingInput(int argc, char **argv, char **filename, char **scheduling_algorithm, char **memory_allocation, int *memory_size, int *quantum);
void readFile(char *filename, int ***input_array, int *input_lines);
void swapArray( int *a, int *b, size_t n );
node createNode(int value);
node push_node(node head, int value);
int pop(node * head);
int processing_job( int *job_card, int currentTime, char *scheduling_algorithm, int quantum, char *memory_allocation, \
        int* page_array, int *next_page_index, int page_array_size);
void finish_processing(int currentTime, int *job_card, int num_in_queue);
void print_statistic(int **input_array, int array_size, int currentTime);
void check_and_evict(int *page_array, int page_array_size, int required_page, int page_index, int currentTime);
void evict_pid(int *page_array, int array_size, int process_id, int currentTime, int *next_page_index);
int check_empty_page(int *page_array, int array_size);
int check_occupied_page(int *page_array, int array_size);

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
    int page_index = 0;
    int no_pages = memory_size/SIZE_PER_PAGE;

    int page_array[no_pages];
    for(int i=0; i< no_pages; i++) page_array[i] = EMPTY;
    // int *page_array = malloc(no_pages * sizeof(int));
    // assert(page_array);
    // memset(page_array, -1, no_pages);


    //Declaring variables for the process
    int currentTime = 0;
    int process_completed = 0;
    int numberInQueue = 0;

    int inputArrayIndex = 0;
    bool isProcessing = false;
    int processing_index = 0;
    int finishTime = 0;

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

        if(isProcessing){
            input_array[processing_index][REMAINING_JOB_TIME_INDEX] -= 1;
        }

        if (isProcessing == true && currentTime == finishTime){
// fprintf(stderr, "remainingjob = %d\n", input_array[processing_index][REMAINING_JOB_TIME_INDEX]);
            //If job have not completed
            if (input_array[processing_index][REMAINING_JOB_TIME_INDEX] > 0){
                push_node(listHead, processing_index);
                numberInQueue += 1 ;
            } else {
                //Proccess Job completed
                evict_pid(page_array, no_pages, input_array[processing_index][PID_INDEX], currentTime, &page_index);

                finish_processing(currentTime, input_array[processing_index], numberInQueue);
                process_completed += 1;

            }
            
            isProcessing =false;
        }


        if(isProcessing == false && numberInQueue > 0){
            processing_index = pop(&listHead);

            finishTime = processing_job( input_array[processing_index], currentTime, scheduling_algorithm, quantum, memory_allocation, \
                page_array, &page_index, no_pages);
                
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
    free(input_array);
    // free(page_array);
    return 0;
}

int count_loadTime(int memory_size){

    int num_page = memory_size/SIZE_PER_PAGE;
    int loadTime = num_page * LOAD_TIME_PER_PAGE;

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


int processing_job( int *job_card, int currentTime, char *scheduling_algorithm, int quantum, char *memory_allocation, \
        int* page_array, int *next_page_index, int page_array_size){

    int finishTime = currentTime;
    int process_id = job_card[PID_INDEX];
    int remainingTime = job_card[REMAINING_JOB_TIME_INDEX];
    int memory_size = job_card[MEM_SIZE_INDEX];

    int page_needed_remaining = ceil((float)memory_size/SIZE_PER_PAGE);
    int loadTime = page_needed_remaining * LOAD_TIME_PER_PAGE;

    if(strcmp(memory_allocation, "u") != 0){
        finishTime += loadTime;
    }

    if( strcmp(memory_allocation, "p") == 0) {
        
        check_and_evict(page_array, page_array_size, page_needed_remaining, *next_page_index, currentTime);
        while(page_needed_remaining > 0){

            //Assign the memory size array with its PID
            page_array[*next_page_index] = process_id;

            *next_page_index += 1;

            if (*next_page_index >= page_array_size){
                *next_page_index=0;
            }
            
            page_needed_remaining -= 1;
        }
        
    }

    if(strcmp(scheduling_algorithm, "ff") == 0){

        finishTime += remainingTime; 

    }else if (strcmp(scheduling_algorithm, "rr") == 0){

        if(remainingTime > quantum){
            finishTime += quantum; 
        }else{
            finishTime += remainingTime; 
        }
    }
    

    printf("%d, RUNNING, id=%d, remaining-time=%d", currentTime, process_id, remainingTime);
    if(strcmp(memory_allocation, "u") != 0){
        
        // //calculate num_page_use
        // int num_page_use = 0;
        // for (int i=0; i < page_array_size; i++){
        //     if (page_array[i] == 1){
        //         num_page_use += 1;
        //     }
        // }
        int page_used = check_occupied_page(page_array, page_array_size);
        int mem_usage_percent = ceil((float) page_used / page_array_size*100);

        printf(", load-time=%d, mem-usage=%d%%, mem-addresses=[", loadTime, mem_usage_percent);
        bool firstPrint = true;
        for (int i=0; i < page_array_size; i++){
            if (page_array[i] == process_id){
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
    return finishTime;
}


void finish_processing(int currentTime, int *job_card, int num_in_queue){
    int process_id = job_card[PID_INDEX];
    job_card[FINISH_TIME_INDEX] = currentTime;
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


void check_and_evict(int *page_array, int page_array_size, int required_page, int page_index, int currentTime){
    int no_emptyPage = check_empty_page(page_array, page_array_size);
    int page_needed = required_page;
    // fprintf(stderr, "no empty page  =%d, page needed =%d page_index = %d\n", no_emptyPage, page_needed, page_index);
    if(no_emptyPage < required_page){
        printf("%d, EVICTED, mem-addresses=[", currentTime);
        bool firstPrint = true;
        while(page_needed > 0 ){
            
            if (page_array[page_index] > EMPTY){
                if(firstPrint){
                    printf("%d", page_index);
                    firstPrint = false;
                }else{
                    printf(",%d", page_index);
                }
                page_array[page_index] = 0;
            }

            if (page_index >= page_array_size){
                page_index = 0;
            } else {
                page_index += 1;
            }
            page_needed -= 1;
        }

        printf("]\n");
    }
    // fprintf(stderr, "no empty page  =%d, page needed =%d\n", no_emptyPage, page_needed);
}

void evict_pid(int *page_array, int array_size, int process_id, int currentTime, int *next_page_index ){
    
    bool firstPrint = true;
    for(int i =0; i < array_size; i++){
        if(page_array[i] == process_id){
            page_array[i] = EMPTY;
            *next_page_index -= 1;

            if(firstPrint){
                printf("%d, EVICTED, mem-addresses=[", currentTime);
                printf("%d", i);
                firstPrint = false;
            }else{
                printf(",%d", i);
            }

            if(*next_page_index < 0) {
                *next_page_index = array_size;
            }
        }
    }

    //Print ending if there eviction occurs
    if(!firstPrint) printf("]\n");
}

int check_empty_page(int *page_array, int array_size){
    int empty_page = 0;

    for (int i=0; i < array_size; i++){
        if (page_array[i] == EMPTY){
            empty_page += 1;
        }
    }
    
    return empty_page;
}

int check_occupied_page(int *page_array, int array_size){
    int occupied_page = 0;

    for (int i=0; i < array_size; i++){
        if (page_array[i] > EMPTY){
            occupied_page += 1;
        }
    }
    return occupied_page;
}
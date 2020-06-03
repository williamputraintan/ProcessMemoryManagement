/*
	COMP30023 Project 2 2020
	by William Putra Intan
*/
#include "constant.h"
#include "queue.h"
#include "inputFunc.h"
#include "processingFunc.h"
#include "helperProcessingFunc.h"

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

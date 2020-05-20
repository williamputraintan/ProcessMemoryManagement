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
#define ISPROCESSED_INDEX 4
#define TRUE 1
#define FALSE 0

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>


//funct prototype

int count_loadTime(int memory_size);
void parsingInput(int argc, char **argv, char **filename, char **scheduling_algorithm, char **memory_allocation, int *memory_size, int *quantum);
void readFile(char *filename, int ***input_array, int *process_count);


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

    printf ("filename = %s, scheduling_algorithm = %s, memory_allocation = %s, memory_size = %d, quantum = %d\n", filename,scheduling_algorithm,memory_allocation,memory_size, quantum);

    int **input_array = malloc(sizeof(int*));
    int process_count = 0;

    readFile(filename, &input_array, &process_count);





    printf("pcount = %d\n", process_count);
    for (int i=0; i<process_count;i++){

        printf("%d %d %d %d\n", input_array[i][TIME_INDEX], input_array[i][PID_INDEX], input_array[i][MEM_SIZE_INDEX], input_array[i][JOB_TIME_INDEX]);
        free(input_array[i]);
    }
    free(input_array);
    
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


void readFile(char *filename, int ***input_array, int *process_count){

    FILE *fp;

    fp = fopen(filename, "r"); // read mode
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    printf("The contents of %s file are:\n", filename);
    
    int val1, val2, val3, val4;
    while(fscanf(fp, "%d %d %d %d\n", &val1, &val2, &val3, &val4) != EOF){
        //Allocating memory for new line
        (*input_array) = realloc(*input_array, ( (*process_count+1) * sizeof(int*)) );
        assert(input_array);

        //Allocating memory for the data each line
        (*input_array)[*process_count]= malloc(10*sizeof(int));
        assert(input_array[*process_count]);

        //Assign the data
        (*input_array)[*process_count][TIME_INDEX] = val1;
        (*input_array)[*process_count][PID_INDEX] = val2;
        (*input_array)[*process_count][MEM_SIZE_INDEX] = val3;
        (*input_array)[*process_count][JOB_TIME_INDEX] = val4;
        (*input_array)[*process_count][ISPROCESSED_INDEX] = 0;

        *process_count+=1;
    }   
    fclose(fp);
 

}


int nextProccess(){
    return 0;
}
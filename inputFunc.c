

#include "inputFunc.h"

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
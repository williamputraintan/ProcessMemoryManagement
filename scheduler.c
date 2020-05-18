/*
	COMP30023 Project 2 2020
	by William Putra Intan
*/

#define DEFAULT_QUANTUM 10
#define SIZE_PER_PAGE 4         //in KB
#define LOAD_TIME_PER_PAGE 2    //in seconds


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


//funct prototype

int count_loadTime(int memory_size);


/*Main Function of the program*/
int main (int argc, char **argv)
{
    //Reading value 
    char *filename = NULL;
    char *scheduling_algorithm = NULL;
    char *memory_allocation = NULL;
    int memory_size;
    int quantum = DEFAULT_QUANTUM;

    int index;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "f:a:m:s:q:")) != -1){
        switch (c)
        {
            case 'f':
                filename = optarg;
                break;
            case 'a':
                scheduling_algorithm = optarg;
                break;
            case 'm':
                memory_allocation = optarg;
                break;
            case 's':
                memory_size = atoi(optarg);
                break;
            case 'q':
                quantum = atoi(optarg);
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
                return 1;
            default:
                abort ();
        }
    }

    printf ("filename = %s, scheduling_algorithm = %s, memory_allocation = %s, memory_size = %d, quantum = %d\n", \
        filename,scheduling_algorithm,memory_allocation,memory_size, quantum);

    for (index = optind; index < argc; index++){
        printf ("Non-option argument %s\n", argv[index]);
    }


    // int input_array[][4];
    // int process_count = 0;

    char ch;
    FILE *fp;

    fp = fopen(filename, "r"); // read mode

    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    printf("The contents of %s file are:\n", filename);

    while((ch = fgetc(fp)) != EOF){
        
        printf("%c", ch);
    }

    fclose(fp);






    return 0;
}

int count_loadTime(int memory_size){

    int num_page = memory_size/SIZE_PER_PAGE;
    int loadTime = num_page * LOAD_TIME_PER_PAGE;

    return loadTime;
}

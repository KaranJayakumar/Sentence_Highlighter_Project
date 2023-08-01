/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2023 Semester 1
    
    Make using
        make problem2a
    
    Run using
        ./problem2a table < text

        or 

        ./problem2a -c table < text
    
    where table is the colour table in the expected
        format (e.g. test_cases/2a-1-table.txt), and
        text is a file containing input text, for 
        example:
    
        ./problem2a test_cases/2a-1-table.txt < test_cases/2a-1-text.txt
    
    The -c can optionally be included as the first 
    argument to print the colours of each term out
    to the terminal in the assigned colours where the
    colour is available.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <error.h>
#include "problem.h"

/* If no -c is provided, the table file is the first argument. */
#define DEFAULT_ARGV_TABLE_FILE 1

int main(int argc, char **argv){
    struct problem *problem;
    struct solution *solution;
    /* Use standard input stream for text. */
    FILE *textFile = stdin;
    /* Load file with table from argv[1] or argv[2]. */
    FILE *tableFile = NULL;
    int tableFileArgIndex = DEFAULT_ARGV_TABLE_FILE;
    int colourMode = 0;

    if(argc < 2){
        fprintf(stderr, "You only gave %d arguments to the program, \n"
            "you should run the program with in the form \n"
            "\t./problem2a table < text\n", argc);
        return EXIT_FAILURE;
    } else {
        /* First argument may be -c. */
        if(argv[1][0] == '-' && argv[1][1] == 'c'){
            colourMode = 1;
            if(argc < 3){
                fprintf(stderr, "You only gave %d arguments to the program, \n"
                    "you should run the program with in the form \n"
                    "\t./problem2a -c table < text\n", argc);
                return EXIT_FAILURE;
            }
            /* If the first argument is -c, the second argument will be the table file */
            tableFileArgIndex++;
        }
        /* Sanity check - we should have the argument for the tableFile */
        assert((tableFileArgIndex + 1) >= argc);
        tableFile = fopen(argv[tableFileArgIndex], "r");
        /* Ensure the file was able to be successfully opened. */
        if(! tableFile){
            fprintf(stderr, "File given as table file was \"%s\", which was unable to be opened\n", argv[2]);
            perror("Reason for file open failure");
            return EXIT_FAILURE;
        }
    }

    problem = readProblemA(textFile, tableFile);

    if(tableFile){
        fclose(tableFile);
    }

    solution = solveProblemA(problem);

    outputProblem(problem, solution, stdout, colourMode);

    freeSolution(solution, problem);

    freeProblem(problem);

    return EXIT_SUCCESS;
}

/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2023 Semester 1
    
    Implementation for data structure used
        in storing the solution and its
        information.

    You may change this file if you would
        like to add additional fields.
*/
struct solution {
    /* The number of terms in the problem. */
    int termCount;
    /* The colour for each term in the sequence of tokens. */
    int *termColours;
    /* The total score for the sentence. */
    int score;
};

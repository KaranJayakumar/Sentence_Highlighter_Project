/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2023 Semester 1
    
    Implementation for data structure used
        in storing the problem and its
        information.
*/

struct termColourTable;

struct colourTransitionTable;

struct termColourTable {
    /* The term the table is for. */
    char *term;
    /* The number of colours in the table including 
        no colour. */
    int colourCount;
    /* The colours. */
    int *colours;
    /* The score for each colour. */
    int *scores;
};

struct colourTransitionTable {
    /* The number of colour transitions in the table. */
    int transitionCount;
    /* The preceeding colours. */
    int *prevColours;
    /* The following colours. */
    int *colours;
    /* The score for each colour transition. */
    int *scores;
};

#ifndef PROBLEMPARTENUM_DEF
#define PROBLEMPARTENUM_DEF 1
enum problemPart {
    PART_A = 0,
    PART_B = 1,
    PART_E = 2,
    PART_F = 3
};
#endif

struct problem {
    /* The number of terms in the text. */
    int termCount;
    /* The original text. */
    char *text;
    /* 
        The term broken into tokens. These will
        be fresh strings if the they are not
        in the term colour table, if they are
        in the term colour table, they will be
        stored at the same place in memory so
        terms in the term table can be compared 
        using either strcmp or equality.
    */
    char **terms;

    /* Which problem part is being solved. */
    enum problemPart part;

    /* Part A onwards. */
    /* The number of term colour tables. */
    int termColourTableCount;
    /* The term colour tables, one for each term. */
    struct termColourTable *colourTables;

    /* Part B onwards. */
    /* 
        The colour transition table, describing the cost 
        of transitions between colours.
    */
    struct colourTransitionTable *colourTransitionTable;
};



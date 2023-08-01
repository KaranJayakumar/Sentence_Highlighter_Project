/*
    Written by Grady Fitzpatrick for
    COMP20007 Assignment 2 2023 Semester 1

    Implementation for module which contains
        Problem 2-related data structures and
        functions.

    Code implemented by <YOU>
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "problem.h"
#include "problemStruct.c"
#include "solutionStruct.c"

/* Number of terms to allocate space for initially. */
#define INITIALTERMS 64

/* Number of colour transitions to allocate space for initially. */
#define INITIALTRANSITIONS 16

/* -1 to show the colour hasn't been set. */
#define DEFAULTCOLOUR (-1)
/* -1 to be lower than zero to highlight in case accidentally used. */
#define DEFAULTSCORE (-1)

/* No colour is assigned where no highlighting rules are present. */
#define NO_COLOUR (0)

/* Marker for non-allowed colours. */
#define NONALLOWED (INT_MIN / 2)

struct problem;
struct solution;

/* Gets the colour with  the maximum value in the colour table for part A*/
int get_max_colour(struct problem *p, int index, char *word, int *score);

/*Gets the color of the transition between the previous and current*/
int get_transition(struct problem *p, int index,
                   int prev_colour, int curr_colour);

/*Returns the maximum color given 2 scores for 2 colors*/
int max(int score_a, int score_b, int a_colour, int b_colour);

int is_term(struct problem *p, int index);

/* Sets up a solution for the given problem. */
struct solution *newSolution(struct problem *problem);

/*
    Reads the given text file into a set of tokens in a sentence
    and the given table file into a set of structs.

    Assumption: Tables are always contiguous, meaning the table never
    needs to be constructed
*/
struct problem *readProblemA(FILE *textFile, FILE *tableFile)
{
    struct problem *p = (struct problem *)malloc(sizeof(struct problem));
    assert(p);

    /* Part B onwards so set as empty. */
    p->colourTransitionTable = NULL;

    int termCount = 0;
    char *text = NULL;
    char **terms = NULL;

    int termColourTableCount = 0;
    struct termColourTable *colourTables = NULL;

    /* Read in text. */
    size_t allocated = 0;
    /* Exit if we read no characters or an error caught. */
    int success = getdelim(&text, &allocated, '\0', textFile);

    if (success == -1)
    {
        /* Encountered an error. */
        perror("Encountered error reading text file");
        exit(EXIT_FAILURE);
    }
    else
    {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    char *tableText = NULL;
    /* Reset allocated marker. */
    allocated = 0;
    success = getdelim(&tableText, &allocated, '\0', tableFile);

    if (success == -1)
    {
        /* Encountered an error. */
        perror("Encountered error reading table file");
        exit(EXIT_FAILURE);
    }
    else
    {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    /* Read term table first. */
    int allocatedColourTables = 0;
    /* Progress through string. */
    int progress = 0;
    /* Table string length. */
    int tableTextLength = strlen(tableText);
    char *lastToken = NULL;
    struct termColourTable *lastTable = NULL;

    while (progress < tableTextLength)
    {
        char *token = NULL;
        int score;
        int colour;
        int nextProgress;
        /* Make sure a token, colour and score are grabbed for each line. For simplicity, freshly allocate the token. */
        assert(sscanf(tableText + progress, "%m[^,],%d,%d %n", &token, &colour, &score, &nextProgress) == 3);
        assert(nextProgress > 0);
        progress += nextProgress;

        if (lastToken == NULL || strcmp(token, lastToken) != 0)
        {
            /* New token, so build new table and add it to problem. */
            // fprintf(stderr, "New token: %s (colour #%d) (%d)\n", token, colour, score);
            lastToken = token;

            if (termColourTableCount == 0)
            {
                /* Allocate initial. */
                colourTables = (struct termColourTable *)malloc(sizeof(struct termColourTable) * INITIALTERMS);
                assert(colourTables);
                allocatedColourTables = INITIALTERMS;
            }
            else
            {
                if ((termColourTableCount + 1) >= allocatedColourTables)
                {
                    /* Need more space for next table. */
                    colourTables = (struct termColourTable *)realloc(colourTables, sizeof(struct termColourTable) * allocatedColourTables * 2);
                    assert(colourTables);
                    allocatedColourTables = allocatedColourTables * 2;
                }
            }
            /* Set last table as fresh table. */
            lastTable = &(colourTables[termColourTableCount]);
            termColourTableCount++;
            /* Initialise table. */
            lastTable->term = token;
            lastTable->colourCount = 0;
            lastTable->colours = NULL;
            lastTable->scores = NULL;
        }
        else
        {
            /* Same as last token add info to new table. */
            // fprintf(stderr, "Same token: %s (colour #%d) (%d)\n", token, colour, score);
            /* Don't need token so free it. */
            free(token);
            token = NULL;
            /* Connect with existing token. */
            token = lastTable->term;
        }
        /* See if we need to increase the space for colours and scores for those colours. */
        if (lastTable->colourCount <= colour)
        {
            int lastCount = lastTable->colourCount;
            lastTable->colours = (int *)realloc(lastTable->colours, sizeof(int) * (colour + 1));
            assert(lastTable->colours);
            lastTable->scores = (int *)realloc(lastTable->scores, sizeof(int) * (colour + 1));
            assert(lastTable->scores);
            for (int i = lastCount; i < colour; i++)
            {
                lastTable->colours[i] = DEFAULTCOLOUR;
                lastTable->scores[i] = DEFAULTSCORE;
            }
            lastTable->colourCount = colour + 1;
        }
        /* Store info. */
        (lastTable->colours)[colour] = colour;
        (lastTable->scores)[colour] = score;

        /* Print new updated table */
        // fprintf(stderr, "Term: %s\n", lastTable->term);
        // fprintf(stderr, "Colour | Score\n");
        // for(int i = 0; i < lastTable->colourCount; i++){
        //     fprintf(stderr, "%6d | %d\n", lastTable->colours[i], lastTable->scores[i]);
        // }
    }

    /* Compress table to not have empty tables. */
    if (colourTables)
    {
        colourTables = (struct termColourTable *)realloc(colourTables, sizeof(struct termColourTable) * termColourTableCount);
        assert(colourTables);
        allocatedColourTables = termColourTableCount;
    }

    /* Done with tableText */
    if (tableText)
    {
        free(tableText);
    }

    /* Now split into terms */
    progress = 0;
    int termsAllocated = 0;
    int textLength = strlen(text);
    while (progress < textLength)
    {
        /* This does greedy term matching - this generally follows the specification
            but also allows for more complex cases (e.g. "Big Oh"). */
        char *nextTerm = NULL;
        int start;
        int maxLengthGreedyMatch = 0;
        int nextProgress;
        while (text[progress] != '\0' && !isalpha(text[progress]))
        {
            progress++;
        }
        start = progress;
        /* Calculate remaining character count to avoid edge case complications */
        int remChars = textLength - start;
        /* See if any of the terms in the table match. */
        for (int i = 0; i < termColourTableCount; i++)
        {
            char *term = colourTables[i].term;
            int termLen = strlen(term);
            if (termLen > remChars)
            {
                /* Not enough characters to fit term. */
                continue;
            }
            /* Check if word boundary. */
            if (!isalpha(text[progress + termLen]))
            {
                /* Yep! See if the term matches */
                int matching = 1;
                for (int j = 0; j < termLen; j++)
                {
                    if (tolower(text[progress + j]) != tolower(term[j]))
                    {
                        /* Non-match */
                        matching = 0;
                        break;
                    }
                }
                if (matching)
                {
                    /* Match, see if better than our current best. */
                    if (termLen > maxLengthGreedyMatch)
                    {
                        maxLengthGreedyMatch = termLen;
                        nextTerm = term;
                    }
                }
            }
        }
        if (!nextTerm)
        {
            /* No match found, try finding word. This may consume punctuation,
                this doesn't really matter. */
            assert(sscanf(text + start, " %ms %n", &nextTerm, &nextProgress) == 1);
            progress += nextProgress;
        }
        else
        {
            progress += maxLengthGreedyMatch;
            /* Move over punctuation if needed. */
            while (text[progress] != '\0' && !isalpha(text[progress]))
            {
                progress++;
            }
        }
        if (termsAllocated == 0)
        {
            terms = (char **)malloc(sizeof(char *) * INITIALTERMS);
            assert(terms);
            termsAllocated = INITIALTERMS;
        }
        else if (termCount >= termsAllocated)
        {
            terms = (char **)realloc(terms, sizeof(char *) * termsAllocated * 2);
            assert(terms);
            termsAllocated = termsAllocated * 2;
        }
        terms[termCount] = nextTerm;
        // fprintf(stderr, "(%s) ", nextTerm);
        termCount++;
    }
    // fprintf(stderr, "\n");

    p->termCount = termCount;
    p->text = text;
    p->terms = terms;

    p->termColourTableCount = termColourTableCount;
    p->colourTables = colourTables;

    p->part = PART_A;

    return p;
}

struct problem *readProblemB(FILE *textFile, FILE *tableFile,
                             FILE *transTable)
{
    /* Fill in Part A sections. */
    struct problem *p = readProblemA(textFile, tableFile);

    /* Fill in Part B sections. */
    p->colourTransitionTable = (struct colourTransitionTable *)malloc(sizeof(struct colourTransitionTable));
    assert(p->colourTransitionTable);
    int transitionCount = 0;
    int transitionAllocated = 0;
    int *prevColours = NULL;
    int *colours = NULL;
    int *scores = NULL;

    int prevColour;
    int colour;
    int score;

    while (fscanf(transTable, "%d,%d,%d ", &prevColour, &colour, &score) == 3)
    {
        if (transitionAllocated == 0)
        {
            prevColours = (int *)malloc(sizeof(int) * INITIALTRANSITIONS);
            assert(prevColours);
            colours = (int *)malloc(sizeof(int) * INITIALTRANSITIONS);
            assert(colours);
            scores = (int *)malloc(sizeof(int) * INITIALTRANSITIONS);
            assert(scores);
            transitionAllocated = INITIALTRANSITIONS;
        }
        else if (transitionCount >= transitionAllocated)
        {
            prevColours = (int *)realloc(prevColours, sizeof(int) * transitionAllocated * 2);
            assert(prevColours);
            colours = (int *)realloc(colours, sizeof(int) * transitionAllocated * 2);
            assert(colours);
            scores = (int *)realloc(scores, sizeof(int) * transitionAllocated * 2);
            assert(scores);
            transitionAllocated = transitionAllocated * 2;
        }
        prevColours[transitionCount] = prevColour;
        colours[transitionCount] = colour;
        scores[transitionCount] = score;
        transitionCount++;
    }

    p->colourTransitionTable->transitionCount = transitionCount;
    p->colourTransitionTable->prevColours = prevColours;
    p->colourTransitionTable->colours = colours;
    p->colourTransitionTable->scores = scores;

    p->part = PART_B;
    return p;
}

struct problem *readProblemE(FILE *textFile, FILE *tableFile,
                             FILE *transTable)
{
    /* Interpretation of inputs is same as Part B. */
    struct problem *p = readProblemB(textFile, tableFile, transTable);

    p->part = PART_E;
    return p;
}

struct problem *readProblemF(FILE *textFile, FILE *tableFile,
                             FILE *transTable)
{
    /* Interpretation of inputs is same as Part B. */
    struct problem *p = readProblemB(textFile, tableFile, transTable);

    p->part = PART_F;
    return p;
}

/*
    Outputs the given solution to the given file. If colourMode is 1, the
    sentence in the problem is coloured with the given solution colours.
*/
void outputProblem(struct problem *problem, struct solution *solution, FILE *stdout,
                   int colourMode)
{
    assert(problem->termCount == solution->termCount);
    if (!colourMode)
    {
        switch (problem->part)
        {
        case PART_A:
        case PART_B:
        case PART_F:
            for (int i = 0; i < problem->termCount; i++)
            {
                if (i != 0)
                {
                    printf(" ");
                }
                printf("%d", solution->termColours[i]);
            }
            printf("\n");
            break;

        case PART_E:
            printf("%d\n", solution->score);
            break;
        }
    }
    else
    {
        const char *(COLOURS_FG[]) = {"\033[38;5;0m", "\033[38;5;0m", "\033[38;5;0m", "\033[38;5;0m"};
        const char *(COLOURS_BG[]) = {"\033[48;5;231m", "\033[48;5;10m", "\033[48;5;11m", "\033[48;5;12m"};
        const char *COLOURS_FG_ERROR = "\033[38;5;1m";
        const char *ENDCODE = "\033[0m";
        const int colourCount = (int)(sizeof(COLOURS_FG) / sizeof(COLOURS_FG[0]));

        for (int i = 0; i < problem->termCount; i++)
        {
            if (i != 0)
            {
                printf(" ");
            }
            /* Place colour code */
            if (solution->termColours[i] < 0 || solution->termColours[i] >= colourCount)
            {
                printf("%s%s%s", COLOURS_FG_ERROR, problem->terms[i], ENDCODE);
            }
            else
            {
                printf("%s%s%s%s", COLOURS_FG[solution->termColours[i]], COLOURS_BG[solution->termColours[i]], problem->terms[i], ENDCODE);
            }
        }
        printf("\n");
    }
}

/*
    Frees the given solution and all memory allocated for it.
*/
void freeSolution(struct solution *solution, struct problem *problem)
{
    if (solution)
    {
        if (solution->termColours)
        {
            free(solution->termColours);
        }
        free(solution);
    }
}

/*
    Frees the given problem and all memory allocated for it.
*/
void freeProblem(struct problem *problem)
{
    if (problem)
    {
        /* Free terms. */
        for (int i = 0; i < problem->termCount; i++)
        {
            /* Don't free terms in colour table as we'll get them later. */
            int inTable = 0;
            for (int j = 0; j < problem->termColourTableCount; j++)
            {
                /* Note we do == because we care about the pointer not the contents. */
                if (problem->terms[i] == problem->colourTables[j].term)
                {
                    inTable = 1;
                    break;
                }
            }
            if (!inTable)
            {
                free(problem->terms[i]);
            }
        }
        if (problem->terms)
        {
            free(problem->terms);
        }

        for (int i = 0; i < problem->termColourTableCount; i++)
        {
            free(problem->colourTables[i].term);
            if (problem->colourTables[i].colours)
            {
                free(problem->colourTables[i].colours);
                free(problem->colourTables[i].scores);
            }
        }
        if (problem->colourTables)
        {
            free(problem->colourTables);
        }
        if (problem->colourTransitionTable)
        {
            free(problem->colourTransitionTable->prevColours);
            free(problem->colourTransitionTable->colours);
            free(problem->colourTransitionTable->scores);
            free(problem->colourTransitionTable);
        }
        if (problem->text)
        {
            free(problem->text);
        }
        free(problem);
    }
}

/* Sets up a solution for the given problem */
struct solution *newSolution(struct problem *problem)
{
    struct solution *s = (struct solution *)malloc(sizeof(struct solution));
    assert(s);
    s->termCount = problem->termCount;
    s->termColours = (int *)malloc(sizeof(int) * s->termCount);
    assert(s->termColours);
    for (int i = 0; i < s->termCount; i++)
    {
        s->termColours[i] = DEFAULTCOLOUR;
    }
    s->score = DEFAULTSCORE;
    return s;
}
/*
    Solves the given problem according to Part A's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemA(struct problem *p)
{
    struct solution *s = newSolution(p);
    /* Fill in: Part A */
    int score = 0;
    //Loop through each term in the text
    for (int i = 0; i < p->termCount; i++)
    {
        //Get the maximum colour and add it to the term colours
        s->termColours[i] = get_max_colour(p, i, p->terms[i], &score);
    }
    return s;
}
int get_max_colour(struct problem *p, int index, char *word, int *score)
{
    int colour = 0;
    //Loop through each term and find the matching term and find max
    for (int j = 0; j < p->termColourTableCount; j++)
    {
        if (p->terms[index] == p->colourTables[j].term)
        {
            int num_colors = p->colourTables[j].colourCount;
            int max = 0;
            for (int k = 0; k < num_colors; k++)
            {
                if (p->colourTables[j].scores[k] > max)
                {
                    //updating the max
                    max = p->colourTables[j].scores[k];
                    colour = p->colourTables[j].colours[k];
                }
            }
            *score += max;
            return colour;
        }
    }
    return colour;
}
int max(int score_a, int score_b, int a_colour, int b_colour)
{
    //Comparator function to call based on scores and colours
    if (score_a > score_b)
    {
        return a_colour;
    }
    else
    {
        return b_colour;
    }
}
struct solution *solveProblemB(struct problem *p)
{
    struct solution *s = newSolution(p);
    int score = 0;
    int prev_colour = -1;
    for (int i = 0; i < p->termCount; i++)
    {
        int max = 0;
        int term_no = -1;
        //Getting the term number of the current word
        for (int j = 0; j < p->termColourTableCount; j++)
        {
            if (p->colourTables[j].term == p->terms[i])
            {
                term_no = j;
                break;
            }
        }
        int colour = 0;
        //Looping through the colour tables and finding the max sum
        for (int j = 0; j < p->colourTables[term_no].colourCount; j++)
        {
            int transition_score = get_transition(p, i, prev_colour, j);
            // get the score for that term and this colour
            int colour_score = p->colourTables[term_no].scores[j];
            if (colour_score + transition_score > max)
            {
                max = colour_score + transition_score;
                colour = p->colourTables[term_no].colours[j];
            }
        }
        score += max;
        s->termColours[i] = colour;
        prev_colour = s->termColours[i];
    }
    s->score = score;
    return s;
}

int get_transition(struct problem *p, int index, int prev_colour, int curr_colour)
{
    //Getting the transition score given a prev and curr colour
    int max_colour = 0;
    for (int i = 0; i < p->colourTransitionTable->transitionCount; i++)
    {
        if (p->colourTransitionTable->prevColours[i] == prev_colour 
            && p->colourTransitionTable->colours[i] == curr_colour)
        {
            return p->colourTransitionTable->scores[i];
        }
    }
    return max_colour;
}
int is_term(struct problem *p, int index)
{
    //Helper function to find the term colour table index of a specific word
    for (int j = 0; j < p->termColourTableCount; j++)
    {
        if (p->colourTables[j].term == p->terms[index])
        {
            return j;
        }
    }
    return -1;
}
struct solution *solveProblemE(struct problem *p)
{
    struct solution *s = newSolution(p);
    int best_scores[p->termCount][4];
    int max_score = 0;
    int best_score;
    int prev_term = 0;
    for (int i = 0; i < p->termCount; i++)
    {
        //Get the term number
        int term_no = is_term(p, i);
        //Looping through each possible prev colour
        for (int j = 0; j < 4; j++)
        {
            best_score = -1;
            //Provision for the first word where best is just the word score
            if (i == 0)
            {
                best_score = p->colourTables[term_no].scores[j];
                best_score = (best_score < 0) ? 0 : best_score;
                best_scores[i][j] = best_score;
                continue;
            }
            //Finding the max score for each prev colour
            //Looping through each possible curr colour
            for (int k = 0; k < 4; k++)
            {
                int transition_score = get_transition(p, term_no, k, j);
                int colour_score = 0;
                int prev_score = best_scores[i - 1][k];
                colour_score = p->colourTables[term_no].scores[j];
                if (p->colourTables[prev_term].colours[k] == -1 
                    || (k > p->colourTables[prev_term].colourCount))
                {
                    transition_score = 0;
                    prev_score = 0;
                }
                if (colour_score == -1)
                {
                    colour_score = 0;
                    transition_score = 0;
                }
                int score = colour_score + transition_score + prev_score;
                //Getting the max 
                if (score > best_score)
                {
                    best_score = score;
                }
            }
            //Updating the matrix and ensuring that this colouring is valid 
            //again
            if (best_score > max_score && p->colourTables[term_no].colours[j] 
                != -1 && j < p->colourTables[term_no].colourCount)
            {
                max_score = best_score;
            }
            best_scores[i][j] = best_score;
        }
        prev_term = term_no;
    }
    s->score = max_score;
    return s;
}
struct solution *solveProblemF(struct problem *p)
{
    struct solution *s = newSolution(p);
    int best_scores[p->termCount][4];
    int max_score = 0;
    int best_score;
    int prev_term = 0;
    int colour;
    for (int i = 0; i < p->termCount; i++)
    {
        // Getting the term number of the current word in the term tables
        int term_no = is_term(p, i);
        for (int j = 0; j < 4; j++)
        {
            best_score = -1;
            if (i == 0)
            {
                best_score = p->colourTables[term_no].scores[j];
                best_score = (best_score < 0) ? 0 : best_score;
                best_scores[i][j] = best_score;
                continue;
            }
            for (int k = 0; k < 4; k++)
            {
                int transition_score = get_transition(p, term_no, k, j);
                int colour_score = 0;
                int prev_score = best_scores[i - 1][k];
                colour_score = p->colourTables[term_no].scores[j];
                // Making sure this colour is valid for this term
                if (p->colourTables[prev_term].colours[k] == -1 ||
                    (k > p->colourTables[prev_term].colourCount))
                {
                    transition_score = 0;
                    prev_score = 0;
                }
                if (colour_score == -1)
                {
                    colour_score = 0;
                    transition_score = 0;
                }
                int score = colour_score + transition_score + prev_score;
                if (score > best_score)
                {
                    best_score = score;
                }
            }
            //Reducing the score to find the new best and back tracking through
            //the bottom of the matrix.
            if (best_score > max_score && p->colourTables[term_no].colours[j] 
                != -1 && j < p->colourTables[term_no].colourCount)
            {
                max_score = best_score;
                colour = j;
            }
            best_scores[i][j] = best_score;
        }
        prev_term = term_no;
    }
    // for(int i = 0; i < p->termCount; i++){
    //     for(int j = 0; j < 4 ; j++){
    //         printf("%d   ", best_scores[i][j]);
    //     }
    //     printf("\n");
    // }
    int counter = p->termCount - 1;
    for (int i = p->termCount - 1; i > 0; i--)
    {
        int term_no = is_term(p, i);
        for (int j = 0; j < 4; j++)
        {
            // printf("score : %d\n", colour);
            if (p->colourTables[term_no].scores[colour] +
                    get_transition(p, term_no, j, colour) +
                    best_scores[i - 1][j] ==
                max_score)
            {
                s->termColours[counter] = colour;
                counter--;
                colour = j;
                max_score = best_scores[i - 1][j];
                // printf(" post : %d \n", best_scores[i - 1][j]);
                break;
            }
        }
    }
    s->termColours[0] = colour;
    return s;
}


/* share.c: Program for determining whether a given bar of chocolate can be broken evenly into a given
            number of pieces of specified sizes. Written by Peter Inslee, member of Team 8, for CS350
			"Algorithms and Complexity", taught by Dr. Bryant York at PSU, Winter 2012. Its a solution
			to "Problem J: Sharing Chocolate" (on page 19 of the included "2010WorldFinalsProblems.pdf"),
			one of the ACM International Collegiate Programming Contest 2010 World Finals problems, which
			are posted at "http://icpc.baylor.edu/ICPCWiki/Wiki.jsp?page=Problem%20Resources".

The figure below shows one way that a chocolate bar consisting of 3 x 4 pieces can be split	into 4 parts
that contain 6, 3, 2, and 1 pieces respectively, by breaking it 3 times. (This corresponds to the first
sample input provided below.)

    ####           ###  #  
    ####   ===>    ###  #
    ####          ## #  #

Input
The input consists of multiple test cases, each describing a chocolate bar to share. Each description
starts with a line containing a single integer n (1 ≤ n ≤ 15), the number of parts into which the bar is
supposed to be split. This is followed by a line containing two integers x and y (1 ≤ x, y ≤ 100), the
dimensions of the chocolate bar. The next line contains n positive integers, giving the number of pieces
that are supposed to be in each of the n parts.

The input is terminated by a line containing the integer zero.

Output
For each test case, first display its case number. Then display whether it is possible to break the
chocolate in the desired way: display "yes" if it is possible, and "no" otherwise; (this program goes one
step further and outputs "unsolved" if the input falls outside of acceptable ranges).

SAMPLE INPUT    OUTPUT FOR THE SAMPLE INPUT
4               Case 1: Yes
3 4             Case 2: No
6 3 2 1
2
2 3
1 5
0

***********************************************************************************************************
OVERALL ALGORITHM I USED
***********************************************************************************************************
* sort array from biggest to smallest when reading it from input
* pop biggest piece
* generate all possible rectangles of its size
* for each of the possible rectangles {
    * place it in top left corner of given bar
    * if it doesn't fit, return false;
    * generate all possible 2-partitions of remaining pieces in array
    * (i.e., given {5, 4, 4, 3, 1, 1}, generate pairs 	{{5},{4, 4, 3, 1, 1}}
														{{4},{5, 4, 3, 1, 1}}
														{{3},{5, 4, 4, 1, 1}}
														{{1},{5, 4, 4, 3, 1}}
														{{5, 4},{4, 3, 1, 1}}
														{{5, 3},{4, 4, 1, 1}}
														{{5, 1},{4, 4, 3, 1}}
														{{4, 4},{5, 3, 1, 1}}
														{{4, 3},{5, 4, 1, 1}}
														{{4, 1},{5, 4, 3, 1}}
														{{3, 1},{5, 4, 4, 1}}
														{{1, 1},{5, 4, 4, 3}}
														{{5, 4, 4},{3, 1, 1}}
														{{5, 4, 3},{4, 1, 1}}
														{{5, 4, 1},{4, 3, 1}}
														{{5, 3, 1},{4, 4, 1}}
														{{5, 1, 1},{4, 4, 3}}
														{{4, 4, 3},{5, 1, 1}}
														{{4, 4, 1},{5, 3, 1}}
														{{4, 3, 1},{5, 4, 1}}
														{{4, 1, 1},{5, 4, 3}}
														{{3, 1, 1},{5, 4, 4}}
    * if possible, break horizontally (this creates two new pieces, one containing the biggest piece)
        * recursive calls
        * if found solution, return true;
    * if possible, break vertically (this creates two new pieces, one containing the biggest piece)
        * recursive calls
        * if found solution, return true;
  }
* if after trying all of the possible rectangles, no solution has been found, return false;
***********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> // needed for 'sqrt()' and 'pow()', note that GCC requires '-lm' flag to use this

typedef enum {NO, YES , UNSOLVED} Result;
typedef struct PieceType
{
    int size;
    int flag;
} Piece;
typedef struct CaseType* CasePtr;
typedef struct CaseType
{
    int n;
    int x;
    int y;
    Piece* pieces;
    Result result;
    CasePtr next;
} Case;

void freeCases(CasePtr caseList)
{
    CasePtr next;

    while (caseList != NULL)
    {
        next = caseList->next;
        free(caseList->pieces);
        free(caseList);
        caseList = next;
    }
}

void writeOutput(CasePtr current)
{
    CasePtr next;
    int i = 1;
    FILE *fp;

    if ((fp = fopen("output.txt", "w")) == NULL)
    {
        fprintf(stderr, "Error: failed to write to 'output.txt'. Quitting...\n\n");
        exit(1);
    }

    while (current != NULL)
    {
        next = current->next;
        fprintf(fp, "Case %d: ", i++);
        switch (current->result)
        {
            case YES:   fprintf(fp, "yes\n");
                        break;
            case NO:    fprintf(fp, "no\n");
                        break;
            default:    fprintf(fp, "unsolved\n");
        }
        current = next;
    }
    fclose(fp);
}

int add(Piece* pieces, int n)
{
    int sum = 0;
    int i;

    for (i = 0; i < n; i++)
        sum += pieces[i].size;
    return sum;
}

void flipFlag(Piece* p)
{
    if (p->flag)
        p->flag = 0;
    else
        p->flag = 1;
}

Result solve(int barX, int barY, Piece* pieces, int n)
{
    if (n == 1) return YES; //base case

    int i, j, count1, count2, sum1, sum2;
    Piece* partition1 = (Piece*)calloc(n - 1, sizeof(Piece)); 
    Piece* partition2 = (Piece*)calloc(n - 1, sizeof(Piece));

    for (j = 0; j < n; j++) // initialize flags
        pieces[j].flag = 0;
 
    int possibilities = (int)pow(2, n - 1);

    for (i = 1; i < possibilities; i++) // generate all possible partitions of the pieces into two sets
    {
        for (j = 0; j < n; j++) // walk through entire array of pieces, flagging partition elements
            if (i % (int)pow(2, j) == 0) 
                flipFlag(&pieces[j]);

        count1 = 0;
        count2 = 0;
        sum1 = 0;
        sum2 = 0;

        for (j = 0; j < n; j++) // load partitions
            if (pieces[j].flag)
            {
                partition1[count1++] = pieces[j];
                sum1 += pieces[j].size;       
            }
            else
            {
                partition2[count2++] = pieces[j];
                sum2 += pieces[j].size;
            }

        if (   sum1 % barX == 0
            && sum2 % barX == 0
            && solve(barX, sum1 / barX, partition1, count1)
            && solve(barX, sum2 / barX, partition2, count2)) // recursive cases
        {
                free(partition1);
                free(partition2);
                return YES;
        }

        if (   sum1 % barY == 0
            && sum2 % barY == 0
            && solve(sum1 / barY, barY, partition1, count1)
            && solve(sum2 / barY, barY, partition2, count2)) // recursive cases
        {
                free(partition1);
                free(partition2);
                return YES;
        }
    }

    free(partition1);
    free(partition2);
    return NO; // after trying all of the possible rectangles, no solution has been found
}

void solveCases(CasePtr c)
{
    CasePtr next;

    while (c != NULL)
    {
        next = c->next;
        if (c->n > 15 || c->x > 100 || c->y > 100)
            c->result = UNSOLVED;
        else if (add(c->pieces, c->n) != c->x * c->y)
            c->result = NO; 
        else
            c->result = solve(c->x, c->y, c->pieces, c->n);
        c = next;
    }
}

void insertSorted(int new, int last, Piece* array)
{
    int i, j;

    for (i = 0; i <= last; i++)
        if (array[i].size < new) 
        {
            for (j = last; i <= j; j--)
                array[j + 1].size = array[j].size;
            array[i].size = new;
            break;
        }
}

CasePtr readInput(int argc, char** argv)
{
    FILE *fp;
    int i, newPiece;
    CasePtr c = (CasePtr)malloc(sizeof(Case));
    CasePtr head = c;
    CasePtr last = NULL;

    if (argc == 1) {
        printf("Please provide a file to use as input. Quitting...\n\n");
        exit(0);
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "%s: can't open '%s' for reading. Quitting...\n\n", argv[0], argv[1]);
        exit(1);
    }

    while (   fscanf(fp,    "%d\n",        &c->n) == 1
           && c->n != 0
           && fscanf(fp, "%d %d\n", &c->x, &c->y) == 2)
    {
        c->pieces = (Piece*)calloc(c->n, sizeof(Piece));

        for (i = 0; i < c->n && fscanf(fp, "%d", &newPiece) == 1; i++)
            insertSorted(newPiece, i, c->pieces);

        if (i != c->n)
        {
            fprintf(stderr, "%s: input file '%s' not formatted right. Quitting...\n\n", argv[0], argv[1]);
            exit(1);
        }

        c->next = (CasePtr)malloc(sizeof(Case));
        last = c;
        c = c->next;
    }

    if (last != NULL)
        last->next = NULL;
    else
        head = NULL;

    free(c);
    fclose(fp);
    return head;
}

int main(int argc, char** argv)
{
    CasePtr caseList = readInput(argc, argv);

    if (caseList != NULL)
    {
        solveCases(caseList);
        writeOutput(caseList);
        freeCases(caseList);
    }
    return 0;
}


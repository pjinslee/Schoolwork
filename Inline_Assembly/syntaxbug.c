// syntaxbug.c                 Peter Inslee                         12/1/07

// This code demonstrates an AT&T syntax bug in the floating-point
//  commands 'fsubp' and 'fdivp'. It affects all the non-commutative, 
//  two register floating-point commands which put their results in 
//  st(i!=0). Try running it in Insight, and pay special attention to the 
//  floating-point registers st(0), and st(1).

#include <stdio.h>

int main (void)
{
    double  try,
            tryagain,
            success;

    asm("   fldpi                   \n" // push pi onto fp reg stack
        "   fld1                    \n" // push +1.0 onto fp reg stack
        "   fsubp                   \n" // should subtract 1 from pi,
                                        //  store the result in st(1),
                                        //  and pop the fp reg stack
        "   fstpl   %[try]          \n" // store st(0) to 'try' and pop
                                        //  the fp reg stack

        "   fldpi                   \n" // push pi onto fp reg stack
        "   fld1                    \n" // push +1.0 onto fp reg stack
        "   fdivp                   \n" // should divide pi by 1, store
                                        //  the result in st(1), and pop
                                        //  the fp reg stack
        "   fstpl   %[tryagain]     \n" // store st(0) to 'tryagain' and
                                        //  pop the fp reg stack

        "   fldpi                   \n" // push pi onto fp reg stack
        "   fld1                    \n" // push +1.0 onto fp reg stack
        "   fdiv    %%st(1), %%st   \n" // should divide 1 by pi and
                                        //  store result in st(0)
        "   fstpl   %[success]      \n" // store st(0) to 'success'
                                        //  and pop the fp reg stack

        :   [try]       "=m"    (try),  // output
            [tryagain]  "=m"    (tryagain),
            [success]   "=m"    (success)
    );

    printf("pi - 1 = %16.14f  Not what I expected!\n", try);
    printf("pi/1 = %16.14f  Nor is this!\n", tryagain);
    printf("1/pi = %16.14f  ", success);
    printf("This version works because its result goes into st(0).\n");

    return 0;
}



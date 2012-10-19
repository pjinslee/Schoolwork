// starter file for Assignment 3xc2

#include <stdio.h>
#include <stdlib.h>

#define PRECISION   3

double powD (double n, double exp)
{
// do not change anything above this comment

// This function attempts to mimic the behavior of the C standard math library
//  function 'pow'. It accepts first a base 'n', and then an exponent 'exp' as
//  command line arguments. It returns n raised to the power of exp. Inputs
//  '0', 'inf', 'nan', and their negative counterparts are considered "special
//  cases" in the comments below. "Normal" is any other non-zero, numerical
//  value.

    double  power       = atof("nan"),
            infinity    = atof("inf");
    short   ctrlwd      = 0x0000,
            mskdwd      = 0x0000;
    
    asm("   movw    $0x0000, %%dx   \n" // "initialize" register %dx

        "   fldl    %[n]            \n" // load base 'n' into st(0)
        "   fxam                    \n" // examine n
        "   fnstsw  %%ax            \n" // load n's status word into %ax
        "   and     $0x4700, %%ax   \n" // mask all but C3, C2, C1, and C0
        "   movw    %%ax, %%cx      \n" // store copy of masked word in %cx

        "   and     $0x0500, %%ax   \n" // mask all but C2 and C0
        "   cmp     $0x0100, %%ax   \n" // is n NaN?
        "   je      NaN             \n" // if so, return 'nan'

        "   fldl    %[exp]          \n" // load exponent 'exp' into st(0)
        "   fxam                    \n" // examine exp
        "   fnstsw  %%ax            \n" // load exp's status word into %ax
        "   and     $0x4700, %%ax   \n" // mask all but C3, C2, C1, and C0
        "   shrw    $8, %%ax        \n" // move high end of masked word to %al
        "   or      %%cx, %%ax      \n" // combine masked status words,
                                        //  (n's in %ah, exp's in %al)
        "   movw    %%ax, %%cx      \n" // store copy of combined word in %cx

        "   and     $0x0705, %%ax   \n" // mask all but n:C2,C1,C0 & exp:C2,C0
        "   cmp     $0x0604, %%ax   \n" // is n negative normal, & exp normal?
        "   je      nNeg            \n" // if so, determine if exp is integer
        "   cmp     $0x0404, %%ax   \n" // is n positive normal, & exp normal?
        "   je      Calc            \n" // if so, calculate power

                                        // Special cases "fall through" here.

        "   cmp     $0x05, %%al     \n" // is exp (+ or -)inf?
        "   je      eInf            \n" // if so, determine sign and examine n
        "   cmp     $0x01, %%al     \n" // is exp (+ or -) NaN or 0?
        "   je      NaN             \n" // if exp is NaN, then power is 'nan'
        "   jl      One             \n" // if exp is 0, then power is +1.0

        "   fld1                    \n" // load +1.0 into st(0)
        "   fcomip  %%st(1), %%st   \n" // compare 1 and exp, and pop 1
        "   jbe     Retn            \n" // if 1 <= exp < inf, then power is n

        "   cmp     $0x07, %%ah     \n" // is n = -inf?
        "   jne     Next            \n" // jump to 'Next' if not
        "   fld1                    \n" // load +1.0 into st(0)
        "   fchs                    \n" // change +1.0 to -1.0
        "   fcomip  %%st(1), %%st   \n" // compare -1 and exp
        "   jae     Nzero           \n" // if exp <= -1, then power is -0.0

    "Next:  and     $0x0002, %%cx   \n" // mask all but exp:C1
        "   cmp     $0x0000, %%cx   \n" // is exp - or +?
        "   je      eInf            \n" // if 0 < exp < 1, then power is the
                                        //  same as if exp were +inf
        "   cmp     $0x05, %%ah     \n" // is n (+ or -)inf?
        "   je      Zero            \n" // if so, then power is +0.0
        "   cmp     $0x02, %%ah     \n" // is n (+ or -)0?
        "   jle     Ninf            \n" // if so, then power is '-inf'

    ////////////////////////////////////// Nothing "falls through" here.

    "eInf:  fxch                    \n" // load n into st(0), exp into st(1)
        "   fabs                    \n" // |n| in st(0)
        "   fld1                    \n" // load +1.0 into st(0)
        "   and     $0x0002, %%cx   \n" // mask all but exp:C1
        "   cmp     $0x0002, %%cx   \n" // is exp - or +?
        "   je      Rvrsd           \n" // if exp is -inf, results are reversed

        "   fcomi   %%st(1), %%st   \n" // compare 1 and |n|
        "   ja      Zero            \n" // if |n|<1, then power is +0.0
        "   jb      Inf             \n" // if |n|>1, then power is 'inf'
        "   jmp     NaN             \n" // if |n|=1, then power is 'nan'

    "Rvrsd: fcomi   %%st(1), %%st   \n" // compare 1 and |n|
        "   ja      Inf             \n" // if |n|<1, then power is 'inf'
        "   jb      Zero            \n" // if |n|>1, then power is +0.0
        "   jmp     NaN             \n" // if |n|=1, then power is 'nan'

                                        // Finished with special cases.
    ////////////////////////////////////// Nothing "falls through" here.

    "nNeg:  fld1                    \n" // load +1.0 into st(0)
        "   fld1                    \n" // +1.0 in st(0) and st(1)
        "   faddp   %%st, %%st(1)   \n" // 1.0 + 1.0 = 2.0, in st(0)
        "   fxch                    \n" // exp in st(0), 2.0 in st(1)
        "   fprem                   \n" // remainder of exp/2 in st(0)
        "   fabs                    \n" // |remainder| in st(0)
        "   fldz                    \n" // load +0.0 into st(0)
        "   fcomip  %%st(1), %%st   \n" // compare 0 and |remainder|, & pop 0
        "   je      Calc            \n" // if exp is even, calculate power

        "   fld1                    \n" // load +1.0 into st(0)
        "   fcomip  %%st(1), %%st   \n" // compare 1 and |remainder|, & pop 1
        "   sete    %%dl            \n" // set flag if exp is odd,
        "   je      Calc            \n" // then calculate power

        "   jmp     NaN             \n" // exp must be a non-integer

    ////////////////////////////////////// Nothing "falls through" here.

    "Calc:  fldl    %[exp]          \n" // load exp into st(0)
        "   fldl    %[n]            \n" // load n into st(0), exp in st(1)
        "   fabs                    \n" // |n| in st(0)
        "   fyl2x                   \n" // exp * log2(n) in st(0)
        "   fld     %%st            \n" // exp * log2(n) in st(0) and st(1)
        "   fstcw   %[ctrlwd]       \n" // store control word
        "   movw    $0x0c00, %%ax   \n" // put mask in %ax
        "   or      %[ctrlwd], %%ax \n" // masked control word in %ax
        "   movw    %%ax, %[mskdwd] \n" // store masked control word
        "   fldcw   %[mskdwd]       \n" // change rounding mode, (round to 0)
        "   frndint                 \n" // round (exp * log2(n)) to integer
        "   fldcw   %[ctrlwd]       \n" // restore control word
        "   fsubr   %%st(1), %%st   \n" // get remainder
        "   f2xm1                   \n" // (2^remainder)-1 in st(0)
        "   fld1                    \n" // load +1.0 into st(0)
        "   faddp                   \n" // (2^remainder) in st(0)
        "   fscale                  \n" // (2^remainder) * 2^(exp*log2(n)) 
                                        //  in st(0) (this is the result)

        "   cmp     $0x0, %%dl      \n" // check flag to see if exp was odd
        "   je      Done            \n" // if not, then done
        "   fldz                    \n" // load +0.0 into st(0)
        "   fcomip  %%st(1), %%st   \n" // compare 0 and result
        "   je      Zero            \n" // if result is 0, power is +0.0
        "   fchs                    \n" // if exp was odd, and n was negative,
                                        //  then change power's sign
        "   jmp     Done            \n" // done

    ////////////////////////////////////// Nothing "falls through" here.

    "Retn:  fldl    %[n]            \n" // load n into st(0)
        "   jmp     Done            \n" // done

    "Nzero: fldz                    \n" // load +0.0 into st(0)
        "   fchs                    \n" // -0.0 in st(0)
        "   jmp     Done            \n" // done

    "Zero:  fldz                    \n" // load +0.0 into st(0)
        "   jmp     Done            \n" // done

    "One:   fld1                    \n" // load +1.0 into st(0)
        "   jmp     Done            \n" // done

    "Ninf:  fldl    %[infinity]     \n" // load +inf into st(0)
        "   fchs                    \n" // -inf in st(0)
        "   jmp     Done            \n" // done

    "Inf:   fldl    %[infinity]     \n" // load +inf into st(0)
        "   jmp     Done            \n" // done

    "Done:  fstpl   %[power]        \n" // store answer in 'power'

    "NaN:   nop                     \n" // do nothing, power is NaN by default

        :   [power]     "=m"    (power)     // outputs
        :   [n]         "m"     (n),        // inputs
            [exp]       "m"     (exp),
            [infinity]  "m"     (infinity),
            [ctrlwd]    "m"     (ctrlwd),
            [mskdwd]    "m"     (mskdwd)
        :   "eax", "ecx", "edx"             // clobbers
    );

    return power;

// do not change anything below this comment, except for printing out your name
}

int main (int argc, char **argv)
{
    double  n = 0.0;
    double  exp = 0.0;

    printf("CS200 - Assignment 03 - Peter Inslee\n");
    if (argc > 1)
        n = atof(argv[1]);
    if (argc > 2)
        exp = atof(argv[2]);

    printf("%.*f to the %.*f = %.*f\n",
           PRECISION, n, PRECISION, exp, PRECISION, powD(n, exp));

    return 0;
}

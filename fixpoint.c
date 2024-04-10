#include "fixpoint.h"
#include <stdio.h>

/*
    Ref: http://www.claysturner.com/dsp/binarylogarithm.pdf
*/
fixpoint64_t fix_log2(fixpoint64_t n)
{
    if (n <= 0)
        return FIXED_MIN;
    fixpoint64_t result = 0;
    // Normalize n to 1 <= n < 2
    while (n < 1UL << SCALE_FACTOR) {
        n <<= 1;
        result -= FIXED_1;
    }

    while (n >= 2UL << SCALE_FACTOR) {
        n >>= 1;
        result += FIXED_1;
    }

    fixpoint64_t b = 1 << (SCALE_FACTOR - 1);
    for (int i = 0; i < SCALE_FACTOR; i++) {
        n = n * n >> SCALE_FACTOR;
        if (n >= 2UL << SCALE_FACTOR) {
            n >>= 1;
            result += b;
        }
        b >>= 1;
    }
    return result;
}

fixpoint64_t fix_log10(fixpoint64_t n)
{
    return (fix_log2(n) / FIXED_LOG2_10) << SCALE_FACTOR;
}

/*
    Digit by digit calculation
*/
fixpoint64_t fix_sqrt(fixpoint64_t n)
{
    if (n <= 1)
        return n;
    fixpoint64_t z = 0;
    for (fixpoint64_t m = 1UL << ((63 - __builtin_clzl(n)) & ~1UL); m;
         m >>= 2) {
        fixpoint64_t b = z + m;
        z >>= 1;
        if (n >= b)
            n -= b, z += m;
    }
    return z << (SCALE_FACTOR >> 1);
}
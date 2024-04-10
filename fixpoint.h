#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdint.h>

/*
  fixpoint64_t is 40 + 24 bits precision
  using two's complement
*/

typedef long fixpoint64_t;
#define SCALE_FACTOR 30

#define FIXED_1 (1UL << SCALE_FACTOR)
#define FIXED_MIN (1UL << 63)
#define FIXED_MAX ~FIXED_MIN
#define FIXED_INT(n) ((n) >> SCALE_FACTOR)
#define FIXED_FRAC(n) ((n) & ((1UL << SCALE_FACTOR) - 1))

#define FIXED_LOG2_10 fix_log2(0xaUL << SCALE_FACTOR)

/* Undefined when n <= 0 */
fixpoint64_t fix_log2(fixpoint64_t n);
fixpoint64_t fix_log10(fixpoint64_t n);
fixpoint64_t fix_sqrt(fixpoint64_t n);

#endif
#include <math.h>
#include <stdio.h>
#include "fixpoint.h"


double fix2double(fixpoint64_t n)
{
    return (double) n / (1UL << SCALE_FACTOR);
}

fixpoint64_t double2fix(double n)
{
    return (fixpoint64_t) round(n * (1UL << SCALE_FACTOR));
}

int main()
{
    FILE *f = fopen("fix.dat", "w");
    if (!f) {
        printf("Cannot open file\n");
        return -1;
    }

    double curr = 0.00001;
    double step = 0.001;
    fixpoint64_t fcurr = 0;
    fixpoint64_t fstep = double2fix(step);
    for (int i = 0; i < 10000000; i++) {
        fprintf(f, "%f %f ", curr, fix2double(fcurr));
        fprintf(f, "%f %f ", sqrt(curr), fix2double(fix_sqrt(fcurr)));
        fprintf(f, "%f %f ", log2(curr), fix2double(fix_log2(fcurr)));
        fprintf(f, "%f %f\n", log10(curr), fix2double(fix_log10(fcurr)));
        curr += step;
        fcurr += fstep;
    }
    fclose(f);
    return 0;
}
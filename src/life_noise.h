#ifndef LIFE_NOISE_H
#define LIFE_NOISE_H

#define LN_CNT 4
static char lnoise[LN_CNT] = {'|', '/', '-', '\\' };
#define LIFENOISE(n, var)   \
    if (verbose > 0) fprintf(stderr, "%c\b", lnoise[(var%LN_CNT)])

#endif /* LIFE_NOISE_H */

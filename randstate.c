#include "randstate.h"

#include <stdint.h>
#include <gmp.h>

// creating a extern variable
gmp_randstate_t state;

// Initializing the seed of the randstate and randstate itself
void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
}

// freeing up the memory given for the randstate.
void randstate_clear(void) {
    gmp_randclear(state);
}


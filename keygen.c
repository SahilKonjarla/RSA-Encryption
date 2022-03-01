#include "numtheory.h"
#include "randstate.h"
#include "rsa.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <gmp.h>
#include <sys/stat.h>
#include <stdbool.h>

#define OPTIONS "b:i:n:d:s:vh"

// prints out helper function
void helper(void) {
    printf("SYNOPSIS\n   Generates an RSA public/private key pair.\n\nUSAGE\n   ./keygen [-hv] [-b "
           "bits] -n pbfile -d pvfile\n\nOPTIONS\n   -h              Display program help and "
           "usage.\n   -v              Display verbose program output.\n   -b bits         Minimum "
           "bits needed for public key n (default: 256).\n   -i confidence   Miller-Rabin "
           "iterations for testing primes (default: 50).\n   -n pbfile       Public key file "
           "(default: rsa.pub).\n   -d pvfile       Private key file (default: rsa.priv).\n   -s "
           "seed         Random seed for testing.\n");
    return;
}

int main(int argc, char **argv) {
    // Initializing needed variables
    int opt, bits, iters, SEED;
    mpz_t p, q, e, d, m, s, n;
    mpz_inits(p, q, e, d, m, s, n, NULL);

    opt = 0;
    bits = 256;
    iters = 50;
    SEED = time(NULL);
    bool verb = false;
    char *pbpath = "rsa.pub";
    char *pvpath = "rsa.priv";
    char *user = "USER";
    char *username;
    FILE *pbfile;
    FILE *pvfile;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        // creating command line options
        switch (opt) {
        case 'b': bits = atoi(optarg); break;
        case 'i': iters = atoi(optarg); break;
        case 'n': pbpath = optarg; break;
        case 'd': pvpath = optarg; break;
        case 's': SEED = atoi(optarg); break;
        case 'v': verb = true; break;
        case 'h': helper(); return -1;
        }
    }

    // opening the pulic key file and private key file
    pbfile = fopen(pbpath, "w");
    pvfile = fopen(pvpath, "w");

    // if there is no public key or private key return error message
    if (pbfile == NULL) {
        printf("Error opening file.\n");
        return -1;
    } else if (pvfile == NULL) {
        printf("Error opening file.\n");
        return -1;
    }

    // change the file permissions and initialize the seed
    fchmod(fileno(pvfile), 0600);
    randstate_init(SEED);

    // create the public key and private key
    rsa_make_pub(p, q, n, e, bits, iters);
    rsa_make_priv(d, e, p, q);

    // get the username of the person running the function
    username = getenv(user);
    mpz_set_str(m, username, 62);
    rsa_sign(s, m, d, n);

    if (verb == true) {
        printf("user = %s\n", username);
        gmp_printf("s (%d bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_printf("p (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_printf("q (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("e (%d bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
        gmp_printf("d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // write the public key and the private key into the specified files
    rsa_write_pub(n, e, s, username, pbfile);
    rsa_write_priv(n, d, pvfile);

    // free up allocated memory for functions
    fclose(pbfile);
    fclose(pvfile);
    randstate_clear();
    mpz_clears(p, q, e, d, m, s, n, NULL);
    return 0;
}


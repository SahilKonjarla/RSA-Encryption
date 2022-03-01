#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <gmp.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#define OPTIONS "i:o:n:vh"

// This function will print out a helper message explaining the function
void helper(void) {
    printf("SYNOPSIS\n   Decrypts data using RSA decryption.\n   Encrypted data is encrypted by "
           "the encrypt program.\n\nUSAGE\n   ./decrypt [-hv] [-i infile] [-o outfile] -n "
           "privkey\n\nOPTIONS\n   -h              Display program help and usage.\n   -v          "
           "    Display verbose program output.\n   -i infile       Input file of data to decrypt "
           "(default: stdin).\n   -o outfile      Output file for decrypted data (default: "
           "stdout).\n   -n pvfile       Private key file (default: rsa.priv).\n");
    return;
}

int main(int argc, char **argv) {
    // Initializing the needed variables for the function
    int opt;
    mpz_t n, d;
    mpz_inits(n, d, NULL);
    FILE *infile = stdin; // specifying where the input is coming from
    FILE *outfile = stdout; // specifying where the output is coming from
    FILE *pvfile;
    char *pvkey = "rsa.priv"; // specifying the private key
    bool verb = false;

    opt = 0;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        // creating the command line arguements
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pvkey = optarg; break;
        case 'v': verb = true; break;
        case 'h': helper(); return -1;
        }
    }

    // opening the private key to get n and d
    pvfile = fopen(pvkey, "r");
    rsa_read_priv(n, d, pvfile);

    // checking if there even is a private key file
    if (pvfile == NULL) {
        printf("Error opening file.\n");
        return -1;
    }

    // if the command line argument is given print out n and d
    if (verb == true) {
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // decrypt the message taken from infile and put it out in outfile
    rsa_decrypt_file(infile, outfile, n, d);

    // free the memory of all the initializied variables
    fclose(pvfile);
    fclose(outfile);
    fclose(infile);
    mpz_clears(n, d, NULL);
    return 0;
}


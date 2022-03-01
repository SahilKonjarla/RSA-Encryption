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

// helper function that prints out a message that explains how to use the function
void helper(void) {
    printf("SYNOPSIS\n   Encrypts data using RSA encryption.\n   Encrypted data is decrypted by "
           "the decrypt program.\n\nUSAGE\n   ./encrypt [-hv] [-i infile] [-o outfile] -n "
           "pubkey\n\nOPTIONS\n   -h              Display program help and usage.\n   -v           "
           "   Display verbose program output.\n   -i infile       Input file of data to encrypt "
           "(default: stdin).\n   -o outfile      Output file for encrypted data (default: "
           "stdout).\n   -n pbfile       Public key file (default: rsa.pub).\n");
    return;
}

int main(int argc, char **argv) {
    // Initializing all the needed variables
    int opt;
    mpz_t m, n, e, s;
    mpz_inits(m, n, e, s, NULL);

    FILE *infile = stdin; // specifying where input is coming from
    FILE *outfile = stdout; // specifying where the output is coming from
    FILE *pbfile; // specifying the public key file
    char username[32];
    char *pbkey = "rsa.pub"; // including the public file name
    bool verb = false;

    opt = 0;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        // intitializing all the command line arguements
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pbkey = optarg; break;
        case 'v': verb = true; break;
        case 'h': helper(); return -1;
        }
    }

    // opening up the pubic key file to get n, e, s, and setting m to the username
    pbfile = fopen(pbkey, "r");
    rsa_read_pub(n, e, s, username, pbfile);
    mpz_set_str(m, username, 62);

    // if there is no public file print out error message
    if (pbfile == NULL) {
        printf("Error opening file.\n");
        return -1;
    }

    // if the public key doesnt match the signature of the messsage print out error message
    if (rsa_verify(m, s, e, n) == false) {
        printf("Error while verifying signature.\n");
        return -1;
    }

    // if the command lne argument is given print out the username, s, n, and e
    if (verb == true) {
        printf("user = %s\n", username);
        gmp_printf("s (%d bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("e (%d bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
    }

    // encrypt the message creating cypher text to be put out into the outfile
    rsa_encrypt_file(infile, outfile, n, e);

    // freeing the memory of all the initialzied variables
    fclose(pbfile);
    fclose(infile);
    fclose(outfile);
    mpz_clears(m, n, e, s, NULL);
    return 0;
}


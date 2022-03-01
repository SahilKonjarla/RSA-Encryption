#include "rsa.h"
#include "randstate.h"
#include "numtheory.h"

#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    // Initializing all variables needed for function
    uint64_t pbits, qbits;
    mpz_t temp, gcd1, phi, p1, q1;
    mpz_inits(temp, phi, gcd1, p1, q1, NULL);

    // creating the range for which the number of bits is created
    srand(time(NULL));

    // choosing the random number of bits for pbits
    pbits = (rand() % (nbits / 2)) + (nbits / 4);
    pbits += 1;
    qbits = nbits - pbits;
    qbits += 1;

    // creating the pimes p,q and initialzing n, as well as creating the totiant
    make_prime(p, pbits, iters);
    make_prime(q, qbits, iters);
    mpz_mul(n, p, q);
    mpz_sub_ui(p1, p, 1);
    mpz_sub_ui(q1, q, 1);

    mpz_mul(phi, p1, q1);

    // making a so that it is coprime to the totiant
    while (mpz_cmp_ui(gcd1, 1) != 0) {
        mpz_urandomb(temp, state, nbits);
        if (is_prime(temp, iters) == 1) {
            gcd(gcd1, phi, temp);
        }
    }
    mpz_set(e, temp);

    // clearing initialized variables and ending function
    mpz_clears(temp, phi, gcd1, p1, q1, NULL);
    return;
}

void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    // writing the elements of the public key into the public key file
    gmp_fprintf(pbfile, "%Zx\n", n);
    gmp_fprintf(pbfile, "%Zx\n", e);
    gmp_fprintf(pbfile, "%Zx\n", s);
    fprintf(pbfile, "%s\n", username);
    return;
}

void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    // reading in the elements of the public key from the public key file
    gmp_fscanf(pbfile, "%Zx\n", n);
    gmp_fscanf(pbfile, "%Zx\n", e);
    gmp_fscanf(pbfile, "%Zx\n", s);
    fscanf(pbfile, "%s\n", username);
    return;
}

void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    // initializing the the variables needed for the function
    mpz_t temp, q1, p1, phi;
    mpz_inits(temp, q1, p1, phi, NULL);

    // making the totiant whic is (p - 1)(q - 1)
    mpz_sub_ui(p1, p, 1);
    mpz_sub_ui(q1, q, 1);
    mpz_mul(phi, p1, q1);

    // getting d which is what is needed for the private key
    mod_inverse(d, e, phi);

    // clearing initialized varibales and ending the function
    mpz_clears(temp, q1, p1, phi, NULL);
    return;
}

void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    // writing the elements of the private key into the private key file
    gmp_fprintf(pvfile, "%Zx\n", n);
    gmp_fprintf(pvfile, "%Zx\n", d);
    return;
}

void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    // reading in the elements of the private key from the private key file
    gmp_fscanf(pvfile, "%Zx\n", n);
    gmp_fscanf(pvfile, "%Zx\n", d);
    return;
}

void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    // performing the encrypt formula
    pow_mod(c, m, e, n);
    return;
}

void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    // Initializing the variables needed for this function
    uint64_t k, j;
    mpz_t m, c;
    mpz_inits(m, c, NULL);
    k = 0;

    // making k = (log base 2 of (n) - 1) // 8
    k = (mpz_sizeinbase(n, 2) - 1) / 8;

    // creating the buffer
    uint8_t *array = calloc(k, sizeof(uint8_t));
    array[0] = 0xFF;

    // making the 0th element of the array equal to 0xFF
    // so there is way to get back to where we started
    while (feof(infile) == 0) {
        j = fread(&array[1], sizeof(uint8_t), k - 1, infile);
        // importing the message
        mpz_import(m, j + 1, 1, sizeof(uint8_t), 1, 0, array);
        // encrypting the message and writing it into the outfile
        rsa_encrypt(c, m, e, n);
        gmp_fprintf(outfile, "%Zx\n", c);
    }

    // clearing the initialzied variables and buffer
    free(array);
    mpz_clears(m, c, NULL);
    return;
}

void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    // performing the decrypt formula
    pow_mod(m, c, d, n);
    return;
}

void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    // Initializing variables needed for this function
    uint64_t k, j;
    mpz_t m, c;
    mpz_inits(m, c, NULL);

    // making k = (log base 2 of (n) - 1) // 8
    k = (mpz_sizeinbase(n, 2) - 1) / 8;

    // creating the buffer
    uint8_t *array = calloc(k, sizeof(uint8_t));

    // looping through the file and decyrpting each block then exporting that file
    // then writing it into the outfile
    while (gmp_fscanf(infile, "%Zx\n", c) > 0) {
        rsa_decrypt(m, c, d, n);
        mpz_export(array, &j, 1, sizeof(uint8_t), 1, 0, m);
        fwrite(&array[1], sizeof(uint8_t), j - 1, outfile);
    }

    // clearing the initialzied variables and buffer
    free(array);
    mpz_clears(m, c, NULL);
    return;
}

void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    // creating a signature s to be verfied using the private key d
    pow_mod(s, m, d, n);
    return;
}

bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    // Initialized variables
    mpz_t t;
    mpz_init(t);

    // computed the power mod putting the message into t
    pow_mod(t, s, e, n);
    // checking if the message and t are the same if so return true if not return false
    if (mpz_cmp(t, m) == 0) {
        mpz_clear(t);
        return true;
    } else {
        mpz_clear(t);
        return false;
    }
}


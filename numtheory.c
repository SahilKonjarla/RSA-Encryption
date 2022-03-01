#include "numtheory.h"
#include "randstate.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>

void gcd(mpz_t d, mpz_t a, mpz_t b) {
    // Initializing the variables needed
    mpz_t alpha, beta, temp, mod;
    mpz_inits(alpha, beta, temp, mod, NULL);

    mpz_set(alpha, a);
    mpz_set(beta, b);

    // this is the loop that is going to see if the gcd is found
    while (mpz_cmp_ui(beta, 0) != 0) {
        mpz_set(temp, beta);
        mpz_mod(mod, alpha, beta);
        mpz_set(beta, mod);
        mpz_set(alpha, temp);
    }
    // when the gcd is found is it going to return the gcd through d
    mpz_set(d, alpha);

    // clears the initlaized variables and ends the function
    mpz_clears(alpha, beta, temp, mod, NULL);
    return;
}

void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {
    // Initizlizing the variables needed and setting the variables to specified values
    mpz_t t, t2, r, r2, q, temp, temp2;
    mpz_inits(t, t2, r, r2, q, temp, temp2, NULL);

    mpz_set(r, n);
    mpz_set(r2, a);
    mpz_set_ui(t, 0);
    mpz_set_ui(t2, 1);

    while (mpz_cmp_ui(r2, 0) != 0) {
        mpz_fdiv_q(q, r, r2); // going to create q which is r/r2

        mpz_set(temp, r);
        mpz_set(r, r2); // setting r equal to r2
        mpz_mul(r2, q, r2);
        mpz_sub(r2, temp, r2); // then setting r2 equal to r - q x r2

        mpz_set(temp2, t);
        mpz_set(t, t2); // setting t equal to t2
        mpz_mul(t2, q, t2);
        mpz_sub(t2, temp2, t2); // then setting t2 equal to t - q x t2
    }

    if (mpz_cmp_ui(r, 1) > 0) { // checking if r is greater than 1
        mpz_set_ui(i, 0); // if so return no inverse
        mpz_clears(t, t2, r, r2, q, temp, NULL);
        return;
    }

    if (mpz_sgn(t) == -1) { // checking if t is less than 0
        mpz_add(t, t, n); // if so add n to t
    }
    mpz_set(i, t); // return the mod inverse through i

    mpz_clears(t, t2, r, r2, q, temp, temp2, NULL); // clear initialized variables and end fucntion
    return;
}

void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) {
    // Initalizing variables needed for function
    mpz_t v, p, e, temp, mod;
    mpz_inits(v, p, e, temp, mod, NULL);

    mpz_set_ui(v, 1);
    mpz_set(p, base);
    mpz_set(e, exponent);
    mpz_set(mod, modulus);

    while (mpz_cmp_ui(e, 0) > 0) { // seeing if the exponent given is greater than 0
        if (mpz_odd_p(e) != 0) { // if e is a odd number
            mpz_mul(v, v, p);
            mpz_mod(v, v, mod); // make v equal to v x p mod n
        }
        mpz_mul(temp, p, p);
        mpz_mod(p, temp, mod); // setting the base to base x base mod n

        mpz_fdiv_q_ui(e, e, 2); // dividing the exponent by 2
    }
    mpz_set(out, v); // returning the power mod therough out

    mpz_clears(v, p, e, temp, mod, NULL); // clearing initialized variables and ending the fucntion
    return;
}

bool is_prime(mpz_t n, uint64_t iters) {
    // Initialziing and setting the proper variables to the values needed
    uint64_t s = 0;
    uint64_t j;
    mpz_t r, temp, n1, n2, base, y, a;
    mpz_inits(r, temp, n1, n2, base, y, a, NULL);

    mpz_set_ui(base, 2);
    mpz_sub_ui(n1, n, 1);
    mpz_sub_ui(n2, n, 3);

    if (mpz_cmp_ui(n, 0) == 0) {
        mpz_clears(r, temp, n1, n2, base, y, a, NULL);
        return false;
    } else if (mpz_cmp_ui(n, 1) == 0) {
        mpz_clears(r, temp, n1, n2, base, y, a, NULL);
        return false;
    } else if (mpz_cmp_ui(n, 2) == 0) {
        mpz_clears(r, temp, n1, n2, base, y, a, NULL);
        return true;
    } else if (mpz_cmp_ui(n, 3) == 0) {
        mpz_clears(r, temp, n1, n2, base, y, a, NULL);
        return true;
    }

    // finding an r so that it is an odd number
    while (mpz_cmp_ui(temp, 1) != 0) {
        s += 1;
        mpz_pow_ui(temp, base, s);
        mpz_cdiv_q(r, n1, temp);
        mpz_mod_ui(temp, r, 2);
    }
    s -= 1;

    // going to run this program for a specified number of iterations
    for (uint64_t i = 1; i <= iters; i += 1) {
        mpz_urandomm(a, state, n2); // choosing a random base from a range of [2, n-1]
        mpz_add_ui(a, a, 2);
        pow_mod(y, a, r, n);
        // checking if the pow_mod does not qual 1 or n-1
        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, n1) != 0) {
            j = 1;
            // going to now check if the number is prime or not
            while (j <= s && mpz_cmp(y, n1) != 0) {
                pow_mod(y, y, base, n);
                // checking if y is equal to 1 if so n is not a prime number
                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(r, temp, n1, n2, base, y, a, NULL);
                    return false;
                }
                j += 1;
            }
            // checking if y is equal to n - 1 if so n is not a prime number
            if (mpz_cmp(y, n1) != 0) {
                mpz_clears(r, temp, n1, n2, base, y, a, NULL);
                return false;
            }
        }
    }
    // if n passes all tests then it is a prime number
    mpz_clears(r, temp, n1, n2, base, y, a, NULL);
    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    // Initializing all the neccesary variables needed
    mpz_t size, prime;
    mpz_inits(size, prime, NULL);

    // getting the number of bits needed to create the new prime
    mpz_ui_pow_ui(size, 2, bits);
    mpz_urandomm(prime, state, size);
    mpz_add(prime, prime, size);

    // is the number created is not prime create a new r
    while (!is_prime(prime, iters)) {
        mpz_urandomm(prime, state, size);
        mpz_add(prime, prime, size);
    }
    // set p to the prime number created
    mpz_set(p, prime);

    // clear the intialzied varaibles and end the function
    mpz_clears(prime, size, NULL);
    return;
}


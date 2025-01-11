#include <math.h>
#include <stdio.h>
#include <stdint.h> // For fixed-width integer types like int64_t
#include <stdlib.h>


#if 0
// GCD Function
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}


int lcm(int a, int b) {
    return (a / gcd(a, b)) * b;
}
#endif

#define EPSILON (1.E-4)

/*
int is_integer(float v)
{
    float f = v - floor(v);
    int r =  (f<=EPSILON) ? 1 : 0;
    //printf("is integer: f=%f -> %d\n", f, r);
    return r;
}
*/
    
void calculate(float fs, float f, int maxn)
{
    int P;
    float minnpf = 1.0;
    int minisup = 0;
    int bestp = 0;
    float N = fs/f;
    for (P=1; P*N<maxn; P++) {
        float np = N*P;
        float npf = np - floor(np);
        printf("test %d -> %f npf=%f %f\n", P, np, npf, 1.0-npf);
        if (npf < EPSILON) {
            printf("P=%d N=%f N*P=%f\n", P, N, N*P);
            return;
        }
        if (npf <= minnpf) {
            minnpf = npf;
            bestp = P;
            minisup = 0;
        }
        if (1.0-npf <= minnpf) {
            minnpf = 1.0-npf;
            bestp = P;
            minisup = 1;
        }
    }
    printf("bestp %d %f %s\n", bestp, minnpf, minisup ? "up" : "dn");
    // N = fs/f   f = fs/N
    int npa = floor(N*bestp);
    if (minisup) npa++;
    float fadj = bestp*fs/npa;
    printf("adj %f\n", fadj);

}

// Main function (example usage)
int main(int argc, char **argv) 
{
    if (argc != 2) {
        fprintf(stderr, "usage %s freq\n", argv[0]);
        exit(1);
    }
    float f = atof(argv[1]);
    if (!f) {
        fprintf(stderr, "not a float :  %s (freq)\n", argv[1]);
        exit(1);
    }

    float fs = 50000.0;   // Sampling frequency
    //float f = 3000.0;     // Desired sine wave frequency
    int maxbuf = 1024;    // Buffer size

   calculate(fs, f, maxbuf);
    return 0;
}


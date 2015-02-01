#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#define MAX_STRIDE  (4096/sizeof(double))
#define LLC_SIZE    (20 * 1024 * 1024)
#define ITERATIONS  (1000)

void triad(double *, double *, double *, double *, int, int);

uint64_t get_time(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

int main(int argc, char *argv[])
{
    uint64_t start, stop;

    // allocate memory
    double *A, *B, *C, *D;

    ssize_t effective_llc = LLC_SIZE / 3;       // to be on the safe side
    ssize_t num_bytes = effective_llc / 4;      // four arrays
    ssize_t N = num_bytes / sizeof(double);
    if ((A = (double *)malloc(N * sizeof(double))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    if ((B = (double *)malloc(N * sizeof(double))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    if ((C = (double *)malloc(N * sizeof(double))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    if ((D = (double *)malloc(N * sizeof(double))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "LLC Size: %.2fB\t%.2fKiB\t%.2fMiB\n", (float)LLC_SIZE, LLC_SIZE / 1024.0f, LLC_SIZE / (1024.0f * 1024.0f));
    fprintf(stderr, "Allocated 4x%zuB=%.2fB\t%.2fKiB\t%.2fMiB\n", num_bytes, (float)4*num_bytes, (float)4*num_bytes / 1024.0f, (float)4*num_bytes / (1024.0f * 1024.0f));

    // initialize memory
    for (ssize_t i=0; i<N; ++i) {
        A[i] = 0.0f;
        B[i] = 1.0f;
        C[i] = 2.0f;
        D[i] = 3.0f;
    }

    fprintf(stderr, "stride\tnet bandwidth [GiB/s]\n");
    double bw, old_bw;
    for (int s = 1; s < MAX_STRIDE; s *= 2) {
        start = get_time();
        for (int i = 0; i < ITERATIONS; ++i)
            triad(A, B, C, D, s, N);
        stop = get_time();

        uint64_t runtime_us = stop-start;
        double runtime_s = (double)runtime_us / (1000 * 1000);
        bw = (double)(N*5/s) * ITERATIONS * sizeof(double) / runtime_s / (1024 * 1024 * 1024);

        // check result and reset
        int error = 0;
        for (ssize_t i=0; i<N; i += s) {
            if (A[i] != 7.0f) {
                printf("unexpected error!\n");
                exit(EXIT_FAILURE);
            }
        }

        if  (s == 1)
            printf("%d\t%.2f\n", s, bw);
        else
            printf("%d\t%.2f\t# factor %.2f slower\n", s, bw, old_bw/bw);

        old_bw = bw;
    }

    return 0;
}

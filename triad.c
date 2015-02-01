void triad(double *A, double *B, double *C, double *D, int s, int N)
{
#pragma novector
    for (int i = 0; i < N; i += s) {
        A[i] = B[i] + C[i] * D[i];
    }
}

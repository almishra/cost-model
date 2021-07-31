/**
 * This is a parallel code with collapsed loops
 */

#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

void initialize(int n, int m, double alpha, double f[m][n])
{
    int i, j;
    /* Initilize initial condition*/
    for (i=0; i<m; i++){
        for (j=0; j<n; j++){
            f[j][i] = (rand()%3) * alpha;
        }
    }
}

#define N 9000
#define M 9000
#define N1 8999
#define M1 8999
int main(int argc, char**argv) {
    int iter_max = 5000;
    int iter;
    double err = 1.0;
    double tol = 0.0000000001;
    //int m = (argc > 1 ? atoi(argv[1]) : 500);
    //int n = (argc > 1 ? atoi(argv[1]) : 500);
//    int N = 500;
//    int M = 500;
    double A[M][N];
    double Anew[M][N];
    double alpha = 0.0543;
    initialize(N, M, alpha, A);

    int i, j;
    struct timeval t1, t2;

    //gettimeofday(&t1, NULL);
    iter = 0;
//    N--;
//    M--;

    double runtime;
    #pragma omp target data map(alloc:Anew[0:M][0:N]) map(A[0:M][0:N])
    while (err>tol && iter<iter_max) {
        err=0.0;
        gettimeofday(&t1, NULL);
        #pragma omp target teams distribute parallel for collapse(2) reduction(max:err) map(err, Anew[0:M][0:N], A[0:M][0:N])
        for(i=1; i<N1; i++) {
            for( j = 1; j < M1; j++) {
                Anew[i][j] = 0.25 * (A[i][j+1] + A[i][j-1] + A[i-1][j] + A[i+1][j]);
                if(err < fabs(Anew[i][j] - A[i][j]))
                    err = fabs(Anew[i][j] - A[i][j]);
            }
        }
        gettimeofday(&t2, NULL);
        runtime = (t2.tv_sec - t1.tv_sec);
        runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;
        printf("%.3f %d\n", runtime, iter);

        gettimeofday(&t1, NULL);
        #pragma omp target teams distribute parallel for collapse(2) map(Anew[0:M][0:N], A[0:M][0:N])
        for(i=1; i<N1; i++) {
            for(j=1; j<M1; j++) {
                A[i][j] = Anew[i][j];
            }
        }
        gettimeofday(&t2, NULL);
        runtime = (t2.tv_sec - t1.tv_sec);
        runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;
        printf("%.3f %d\n", runtime, iter);
        iter++;
    }
    //gettimeofday(&t2, NULL);
    //double runtime = (t2.tv_sec - t1.tv_sec);
    //runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;

    //printf("%.3f %d\n", runtime, iter);

    return 0;
}

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

//#define N 100
//#define M 100
//#define N1 99
//#define M1 99
int main(int argc, char**argv) {
    int iter_max = 20;
    int iter;
    double err = 1.0;
    double tol = 0.0000000001;

    int dev = 0;
    if(argc > 1)
      dev = atoi(argv[1]);

    double A[M][N];
    double Anew[M][N];
    double alpha = 0.0543;
    initialize(N, M, alpha, A);

    int i, j;
    struct timeval t1, t2;

    iter = 0;

    double runtime;
    #pragma omp target data map(alloc:Anew[0:M][0:N]) map(A[0:M][0:N]) device(dev)
    while (err>tol && iter<iter_max) {
        err=0.0;
        gettimeofday(&t1, NULL);
        //#pragma omp target teams distribute parallel for collapse(2) reduction(max:err) map(err, Anew[0:M][0:N], A[0:M][0:N])
        #pragma omp target teams distribute parallel for collapse(2) reduction(max:err) map(err) device(dev)
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
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.3f %d\n", N1, M1,(N1-1)*(M1-1),0,31,4,1,8,8,4,5,1,0,0,2,runtime, iter);

        gettimeofday(&t1, NULL);
        //#pragma omp target teams distribute parallel for collapse(2) map(Anew[0:M][0:N], A[0:M][0:N])
        #pragma omp target teams distribute parallel for collapse(2) device(dev) 
        for(i=1; i<N1; i++)
            for(j=1; j<M1; j++)
                A[i][j] = Anew[i][j];

        gettimeofday(&t2, NULL);
        runtime = (t2.tv_sec - t1.tv_sec);
        runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.3f %d\n", N1, M1,(N1-1)*(M1-1),0,6,0,0,0,0,0,0,0,0,0,1,runtime, iter);
        fflush(stdout);
        iter++;
    }

    return 0;
}

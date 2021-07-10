#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>

#define N 7100

void gen_matrix(double* A)
{
#pragma omp parallel for
  for (int i = 0; i < N; i++) 
    for (int j = 0; j < N; j++) 
      A[i*N + j] = (i * j) % 10;
}

int main(int argc, char **argv)
{
  double A[N*N], B[N*N], C[N*N];
  struct timeval  tv1, tv2;

#pragma omp parallel for
  for (int i = 0; i < N; i++) 
    for (int j = 0; j < N; j++) {
      A[i*N + j] = rand() % 10;
      B[i*N + j] = rand() % 10;
    }

  printf("Starting...\n");
//  for(int n=50; n<=N; n+=250) {
//    for(int m=40; m<=N; m+=400) {
//      for(int o=40; o<=N; o+=300) {
  for(int n=6800; n<=6800; n++) {
    for(int m=50; m<=4050; m+=400) {
      for(int o=6940; o<=6940; o+=400) {
        gettimeofday(&tv1, NULL);
#pragma omp parallel for collapse(2)
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < o; j++) {
            double sum = 0.0;
            for (int k = 0; k < m; k++)
              sum = sum + A[i * m + k] * B[k * o + j];
            C[i * o + j] = sum;
          }
        }
        gettimeofday(&tv2, NULL);
      
        printf("%d,%d,%d,%d,%.0f\n", n,m,m,o,
            (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
        fflush(stdout);
      }
    }
  }

  return 0;
}


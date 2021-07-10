#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

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
  char output[100];
  if(argc > 1)
    strcpy(output, argv[1]);
  else
    strcpy(output, "output.log");

  FILE *fp = fopen(output, "w");
  double A[N*N], B[N*N], C[N*N];
  struct timeval  tv1, tv2;

#pragma omp parallel for
  for (int i = 0; i < N; i++) 
    for (int j = 0; j < N; j++) {
      A[i*N + j] = rand() % 10;
      B[i*N + j] = rand() % 10;
    }

  int dev = omp_get_num_devices();

  if(dev > 0) {
    omp_set_num_threads(dev);
    for(int i=0; i<dev; i++) {
#pragma omp target device(i)
      { }
    }

    printf("Starting...\n");
#pragma omp parallel for
    for(int n=50; n<=N; n+=250) {
      for(int m=50; m<=N; m+=400) {
        for(int o=40; o<=N; o+=300) {
          double time[3];
          for(int count = 0; count < 3; count++) {
            gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for collapse(2) map(A[0:n*m], B[0:m*o], C[0:n*o]) device(omp_get_thread_num())
            for (int i = 0; i < n; i++) {
              for (int j = 0; j < o; j++) {
                double sum = 0.0;
                for (int k = 0; k < m; k++)
                  sum = sum + A[i * m + k] * B[k * o + j];
                C[i * o + j] = sum;
              }
            }
            gettimeofday(&tv2, NULL);
            time[count] = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
        
            //printf("%d,%d,%d,%d,%.0f\n", n,m,m,o,
            //    (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
            //fflush(stdout);
          }
#pragma omp critical
          {
          for(int count = 0; count < 3; count++) {
            fprintf(fp, "%d,%d,%d,%d,%.0f\n", n,m,m,o,time[count]);
          }
          fflush(fp);
          }
        }
      }
    }
  } else {
    printf("No device found\n");
  }
  fclose(fp);
  return 0;
}


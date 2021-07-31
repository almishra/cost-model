#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include "cuda_runtime.h"

#define N 7100

int main(int argc, char **argv)
{
  char output[100];
  if(argc > 1)
    strcpy(output, argv[1]);
  else
    strcpy(output, "output.log");

  FILE *fp = fopen(output, "w");
  double A[N*N], B[N*N], C[N*N], D[N*N];
  struct timeval  tv1, tv2;

#pragma omp parallel for
  for (int i = 0; i < N; i++) 
    for (int j = 0; j < N; j++) {
      A[i*N + j] = rand() % 10;
      B[i*N + j] = rand() % 10;
    }

  int dev = omp_get_num_devices();

  if(dev > 0) {
    //omp_set_num_threads(dev);
    for(int i=0; i<dev; i++) {
#pragma omp target device(i)
      { }
    }

    printf("Starting...\n");
    //int n=6800, o=3040;
    //int m = 7000;

//    for (int n = 6300; n >= 100; n-=400) {
//    for (int o = 7100; o >= 100; o-=400) {
    for (int n = 5500; n >= 100; n-=400) {
    for (int o = 4300; o >= 100; o-=400) {
    for (int m = 500; m <= 7100; m+=400) {
      printf("Generating D\n");
      gettimeofday(&tv1, NULL);
#pragma omp parallel for
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < o; j++) {
          double sum = 0.0;
          for (int k = 0; k < m; k++)
            sum = sum + A[i * m + k] * B[k * o + j];
          D[i * o + j] = sum;
        }
      }
      gettimeofday(&tv2, NULL);
      fprintf(fp, "%d,%d,%d,%.0f", n, o, m, (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
      //fprintf(fp, "CPU - %d,%d,%d,%d,%.0f\n", n,m,m,o,(double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);

      //double time;
      double time[dev];
//      int devCount;
//      cudaGetDeviceCount(&devCount);
//      printf("%d\n", devCount);
#pragma omp parallel for
      for(int count = 0; count < dev; count++) {
//#pragma omp critical
//        printf("Flusing GPU %d\n", count);
       // cudaDeviceReset();
//        printf("Generating C for %d\n", count);
        gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for collapse(2) map(A[0:n*m], B[0:m*o], C[0:n*o]) device(omp_get_thread_num())
//#pragma omp target teams distribute parallel for collapse(2) map(A[0:n*m], B[0:m*o], C[0:n*o])
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
        //time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
      
        for(int i=0; i<n; i++)
          for(int j=0; j<o; j++)
            if(C[i * o + j] != D[i * o + j]) {
              printf("Error m=%d, i=%d, j=%d\n", m, i, j);
              fflush(stdout);
              fclose(fp);
              exit(-1);
            }
      }
//#pragma omp critical
//      {
        //fprintf(fp, "%d,%d,%d,%d,%.0f\n", n,m,m,o,time);
      for(int count = 0; count < dev; count++) {
        fprintf(fp, ",%.0f", time[count]);
//        fflush(fp);
      }
    //}
    fprintf(fp, "\n");
    fflush(fp);
  }
    }
    }
  } else {
    printf("No device found\n");
  }
  fclose(fp);
  return 0;
}


#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include "cuda_runtime.h"

#define N 3000

int main(int argc, char **argv)
{
  double A[N*N], B[N*N], C[N*N], C1[N*N], D[N*N];
  struct timeval  tv1, tv2;

#pragma omp parallel for
  for (int i = 0; i < N; i++) 
    for (int j = 0; j < N; j++) {
      A[i*N + j] = rand() % 10;
      B[i*N + j] = rand() % 10;
      C[i*N + j] = rand() % 10;
    }

#pragma omp target
  { }

  gettimeofday(&tv1, NULL);
#pragma omp parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("CPU - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target 
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target 
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU target - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
#pragma omp target teams
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU target teams - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU distribute - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU distribute parallel for - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for
  for (int j = 0; j < N; j++) {
    for (int i = 0; i < N; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute parallel for
  for (int j = 0; j < N; j++) {
    for (int i = 0; i < N; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU distribute parallel for swap - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute
  for (int i = 0; i < N; i++) {
#pragma omp parallel for
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute
  for (int i = 0; i < N; i++) {
#pragma omp parallel for
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU distribute inner parallel for - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute
  for (int j = 0; j < N; j++) {
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute
  for (int j = 0; j < N; j++) {
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU distribute inner parallel for swap - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for collapse(2) 
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute parallel for collapse(2) 
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU collapse - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  gettimeofday(&tv1, NULL);
#pragma omp target data map(to: A[0:N*N], B[0:N*N], C[0:N*N]) map(alloc: C1[0:N*N]) map(from:D[0:N*N])
  {
#pragma omp target teams distribute parallel for collapse(2)
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * N + j];
      C1[i * N + j] = sum;
    }
  }
#pragma omp target teams distribute parallel for collapse(2)
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + C1[i * N + k] * C[k * N + j];
      D[i * N + j] = sum;
    }
  }
  }
  gettimeofday(&tv2, NULL);
  printf("GPU collapse map - %.0f\n", (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000);
  fflush(stdout);

  return 0;
}


#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

int main(int argc, char **argv)
{
  int M = atoi(argv[1]);
  int N = atoi(argv[2]);
  int O = atoi(argv[3]);

  double *A = (double*) malloc(sizeof(double) * M * N);
  double *B = (double*) malloc(sizeof(double) * N * O);
  double *C = (double*) malloc(sizeof(double) * M * O);

  struct timeval tv1, tv2;
  int runtime;
  int dev = 0;

#pragma omp parallel for
  for (int i = 0; i < M; i++) 
    for (int j = 0; j < N; j++) {
      A[i*N + j] = rand() % 10;
    }
  fflush(stdout);

#pragma omp parallel for
  for (int i = 0; i < N; i++) 
    for (int j = 0; j < O; j++) {
      B[i*O + j] = rand() % 10;
    }
  fflush(stdout);

#pragma omp target device(dev)
  {}

  /**** CPU Execution ****/
  gettimeofday(&tv1, NULL);
#pragma omp parallel for
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < O; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("CPU,%d,%d,%d,%d,%0.4f\n", M, N, O, runtime, (double)(runtime/1000000.0));
  fflush(stdout);

  /**** GPU Execution - Combined ****/
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < O; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("distribute_parallel_for,%d,%d,%d,%d,%.4f\n", M, N, O, runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  /**** GPU Execution - Split ****/
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
  for (int i = 0; i < M; i++) {
#pragma omp parallel for
    for (int j = 0; j < O; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("distribute_parallel_for_split,%d,%d,%d,%d,%.4f\n", M, N, O, runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  /**** GPU Execution - Swap Combined ****/
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
  for (int j = 0; j < O; j++) {
    for (int i = 0; i < M; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("swap_distribute_parallel_for,%d,%d,%d,%d,%.4f\n", M, N, O, runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  /**** GPU Execution - Swap Split ****/
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
  for (int j = 0; j < O; j++) {
#pragma omp parallel for
    for (int i = 0; i < M; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("swap_distribute_parallel_for_split,%d,%d,%d,%d,%.4f\n", M, N, O, runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  /**** GPU Execution - Collapse ****/
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for collapse(2) map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < O; j++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("distribute_parallel_for_collapse,%d,%d,%d,%d,%.4f\n", M, N, O, runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  /**** GPU Execution - Swap Collapse ****/
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for collapse(2) map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
  for (int j = 0; j < O; j++) {
    for (int i = 0; i < M; i++) {
      double sum = 0.0;
      for (int k = 0; k < N; k++)
        sum = sum + A[i * N + k] * B[k * O + j];
      C[i * O + j] = sum;
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000;
  runtime += tv2.tv_usec - tv1.tv_usec;
  printf("swap_distribute_parallel_for_collapse,%d,%d,%d,%d,%.4f\n", M, N, O, runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  free(A);
  free(B);
  free(C);
  return 0;
}


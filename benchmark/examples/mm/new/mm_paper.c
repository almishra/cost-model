#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include "cuda_runtime.h"

#ifndef M
#define M 100
#endif

#ifndef N
#define N 300
#endif

#ifndef O
#define O 200
#endif

int main(int argc, char **argv)
{
  //double A[M*N], B[N*O], C[M*O];
  double *A = (double*) malloc(sizeof(double)*M*N);
  double *B = (double*) malloc(sizeof(double)*N*O);
  double *C = (double*) malloc(sizeof(double)*M*O);
  struct timeval  tv1, tv2;
  int runtime;
  int dev = 0;
  if(argc > 1)
    dev = atoi(argv[1]);

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
  { }

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
  printf("CPU,%d,%0.4f\n", runtime, (double)(runtime/1000000.0));
  fflush(stdout);

  printf("Kernel,Outer,Inner,Reduction,VarDecl,refExpr,intLiteral,floatLiteral,mem_to,mem_from,add_sub_int,add_sub_double,mul_int,mul_double,div_int,div_double,assign_int,assign_double,runtime_us,runtime\n");
  fflush(stdout);
/*  gettimeofday(&tv1, NULL);
#pragma omp target parallel for map(to:A[0:M*N], B[0:N*O]) map(from:C[0:M*O]) device(dev)
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
  printf("target_parallel_for_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);
*/
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
  printf("distribute_parallel_for_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

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
  printf("distribute_parallel_for_split_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

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
  printf("swap_distribute_parallel_for_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

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
  printf("swap_distribute_parallel_for_split_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

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
  printf("distribute_parallel_for_collapse_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

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
  printf("swap_distribute_parallel_for_collapse_%d_%d_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.4f\n", 
         M, N, O, M, O, 0, 2*O*1, 10*N*O+6*O, 3*O*N+3*O+1, O, 8*M*N+8*N*O, 8*M*O, 2*O+3*N*O, N*O, 2*N*O+2*O, N*O, 0, 0, 0, 2*O+N*O,
         runtime, (double)(runtime / 1000000.0));
  fflush(stdout);

  free(A);
  free(B);
  free(C);
  return 0;
}


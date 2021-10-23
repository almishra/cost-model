#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#ifndef M
#define M 200
#endif

#ifndef N
#define N 300
#endif

int main(int argc, char **argv)
{
  char output[100];
  int dev = 0;
  if(argc > 2)
    dev = atoi(argv[2]);
  if(argc > 1)
    strcpy(output, argv[1]);
  else
    strcpy(output, "output_matrix_vector.csv");

  int num_dev = omp_get_num_devices();
  if (num_dev < dev+1) { 
    printf("Cannot offload to device %d. Only %d devices available\n", dev, num_dev);
    exit(-1);
  }
  FILE *fp = fopen(output, "w");
  double A[M*N], B[N], C[M], D[M];
  struct timeval  tv1, tv2;
  double runtime; 

#pragma omp parallel for
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      A[i*N + j] = i*N + j + 1;
    //A[i*N + j] = rand() % 10;
    C[i] = 0;
    D[i] = 0;
  }
#pragma omp parallel for
  for (int i = 0; i < N; i++) {
    B[i] = i+1;
    //B[i] = rand() % 10;
  }

#ifdef __DEBUG
  printf("A:\n");
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)  
      printf("%.0lf\t", A[i*N + j]);
    printf("\n");
  }
  printf("B:\n");
  for (int i = 0; i < N; i++) {
    printf("%.0lf\n", B[i]);
  }
#endif
  fprintf(fp,"cpu,gpu,collapse,collapse_swap,combined,combined_swap,split,split_swap,sched_static,sched_dynamic,sched_guided,outer,inner,reduction,var_decl,ref_expr,int_literal,float_literal,mem_to,mem_from,add_sub_int,add_sub_double,mul_int,mul_double,div_int,div_double,assign_int,assign_double,logical,runtimes\n");
  fflush(fp);

  for(int count = 0; count < 4; count++) {
  gettimeofday(&tv1, NULL);
#pragma omp parallel for
  for (int i = 0; i < M; i++) {
    D[i] = 0;
    for (int j = 0; j < N; j++) {
      D[i] += A[i*N+j] * B[j];
    }
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "1,0,0,0,0,0,0,0,1,0,0,%d,%d,0,1,%d,%d,0,%d,%d,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, 0, 0, 2*N, N, N, N, N, N, runtime);
  fflush(fp);

#ifdef __DEBUG
  printf("D:\n");
  for (int i = 0; i < M; i++) {
    printf("%.0lf\n", D[i]);
  }
#endif
#pragma omp target
  {}

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for map(to:A[0:M*N], B[0:N]) map(C[0:M]) 
  for (int i = 0; i < M; i++) {
    double sum = 0;
    for (int j = 0; j < N; j++) {
      sum += A[i*N+j] * B[j];
    }
    C[i] = sum;
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "0,1,0,0,1,0,0,0,1,0,0,%d,%d,0,1,%d,%d,0,%ld,%ld,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N, sizeof(double)*M, 2*N, N, N, N, N, N, runtime);
  fflush(fp);
  printf("GPU combined = %.2f\n",
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec)); 
  for(int i=0; i<M; i++)
    if(C[i] != D[i]) {
      printf("Error C[%d] = %.0lf, D[%d] = %.0lf\n", i, C[i], i, D[i]);
      fflush(stdout);
      fclose(fp);
      exit(-1);
    }
  printf("Success\n");

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for schedule(dynamic) map(to:A[0:M*N], B[0:N]) map(C[0:M]) 
  for (int i = 0; i < M; i++) {
    double sum = 0;
    for (int j = 0; j < N; j++) {
      sum += A[i*N+j] * B[j];
    }
    C[i] = sum;
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "0,1,0,0,1,0,0,0,0,1,0,%d,%d,0,1,%d,%d,0,%ld,%ld,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N, sizeof(double)*M, 2*N, N, N, N, N, N, runtime);
  fflush(fp);
  printf("GPU combined dynamic = %.2f\n",
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec)); 
  for(int i=0; i<M; i++)
    if(C[i] != D[i]) {
      printf("Error C[%d] = %.0lf, D[%d] = %.0lf\n", i, C[i], i, D[i]);
      fflush(stdout);
      fclose(fp);
      exit(-1);
    }
  printf("Success\n");

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for schedule(guided) map(to:A[0:M*N], B[0:N]) map(C[0:M]) 
  for (int i = 0; i < M; i++) {
    double sum = 0;
    for (int j = 0; j < N; j++) {
      sum += A[i*N+j] * B[j];
    }
    C[i] = sum;
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "0,1,0,0,1,0,0,0,0,0,1,%d,%d,0,1,%d,%d,0,%ld,%ld,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N, sizeof(double)*M, 2*N, N, N, N, N, N, runtime);
  fflush(fp);
  printf("GPU combined guided = %.2f\n",
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec)); 
  for(int i=0; i<M; i++)
    if(C[i] != D[i]) {
      printf("Error C[%d] = %.0lf, D[%d] = %.0lf\n", i, C[i], i, D[i]);
      fflush(stdout);
      fclose(fp);
      exit(-1);
    }
  printf("Success\n");

//  printf("Memory to: %.2lf\n", (sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N) / 1024.0 / 1024.0 / 1024.0);
//  printf("Memory from: %.2lf\n", sizeof(double)*M / 1024.0);
  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute map(to:A[0:M*N], B[0:N]) map(C[0:M]) 
  for (int i = 0; i < M; i++) {
    double sum = 0;
#pragma omp parallel for reduction(+:sum)
    for (int j = 0; j < N; j++) {
      sum += A[i*N+j] * B[j];
    }
    C[i] = sum;
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "0,1,0,0,0,0,1,0,1,0,0,%d,%d,1,1,%d,%d,0,%ld,%ld,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N, sizeof(double)*M, 2*N, N, N, N, N, N, runtime);
  fflush(fp);
  printf("GPU split = %.2f\n",
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec)); 
#ifdef __DEBUG
  printf("C:\n");
  for (int i = 0; i < M; i++) {
    printf("%.0lf\n", C[i]);
  }
#endif
  for(int i=0; i<M; i++)
    if(C[i] != D[i]) {
      printf("Error C[%d] = %.0lf, D[%d] = %.0lf\n", i, C[i], i, D[i]);
      fflush(stdout);
      fclose(fp);
      exit(-1);
    }
  printf("Success\n");

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute map(to:A[0:M*N], B[0:N]) map(C[0:M]) 
  for (int i = 0; i < M; i++) {
    double sum = 0;
#pragma omp parallel for reduction(+:sum) schedule(dynamic)
    for (int j = 0; j < N; j++) {
      sum += A[i*N+j] * B[j];
    }
    C[i] = sum;
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "0,1,0,0,0,0,1,0,0,1,0,%d,%d,1,1,%d,%d,0,%ld,%ld,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N, sizeof(double)*M, 2*N, N, N, N, N, N, runtime);
  fflush(fp);
  printf("GPU split dynamic = %.2f\n",
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec)); 
  for(int i=0; i<M; i++)
    if(C[i] != D[i]) {
      printf("Error C[%d] = %.0lf, D[%d] = %.0lf\n", i, C[i], i, D[i]);
      fflush(stdout);
      fclose(fp);
      exit(-1);
    }
  printf("Success\n");

  gettimeofday(&tv1, NULL);
#pragma omp target teams distribute map(to:A[0:M*N], B[0:N]) map(C[0:M]) 
  for (int i = 0; i < M; i++) {
    double sum = 0;
#pragma omp parallel for reduction(+:sum) schedule(guided)
    for (int j = 0; j < N; j++) {
      sum += A[i*N+j] * B[j];
    }
    C[i] = sum;
  }
  gettimeofday(&tv2, NULL);
  runtime = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
  fprintf(fp, "0,1,0,0,0,0,1,0,0,0,1,%d,%d,1,1,%d,%d,0,%ld,%ld,%d,%d,%d,%d,0,0,1,%d,%d,%.0lf\n", M, N, 9*N, 2*N, sizeof(double)*M*N + sizeof(double)*M + sizeof(double)*N, sizeof(double)*M, 2*N, N, N, N, N, N, runtime);
  fflush(fp);
  printf("GPU split guided = %.2f\n",
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec)); 
  for(int i=0; i<M; i++)
    if(C[i] != D[i]) {
      printf("Error C[%d] = %.0lf, D[%d] = %.0lf\n", i, C[i], i, D[i]);
      fflush(stdout);
      fclose(fp);
      exit(-1);
    }
  printf("Success\n");
  fprintf(fp, "--------------------------------------------\n");
  fflush(fp);
  }

  fclose(fp);
  return 0;
}

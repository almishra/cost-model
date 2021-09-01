#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <omp.h>
#include <sys/time.h>

#define MAX_ITER 20

// Maximum value of the matrix element
#define MAX 100
#define TOL 0.000001

// Generate a random float number with the maximum value of max
double rand_double(const int max) {
  return ((double)rand() / (double)(RAND_MAX)) * max;
}

#define n1 511
int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("Call this program with two parameters: <matrix_size> <output_file>\n");
    printf("\t matrix_size: Add 2 to a power of 2 (e.g. : 18, 1026)\n");
    exit(1);
  }

  FILE *fp;
  const int n = atoi(argv[1]);
  //const int n = 512;

  if(argc > 2)
    fp = fopen(argv[2], "w");
  else
    fp = fopen("output.csv", "w");
  if(!fp) {
    fprintf(stderr, "Error: Unable to open output file\n");
    return -1;
  }

  int num_dev = omp_get_num_devices();
  fprintf(fp, "kernel,Outer,Inner,reduction,VarDecl,refExpr,intLiteral,floatLiteral,mem_to,mem_from,add_sub_int,add_sub_double,mul_int,mul_double,div_int,div_double,assign_int,assign_double,runtime1,runtime\n");
  printf("kernel,Outer,Inner,reduction,VarDecl,refExpr,intLiteral,floatLiteral,mem_to,mem_from,add_sub_int,add_sub_double,mul_int,mul_double,div_int,div_double,assign_int,assign_double,runtime1,runtime\n");
  fflush(stdout);
#pragma omp parallel for
  for(int dev=0; dev<num_dev; dev++) {
    const int mat_dim = n * n;
    double mat[mat_dim];
#pragma omp critical
    {
      printf("Allocating in %d\n", dev);
      fflush(stdout);
    }
#pragma omp target teams distribute parallel for collapse(2) map(from: mat[0:mat_dim]) device(dev)
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++)
        mat[i*n+j] = (double)(i*j);

#pragma omp critical
    {
      printf("Done in %d\n", dev);
      fflush(stdout);
    }

    // Parallelized solver
    double diff;

    int done = 0;
    int cnt_iter = 0;
    long total_size = 0;
    while (!done && (cnt_iter < MAX_ITER)) {
      diff = 0;

      struct timeval tv1, tv2;
      double runtime;
      total_size = sizeof(double) * mat_dim;
      gettimeofday(&tv1, NULL);
#pragma omp target teams distribute parallel for collapse(2) reduction(+:diff) device(dev) map(mat[0:mat_dim])
      for (int i = 1; i < n-1; i++) {
        for (int j = 1; j < n-1; j++) {

          const double temp = mat[i * n + j];

          mat[i * n + j] = 0.2 * (
              mat[i * n + j]
              + mat[i * n + j-1]
              + mat[(i-1) * n + j]
              + mat[i * n + j+1]
              + mat[(i+1) * n + j]
              );

          double x = mat[i * n + j] - temp;
          if(x < 0) x *= -1;
          diff += x;
        }
      }
      gettimeofday(&tv2, NULL);
      runtime = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
#pragma omp critical
      fprintf(fp, "Kernel_%d,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%d,%.0lf,%.6lf\n", dev, n-2, n-2, 1, 2, 37, 6, 1, total_size, total_size, 12, 6, 8, 2, 0, 0, 0, 4, runtime, runtime / 1000000.0);

      if (diff/mat_dim < TOL) {
        done = 1;
      }
      cnt_iter ++;
    }
  }

  return 0;
}

#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <sys/time.h>

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

int main(int argc, char**argv) {
  int iter_max = 5000;
  int iter;
  double err = 1.0;
  double tol = 0.0000000001;
  double A[1000][1000];
  double Anew[1000][1000];
  double alpha = 0.0543;
  initialize(1000, 1000, alpha, A);

  int i, j;
  struct timeval t1, t2;

  gettimeofday(&t1, NULL);
  iter = 0;


  while (err>tol && iter<iter_max) {
    err=0.0;
#pragma omp target data map(to:A) map(err, Anew)
#pragma omp target teams reduction(max:err)

    for(i=1; i<1000-1; i++) {
#pragma omp distribute parallel for
      for( j = 1; j < 1000 - 1; j++) {
        Anew[i][j] = 0.25 * (A[i][j+1] + A[i][j-1] + A[i-1][j] + A[i+1][j]);
        double val = Anew[i][j] - A[i][j]; 
        if(val < 0) val *= -1;
        if(err < val)
          err = val;
      }
    }

#pragma omp target data map(to: Anew) map(from:A)
#pragma omp target teams 
#pragma omp distribute parallel for collapse(2)
    for(i=1; i<1000-1; i++) {

      for(j=1; j<1000-1; j++) {
        A[i][j] = Anew[i][j];      
      }
    }
    iter++;
  }
  gettimeofday(&t2, NULL);
  unsigned runtime = (t2.tv_sec - t1.tv_sec) * 1000000;
  runtime += (t2.tv_usec - t1.tv_usec);

  printf("%d\n", runtime);
  return 0;
}

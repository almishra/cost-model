#include <stdio.h>
#include <omp.h>

int main()
{
  int A = -1;
#pragma omp target map(A)
  {
    A = omp_is_initial_device();
  }

  if (A == 0) {
    printf("Offloading is supported!\n");
  } else {
    printf("Offloading NOT supported!\n");
  }

  return 0;
}

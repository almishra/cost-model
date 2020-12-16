gen() 
{
  file=$1
  ROW=$2
  PRAGMA_MAP=""
  PRAGMA_TARGET=""
  PRAGMA=""
  if [[ $# -gt 2 ]]; then
    if [[ ! $3 = "NA" ]]; then
      PRAGMA_MAP=$3
    fi
    if [[ $# -gt 3 ]]; then
      if [[ ! $4 = "NA" ]]; then
        PRAGMA_TARGET=$4
      fi
      if [[ $# -gt 4 ]]; then
        if [[ ! $5 = "NA" ]]; then
          PRAGMA=$5
        fi
      fi
    fi
  fi
  cat << EOF > $file
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <sys/time.h>

void init(int n, double a[n])
{
  /* Initilize array*/
  for (int i=0; i<n; i++){
    a[i] = rand()%10+0.5;
  }
}

#ifdef DEBUG
void print(int n, double a[n])
{
  for (int i=0; i<n; i++){
    printf("%0.2lf ", a[i]);
  }
  printf("\n");
}
#endif

int main(int argc, char**argv) {
  double *A = (double*) malloc(sizeof(double) * $ROW);
  double *B = (double*) malloc(sizeof(double) * $ROW);
  double *C = (double*) malloc(sizeof(double) * $ROW);

  init($ROW, A);
  init($ROW, B);

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

$PRAGMA_MAP
$PRAGMA_TARGET
$PRAGMA
  for(int i=0; i<$ROW; i++) {
    C[i] = A[i] * B[i];
    C[i] *= A[i] * B[i];
    C[i] /= A[i] / B[i];
    C[i] += A[i];
  }
  gettimeofday(&t2, NULL);
  unsigned runtime = (t2.tv_sec - t1.tv_sec) * 1000000;
  runtime += (t2.tv_usec - t1.tv_usec);

#ifdef DEBUG
  printf("A = ");
  print($ROW, A);
  printf("B = ");
  print($ROW, B);
  printf("C = ");
  print($ROW, C);
#endif
  printf("%d\n", runtime);
  return 0;
}
EOF
}

#ROW=400000000
ROW=400
name="array_${ROW}"
name_target="${name}_target"
name_target_static="${name_target}_static"
name_target_dynamic="${name_target}_dynamic"
name_target_guided="${name_target}_guided"
name_target_auto="${name_target}_auto"
name_target_runtime="${name_target}_runtime"
name_target_teams="${name_target}_teams"
name_target_teams_static="${name_target_teams}_static"
name_target_teams_dynamic="${name_target_teams}_dynamic"
name_target_teams_guided="${name_target_teams}_guided"
name_target_teams_auto="${name_target_teams}_auto"
name_target_teams_runtime="${name_target_teams}_runtime"
name_target_teams_distribute="${name_target_teams}_distribute"
name_target_teams_distribute_static="${name_target_teams_distribute}_static"
name_target_teams_distribute_dynamic="${name_target_teams_distribute}_dynamic"
name_target_teams_distribute_guided="${name_target_teams_distribute}_guided"
name_target_teams_distribute_auto="${name_target_teams_distribute}_auto"
name_target_teams_distribute_runtime="${name_target_teams}_runtime"
name_target_teams_distribute_parallel_for="${name_target_teams_distribute}_parallel_for"
name_target_teams_distribute_parallel_for_static="${name_target_teams_distribute_parallel_for}_static"
name_target_teams_distribute_parallel_for_dynamic="${name_target_teams_distribute_parallel_for}_dynamic"
name_target_teams_distribute_parallel_for_guided="${name_target_teams_distribute_parallel_for}_guided"
name_target_teams_distribute_parallel_for_auto="${name_target_teams_distribute_parallel_for}_auto"
name_target_teams_distribute_parallel_for_runtime="${name_target_teams_distribute_parallel_for}_runtime"

gen ${name}.c $ROW "NA" "NA" "#pragma omp parallel for"

gen ${name_target}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target" "#pragma omp parallel for"
gen ${name_target_static}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target" "#pragma omp parallel for schedule(static)"
gen ${name_target_dynamic}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target" "#pragma omp parallel for schedule(dynamic)"
gen ${name_target_guided}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target" "#pragma omp parallel for schedule(guided)"
gen ${name_target_auto}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target" "#pragma omp parallel for schedule(auto)"
gen ${name_target_runtime}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target" "#pragma omp parallel for schedule(runtime)"

gen ${name_target_teams}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp parallel for"
gen ${name_target_teams_static}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp parallel for schedule(static)"
gen ${name_target_teams_dynamic}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp parallel for schedule(dynamic)"
gen ${name_target_teams_guided}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp parallel for schedule(guided)"
gen ${name_target_teams_auto}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp parallel for schedule(auto)"
gen ${name_target_teams_runtime}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp parallel for schedule(runtime)"

gen ${name_target_teams_distribute}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute"

gen ${name_target_teams_distribute_parallel_for}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute parallel for"
gen ${name_target_teams_distribute_parallel_for_static}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute parallel for schedule(static)"
gen ${name_target_teams_distribute_parallel_for_dynamic}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute parallel for schedule(dynamic)"
gen ${name_target_teams_distribute_parallel_for_guided}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute parallel for schedule(guided)"
gen ${name_target_teams_distribute_parallel_for_auto}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute parallel for schedule(auto)"
gen ${name_target_teams_distribute_parallel_for_runtime}.c $ROW "#pragma omp target data map(to:A[0:$ROW], B[0:$ROW]) map(from:C[0:$ROW])" "#pragma omp target teams" "#pragma omp distribute parallel for schedule(runtime)"

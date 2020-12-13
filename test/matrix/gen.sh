gen_func() {
  file=$1
  ROW1=$2
  COL1=$3
  ROW2=$4
  COL2=$5
  PRAGMA1=""
  PRAGMA2=""
  PRAGMA3=""
  PRAGMA4=""
  if [[ $# -gt 5 ]]
  then
    PRAGMA1=$6
    if [[ $# -gt 6 ]]
    then
      PRAGMA2=$7
      if [[ $# -gt 7 ]]
      then
        if [[ $# -gt 8 ]]
        then
          if [[ $8 = "NA" ]]
          then
            PRAGMA3=""
          else
            PRAGMA3=$8
          fi
          PRAGMA4=$9
        else
          PRAGMA3=$8
        fi
      fi
    fi
  fi
  
  cat << EOF > $file
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int *init(int R, int C)
{
  int *A = (int*) malloc(sizeof(int)*R*C);

#pragma omp parallel for
  for(int i=0; i<R; i++)
    for(int j=0; j<C; j++)
      A[i*C+j] = random() % 10;

  return A;
}

#ifdef DEBUG
void print(int R, int C, int *A)
{
  for(int i=0; i<R; i++) {
    for(int j=0; j<C; j++)
      printf("%4d ", A[i*C+j]);
    printf("\n");
  }
}
#endif

int main()
{
  int *A = init($ROW1, $COL1);
  int *B = init($ROW2, $COL2);
  int *C = (int*) malloc(sizeof(int)*$ROW1*$COL2);

  struct timeval start, end;
  gettimeofday(&start, NULL);
$PRAGMA1
$PRAGMA2
$PRAGMA3
  for(int i=0; i<$ROW1; i++) {
$PRAGMA4
    for(int j=0; j<$COL2; j++) {
      C[i*$COL2+j] = 0;
      for(int k=0; k<$COL1; k++)
        C[i*$COL2+j] += A[i*$COL1+k] * B[k*$COL2+j];
    }
  }
  gettimeofday(&end, NULL);
  unsigned delta = (end.tv_sec  - start.tv_sec) * 1000000u + 
                   end.tv_usec - start.tv_usec;
  printf("%d\n", delta);

#ifdef DEBUG
  printf("Matrix A\n");
  print($ROW1, $COL1, A);
  printf("Matrix B\n");
  print($ROW2, $COL2, B);
  printf("Matrix C\n");
  print($ROW1, $COL2, C);
#endif

  return 0;
}

EOF
}

I=( 5000 ); #5 500 );
J=( 10000 ); #5 100 15 );
K=( 5000 ); #10 500 );
for i in ${I[@]}
do
  for j in ${J[@]}
  do
    for k in ${K[@]}
    do
      echo "matrix_${i}_${j}_${j}_${k}";
      ROW1=$i
      COL1=$j
      ROW2=$j
      COL2=$k
      name="matrix_${ROW1}_${COL1}_${ROW2}_${COL2}"
      name_collapse="${name}_collapse"
      name_target="${name}_target"
      name_target_teams="${name_target}_teams"
      name_target_teams_distribute="${name_target_teams}_distribute"
      name_target_teams_inner_distribute="${name_target_teams}_inner_distribute"
      name_target_teams_distribute_inner_parallel="${name_target_teams_distribute}_inner_parallel"
      name_target_teams_distribute_inner_parallel_for="${name_target_teams_distribute_inner_parallel}_for"
      name_target_teams_inner_distribute_parallel_for="${name_target_teams_inner_distribute}_parallel_for"
      name_target_teams_distribute_parallel_for="${name_target_teams_distribute}_parallel_for"
      name_target_collapse="${name_target}_collapse"
      name_target_teams_collapse="${name_target_teams}_collapse"
      name_target_teams_distribute_parallel_for_collapse="${name_target_teams_distribute_parallel_for}_collapse"
      gen_func "${name}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp parallel for"
      gen_func "${name_collapse}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp parallel for collapse(2)"
      gen_func "${name_target}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target" "#pragma omp parallel for"
      gen_func "${name_target_teams}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams" "#pragma omp parallel for"
      gen_func "${name_target_teams_distribute}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams distribute"
      gen_func "${name_target_teams_inner_distribute}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams" "NA" "#pragma omp distribute"
      gen_func "${name_target_teams_distribute_inner_parallel}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams distribute" "NA" "#pragma omp parallel"
      gen_func "${name_target_teams_distribute_inner_parallel_for}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target" "#pragma omp teams distribute" "#pragma omp parallel for"
      gen_func "${name_target_teams_inner_distribute_parallel_for}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams" "NA" "#pragma omp distribute parallel for"
      gen_func "${name_target_teams_distribute_parallel_for}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams distribute parallel for"
      gen_func "${name_target_collapse}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target" "#pragma omp parallel for collapse(2)"
      gen_func "${name_target_teams_collapse}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams" "#pragma omp parallel for collapse(2)"
      gen_func "${name_target_teams_distribute_parallel_for_collapse}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
               "#pragma omp target teams distribute parallel for collapse(2)"
    done
  done
done

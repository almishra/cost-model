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
          PRAGMA3=""
          PRAGMA4=$9
        else
          PRAGMA3=$8
        fi
      fi
    fi
  fi
  
  shift
  echo "#include <stdio.h>" > $file
  echo "#include <stdlib.h>" >> $file
  echo "#include <sys/time.h>" >> $file
  echo "" >> $file
  echo "int *init(int R, int C)" >> $file
  echo "{" >> $file
  echo "  int *A = (int*) malloc(sizeof(int)*R*C);" >> $file
  echo "" >> $file
  echo "  for(int i=0; i<R; i++)" >> $file
  echo "    for(int j=0; j<C; j++)" >> $file
  echo "      A[i*C+j] = random() % 10;" >> $file
  echo "" >> $file
  echo "  return A;" >> $file
  echo "}" >> $file
  echo "" >> $file
  echo "#ifdef DEBUG" >> $file
  echo "void print(int R, int C, int *A)" >> $file
  echo "{" >> $file
  echo "  for(int i=0; i<R; i++) {" >> $file
  echo "    for(int j=0; j<C; j++)" >> $file
  echo "      printf(\"%4d \", A[i*C+j]);" >> $file
  echo "    printf(\"\\n\");" >> $file
  echo "  }" >> $file
  echo "}" >> $file
  echo "#endif" >> $file
  echo "" >> $file
  echo "int main()" >> $file
  echo "{" >> $file
  echo "  int *A = init($ROW1, $COL1);" >> $file
  echo "  int *B = init($ROW2, $COL2);" >> $file
  echo "  int *C = (int*) malloc(sizeof(int)*$ROW1*$COL2);" >> $file
  echo "" >> $file
  echo "  struct timeval start, end;" >> $file
  echo "  gettimeofday(&start, NULL);" >> $file
  if [ -n "$PRAGMA1" ]
  then
    echo "$PRAGMA1" >> $file
  fi
  if [ -n "$PRAGMA2" ]
  then
    echo "$PRAGMA2" >> $file
  fi
  if [ -n "$PRAGMA3" ]
  then
    echo "$PRAGMA3" >> $file
  fi
  echo "  for(int i=0; i<$ROW1; i++) {" >> $file
  if [ -n "$PRAGMA4" ]
  then
    echo "$PRAGMA4" >> $file
  fi
  echo "    for(int j=0; j<$COL2; j++) {" >> $file
  echo "      C[i*$COL2+j] = 0;" >> $file
  echo "      for(int k=0; k<$COL1; k++)" >> $file
  echo "        C[i*$COL2+j] += A[i*$COL1+k] * B[k*$COL2+j];" >> $file
  echo "    }" >> $file
  echo "  }" >> $file
  echo "  gettimeofday(&end, NULL);" >> $file
  echo "  unsigned delta = (end.tv_sec  - start.tv_sec) * 1000000u + " >> $file
  echo "                   end.tv_usec - start.tv_usec;" >> $file
  echo "  printf(\"%d\\n\", delta);" >> $file
  echo "" >> $file
  echo "#ifdef DEBUG" >> $file
  echo "  printf(\"Matrix A\\n\");" >> $file
  echo "  print($ROW1, $COL1, A);" >> $file
  echo "  printf(\"Matrix B\\n\");" >> $file
  echo "  print($ROW2, $COL2, B);" >> $file
  echo "  printf(\"Matrix C\\n\");" >> $file
  echo "  print($ROW1, $COL2, C);" >> $file
  echo "#endif" >> $file
  echo "" >> $file
  echo "  return 0;" >> $file
  echo "}" >> $file
  echo "" >> $file
}

#for (( i=5; i<=10; i+=5 ))
#do
#  for(( j=5; j<=10; j+=5 ))
#  do
#    sed "s/#define ROW 2/#define ROW ${i}/g" matrix.c > matrix_collapse_${i}_${j}.c
#    sed -i "s/#define COL 2/#define COL ${j}/g" matrix_collapse_${i}_${j}.c
#  done
#done
name="matrix.c"
ROW1=10
COL1=10
ROW2=10
COL2=5
name="matrix_${ROW1}_${COL1}_${ROW2}_${COL2}"
name1="${name}_collapse"
name2="${name}_target"
name3="${name}_target_teams"
gen_func "${name}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp parallel for"
gen_func "${name1}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp parallel for collapse(2)"
gen_func "${name2}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target" "#pragma omp parallel for"
gen_func "${name3}.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams" "#pragma omp parallel for"
gen_func "matrix_target_teams_distribute.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams distribute"
gen_func "matrix_target_teams_inner_distribute.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams" "NA" "#pragma omp distribute"
gen_func "matrix_target_teams_distribute_inner_parallel.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams distribute" "NA" "#pragma omp parallel for"
gen_func "matrix_target_teams_inner_distribute_parallel_for.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams" "NA" "#pragma omp distribute parallel for"
gen_func "matrix_target_teams_distribute_parallel_for.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams distribute parallel for"
gen_func "matrix_target_collapse.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target" "#pragma omp parallel for collapse(2)"
gen_func "matrix_target_teams_collapse.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams" "#pragma omp parallel for collapse(2)"
gen_func "matrix_target_teams_distribute_parallel_for_collapse.c" $ROW1 $COL1 $ROW2 $COL2 "#pragma omp target data map(to:A[0:$ROW1*$COL1], B[0:$ROW2*$COL2]) map(C[0:$ROW1*$COL2])" \
         "#pragma omp target teams distribute parallel for collapse(2)"

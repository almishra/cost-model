gen() 
{
  file=$1
  ROW=$2
  COL=$3
  shift 3
  PRAGMA_MAP1=""
  PRAGMA_MAP2=""
  PRAGMA_MAP3=""
  PRAGMA_TARGET1=""
  PRAGMA_TARGET2=""
  PRAGMA1=""
  PRAGMA2=""
  PRAGMA3=""
  PRAGMA4=""
  if [[ $# -gt 0 ]]; then
    if [[ ! $1 = "NA" ]]; then
      PRAGMA_MAP1=$1
    fi
    if [[ $# -gt 1 ]]; then
      if [[ ! $2 = "NA" ]]; then
        PRAGMA_MAP2=$2
      fi
      if [[ $# -gt 2 ]]; then
        if [[ ! $3 = "NA" ]]; then
          PRAGMA_TARGET1=$3
        fi
        if [[ $# -gt 3 ]]; then
          if [[ ! $4 = "NA" ]]; then
            PRAGMA1=$4
          fi
          if [[ $# -gt 4 ]]; then
            if [[ ! $5 = "NA" ]]; then
              PRAGMA2=$5
            fi
            if [[ $# -gt 5 ]]; then
              if [[ ! $6 = "NA" ]]; then
                PRAGMA_MAP3=$6
              fi
              if [[ $# -gt 6 ]]; then
                if [[ ! $7 = "NA" ]]; then
                  PRAGMA_TARGET2=$7
                fi
                if [[ $# -gt 7 ]]; then
                  if [[ ! $8 = "NA" ]]; then
                    PRAGMA3=$8
                  fi
                  if [[ $# -gt 8 ]]; then
                    if [[ ! $9 = "NA" ]]; then
                      PRAGMA4=$9
                    fi
                  fi
                fi
              fi
            fi
          fi
        fi
      fi
    fi
  fi
  cat << EOF > $file
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
  double A[$ROW][$COL];
  double Anew[$ROW][$COL];
  double alpha = 0.0543;
  initialize($COL, $ROW, alpha, A);

  int i, j;
  struct timeval t1, t2;

  gettimeofday(&t1, NULL);
  iter = 0;

$PRAGMA_MAP1
  while (err>tol && iter<iter_max) {
    err=0.0;
$PRAGMA_MAP2
$PRAGMA_TARGET1
$PRAGMA1
    for(i=1; i<$ROW-1; i++) {
$PRAGMA2
      for( j = 1; j < $COL - 1; j++) {
        Anew[i][j] = 0.25 * (A[i][j+1] + A[i][j-1] + A[i-1][j] + A[i+1][j]);
        double val = Anew[i][j] - A[i][j]; 
        if(val < 0) val *= -1;
        if(err < val)
          err = val;
      }
    }

$PRAGMA_MAP3
$PRAGMA_TARGET2
$PRAGMA3
    for(i=1; i<$ROW-1; i++) {
$PRAGMA4
      for(j=1; j<$COL-1; j++) {
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
EOF
}

ROW=1000
COL=1000
name="jacobi_${ROW}_${COL}"
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

gen ${name}.c $ROW $COL "NA" "NA" "NA" "#pragma omp parallel for reduction(max:err)" "NA" "NA" "NA" "#pragma omp parallel for" "NA"
gen ${name_collapse}.c $ROW $COL "NA" "NA" "NA" "#pragma omp parallel for reduction(max:err) collapse(2)" "NA" "NA" "NA" "#pragma omp parallel for collapse(2)" "NA"
gen ${name_target}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target reduction(max:err)" "#pragma omp parallel for reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target teams reduction(max:err)" "#pragma omp parallel for reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_distribute}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "NA" "#pragma omp target teams distribute reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_inner_distribute}.c $ROW $COL "NA" "NA" "#pragma omp target teams reduction(max:err)" "NA" "#pragma omp distribute" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_distribute_inner_parallel}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target teams distribute reduction(max:err)" "NA" "#pragma omp parallel" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_distribute_inner_parallel_for}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target teams distribute reduction(max:err)" "NA" "#pragma omp parallel for" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_inner_distribute_parallel_for}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target teams reduction(max:err)" "NA" "#pragma omp distribute parallel for" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_distribute_parallel_for}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "NA" "#pragma omp target teams distribute parallel for reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_collapse}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target reduction(max:err)" "#pragma omp parallel for collapse(2) reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_collapse}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "#pragma omp target teams reduction(max:err)" "#pragma omp parallel for collapse(2) reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"
gen ${name_target_teams_distribute_parallel_for_collapse}.c $ROW $COL "NA" "#pragma omp target data map(to:A) map(err, Anew)" "NA" "#pragma omp target teams distribute parallel for collapse(2) reduction(max:err)" "NA" "#pragma omp target data map(to: Anew) map(from:A)" "#pragma omp target teams " "#pragma omp distribute parallel for collapse(2)" "NA"

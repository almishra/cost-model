cd /gpfs/projects/ChapmanGroup/alok/git/cost-model/benchmark/examples/matrix_vector

if [ "$#" -ne 1 ]; then
  echo "Illegal number of parameters"
  exit
fi

for (( M=100; M<=17000; M+=500)); 
do 
  for (( N=100; N<=10000; N+=500)); 
  do 
    if [ ! -f matrix_vector_${M}_${N}.out]
    then
      clang -O0 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/cm/shared/apps/cuda10.0/toolkit/10.0.130 -Xopenmp-target -march=sm_37 matrix_vector.c -o matrix_vector_${M}_${N}.out -DM=${M} -DN=${N};
      ./matrix_vector_${M}_${N}.out output_matrix_vector_${M}_${N}.csv $1
      echo "###################################" >> output_matrix_vector_${M}_${N}.csv
      nvidia-smi >> output_matrix_vector_${M}_${N}.csv
      echo "###################################" >> output_matrix_vector_${M}_${N}.csv
      lscpu >> output_matrix_vector_${M}_${N}.csv
    fi
  done
done


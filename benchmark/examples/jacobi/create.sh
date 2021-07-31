echo "#!/bin/bash" > run0.lsf
echo "BSUB -W 2:00" >> run0.lsf
echo "BSUB -nnodes 1" >> run0.lsf
echo "BSUB -P csc401" >> run0.lsf
echo "BSUB -o jacobi0.o%J" >> run0.lsf
echo "BSUB -J jacobi0" >> run0.lsf
echo "" >> run0.lsf
echo "module load cuda/10.1.243 llvm/13.0.0-20210225" >> run0.lsf
echo "export OMP_NUM_THREADS=168" >> run0.lsf
echo "cd /gpfs/alpine/scratch/alokm/csc401/cost-model/benchmark/examples/jacobi" >> run0.lsf
echo "" >> run0.lsf
echo "jsrun -n 1 -c 42 -g 6 -bpacked:42 ./run0.sh" >> run0.lsf 

for (( i=1; i<=16; i++ ));
do 
  cp run0.lsf run${i}.lsf; 
  sed -i "s/jacobi0/jacobi$i/" run${i}.lsf; 
  sed -i "s/run0.sh/run$i.sh/" run${i}.lsf; 
done

for (( i=0; i<=16; i++ )); 
do 
  echo "cd /gpfs/alpine/scratch/alokm/csc401/cost-model/benchmark/examples/jacobi" > run${i}.sh; 
  echo >> run${i}.sh; 
  x=$((6 * i)); 
  echo "./jacobi${x} 0 > output${x}.csv" >> run${i}.sh; 
  x=$((x + 1)); 
  echo "./jacobi${x} 1 > output${x}.csv" >> run${i}.sh; 
  x=$((x + 1)); 
  echo "./jacobi${x} 2 > output${x}.csv" >> run${i}.sh; 
  x=$((x + 1)); 
  echo "./jacobi${x} 3 > output${x}.csv" >> run${i}.sh; 
  x=$((x + 1)); 
  echo "./jacobi${x} 4 > output${x}.csv" >> run${i}.sh; 
  x=$((x + 1)); 
  echo "./jacobi${x} 5 > output${x}.csv" >> run${i}.sh; 
done

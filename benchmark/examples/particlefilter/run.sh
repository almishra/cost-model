for (( i=10000000; i<=40000000; i+=1000000)); 
do 
#clang++ -lm -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/cm/shared/apps/cuda10.0/toolkit/10.0.130 -Xopenmp-target -march=sm_37 ex_particle_OPENMP_seq.cpp -o offload -DNparticles=$i; 
  g++ -fopenmp -foffload=nvptx-none -lm -fno-fast-math -fno-associative-math ex_particle_OPENMP_seq.cpp -o offload_${i} -DNparticles=$i
#  echo $i; 
  ./offload_${i} -x 128 -y 128 -z 10 -np 1 >> output_${i}; 
#  echo "";
done

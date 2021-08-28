cd /gpfs/projects/ChapmanGroup/alok/git/cost-model/benchmark/examples/mm/gcc

for (( M=2150; M<=3000; M+=100)); 
do 
  for (( N=2150; N<=3000; N+=100)); 
  do 
    for (( O=2150; O<=3000; O+=100)); 
    do 
      if [ ! -f mm_paper_${M}_${N}_${O}.out ]                                      
      then
#clang -Wall -O0 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/cm/shared/apps/cuda10.0/toolkit/10.0.130 -Xopenmp-target -march=sm_37 mm_paper.c -o mm_paper_${M}_${N}_${O}.out -DM=${M} -DN=${N} -DO=${O};
        gcc -fopenmp -foffload=nvptx-none mm_paper.c -o mm_paper_${M}_${N}_${O}.out -DM=${M} -DN=${N} -DO=${O}
        ./mm_paper_${M}_${N}_${O}.out 1 > output_${M}_${N}_${O}
      fi
    done
  done
done


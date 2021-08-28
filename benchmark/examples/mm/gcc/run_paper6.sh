cd /gpfs/projects/ChapmanGroup/alok/git/cost-model/benchmark/examples/mm/gcc

for (( M=4500; M<=6000; M+=500)); 
do 
  for (( N=4500; N<=6000; N+=500)); 
  do 
    for (( O=4500; O<=6000; O+=500)); 
    do 
      if [ ! -f mm_paper_${M}_${N}_${O}.out ]                                      
      then
#clang -Wall -O0 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/cm/shared/apps/cuda10.0/toolkit/10.0.130 -Xopenmp-target -march=sm_37 mm_paper.c -o mm_paper_${M}_${N}_${O}.out -DM=${M} -DN=${N} -DO=${O};
        gcc -fopenmp -foffload=nvptx-none mm_paper.c -o mm_paper_${M}_${N}_${O}.out -DM=${M} -DN=${N} -DO=${O}
        ./mm_paper_${M}_${N}_${O}.out 6 > output_${M}_${N}_${O}
      fi
    done
  done
done


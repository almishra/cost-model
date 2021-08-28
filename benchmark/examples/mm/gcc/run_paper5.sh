cd /gpfs/projects/ChapmanGroup/alok/git/cost-model/benchmark/examples/mm/gcc

for (( M=1200; M<=4000; M+=500)); 
do 
  for (( N=1200; N<=4000; N+=500)); 
  do 
    for (( O=1200; O<=4200; O+=500)); 
    do 
      if [ ! -f mm_paper_${M}_${N}_${O}.out ]                                      
      then
#clang -Wall -O0 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/cm/shared/apps/cuda10.0/toolkit/10.0.130 -Xopenmp-target -march=sm_37 mm_paper.c -o mm_paper_${M}_${N}_${O}.out -DM=${M} -DN=${N} -DO=${O};
        gcc -fopenmp -foffload=nvptx-none mm_paper.c -o mm_paper_${M}_${N}_${O}.out -DM=${M} -DN=${N} -DO=${O}
        ./mm_paper_${M}_${N}_${O}.out 5 > output_${M}_${N}_${O}
      fi
    done
  done
done


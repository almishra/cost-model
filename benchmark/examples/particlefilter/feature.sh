for (( i=11000000; i<12000000; i+=1000000)); 
do 
  var=$(clang++ -fopenmp -Xclang -load -Xclang /gpfs/projects/ChapmanGroup/alok/git/cost-model/inst/opt/llvm/lib/InstructionCount.so -Xclang -plugin -Xclang -inst-count ex_particle_OPENMP_seq.cpp -DNparticles=$i -c 2> /dev/null | grep particleFilter);
  for j in $var
  do
    echo $j
    echo "-----"
  done
#  echo $var
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
#  echo $var | tail -n5
done

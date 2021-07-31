for ((n=500; n<=9500; n+=1000))
do
  for ((m=500; m<=9500; m+=1000))
  do
    num=$(( (n - 500) / 100 + (m - 500) / 1000))
    n1=$(( n - 1 ))
    m1=$(( m - 1 ))
    clang -Wall -O3 -fopenmp  -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/sw/summit/cuda/10.1.243 -Xopenmp-target -march=sm_70 jacobi.c -o jacobi${num} -DN=$n -DM=$m -DN1=$n1 -DM1=$m1
    echo -ne "$num "
  done
done
echo

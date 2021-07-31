cd /gpfs/alpine/scratch/alokm/csc401/cost-model/benchmark/examples/jacobi

./jacobi1 0 > output1.log &
./jacobi2 1 > output2.log &
./jacobi3 2 > output3.log &
./jacobi4 3 > output4.log &
./jacobi5 4 > output5.log &
./jacobi6 5 > output6.log &

wait

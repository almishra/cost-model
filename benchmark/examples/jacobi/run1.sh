cd /gpfs/alpine/scratch/alokm/csc401/cost-model/benchmark/examples/jacobi

./jacobi0 0 > output0.log &
./jacobi1 1 > output1.log &
./jacobi2 2 > output2.log &
./jacobi3 3 > output3.log &
./jacobi4 4 > output4.log &
./jacobi5 5 > output5.log &

wait

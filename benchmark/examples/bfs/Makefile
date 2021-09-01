# C compiler
CC = clang++
CC_FLAGS = -g -Wall -fopenmp -O2
GPU_FLAGS = -fopenmp-targets=nvptx64-nvidia-cuda  --cuda-path=$(CUDA_TOOLKIT_ROOT_DIR)

all: check-env bfs bfs_offload bfs_gcc

check-env:
#ifndef COMPUTE_CAPABILITY
#    $(error COMPUTE_CAPABILITY is undefined. Please export COMPUTE_CAPABILITY to proper the compute capability of your target device. For example V100 has a compute capability of 7.0. Then export COMPUTE_CAPABILITY=70)
#endif
ifndef CUDA_TOOLKIT_ROOT_DIR
    $(error CUDA_TOOLKIT_ROOT_DIR is undefined. Please set CUDA_TOOLKIT_ROOT_DIR to the install location of cuda.)
endif

bfs: bfs.cpp 
	$(CC) $(CC_FLAGS) bfs.cpp -o bfs 

bfs_offload: bfs.cpp
	$(CC) $(CC_FLAGS) $(GPU_FLAGS) -Xopenmp-target -march=sm_37 -DOMP_OFFLOAD bfs.cpp -o bfs_offload

bfs_gcc: bfs.cpp
	g++ -fopenmp -foffload=nvptx-none  -lm -fno-fast-math -fno-associative-math bfs.cpp -o bfs_gcc

input:
	cd inputGen;
	./graphgen 200000000 200M
	mv graph200M.txt ..
	cd ..

clean:
	rm -f result.txt bfs bfs_offload 

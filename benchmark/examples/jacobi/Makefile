CFLAGS = -Wall -O3 -fopenmp 
OFFLOAD_FLAG = -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=$(CUDA_TOOLKIT_ROOT_DIR)
CC=clang

all: check-env jacobi

check-env:
#ifndef COMPUTE_CAPABILITY
#	$(error COMPUTE_CAPABILITY is undefined. Please export COMPUTE_CAPABILITY to proper the compute capability of your target device. For example V100 has a compute capability of 7.0. Then export COMPUTE_CAPABILITY=70)
#endif
ifndef CUDA_TOOLKIT_ROOT_DIR
	$(error CUDA_TOOLKIT_ROOT_DIR is undefined. Please set CUDA_TOOLKIT_ROOT_DIR to the install location of cuda.)
endif

jacobi: jacobi.c
	$(CC) $(CFLAGS) $(OFFLOAD_FLAG) -Xopenmp-target -march=sm_70 jacobi.c -o jacobi

inst: jacobi.c
	$(CC) -fopenmp -Xclang -load -Xclang /gpfs/projects/ChapmanGroup/alok/git/cost-model/inst/opt/llvm/lib/InstructionCount.so -Xclang -plugin -Xclang -inst-count jacobi.c

clean:
	$(RM) jacobi

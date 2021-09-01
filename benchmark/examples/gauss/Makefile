CC = clang
CC_FLAGS =  -fopenmp -Wall
GPU_FLAGS = -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=$(CUDA_TOOLKIT_ROOT_DIR) -DOMP_OFFLOAD

all: check-env offload 

check-env:
ifndef CUDA_TOOLKIT_ROOT_DIR
	$(error CUDA_TOOLKIT_ROOT_DIR is undefined. Please set CUDA_TOOLKIT_ROOT_DIR to the install location of cuda.)
endif

offload: gs_openmp_target.c
	$(CC) $(CC_FLAGS) $(GPU_FLAGS) -Xopenmp-target -march=sm_37 -o gs_offload gs_openmp_target.c -lm

clean:
	$(RM) gs_offload


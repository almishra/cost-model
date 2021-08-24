CFLAGS = -Wall -O0 -fopenmp
OFFLOAD_FLAG = -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=$(CUDA_TOOLKIT_ROOT_DIR)
CC=clang

all: mm_paper

check-env:
ifndef CUDA_TOOLKIT_ROOT_DIR
    $(error CUDA_TOOLKIT_ROOT_DIR is undefined. Please set CUDA_TOOLKIT_ROOT_DIR to the install location of cuda.)
endif

mm_paper: mm_paper.c
	$(CC) $(CFLAGS) $(OFFLOAD_FLAG) -Xopenmp-target -march=sm_37 mm_paper.c -o mm_paper --cuda-gpu-arch=sm_37 -L$(CUDA_TOOLKIT_ROOT_DIR) -lcudart_static -ldl -lrt

clean:
	$(RM) mm_paper mm_paper_*.out

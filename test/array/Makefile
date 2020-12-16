CC=clang
CXX=clang++
CFLAGS=-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=sm_$(COMPUTE_CAPABILITY)
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

all: check-env $(PROGS)

check-env:
ifndef COMPUTE_CAPABILITY
    $(error COMPUTE_CAPABILITY is undefined. Please export COMPUTE_CAPABILITY to proper the compute capability of your target device. For example V100 has a compute capability of 7.0. Then export COMPUTE_CAPABILITY=70)
endif

%: %.c
	$(CC) $(CFLAGS)  -o $@ $<

clean:
	$(RM) $(PROGS)

CC=clang
CXX=clang++
CFLAGS=-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=sm_70
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS)  -o $@ $<

clean:
	$(RM) $(PROGS)

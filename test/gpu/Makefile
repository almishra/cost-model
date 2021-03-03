CC=clang
CXX=clang++
CFLAGS=-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=sm_70
LDFLAGS=-L$(CLANG_LIB)
SRCS = $(wildcard kernel*.cpp)
PROGS = $(patsubst kernel%.cpp,kernel%.out,$(SRCS))

all: $(PROGS)

kernel%.out: kernel%.cpp
	$(CXX) $(LDFLAGS) $(CFLAGS)  -o $@ $<

gen: gen.cpp
	$(CXX) gen.cpp -o gen

clean-src: clean
	$(RM) $(SRCS)

clean:
	$(RM) $(PROGS)

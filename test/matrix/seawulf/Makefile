CC=clang
CXX=clang++
#CFLAGS=-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=sm_37
CFLAGS=-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=sm_70
LDFLAGS=-L$(CLANG_LIB)
SRCS = $(wildcard matrix*.cpp)
PROGS = $(patsubst matrix%.cpp,matrix%.out,$(SRCS))

all: $(PROGS)

matrix%.out: matrix%.cpp
	$(CXX) $(LDFLAGS) $(CFLAGS) -o $@ $<

gen: gen.cpp
	$(CXX) gen.cpp -o gen

clean-src: clean
	$(RM) $(SRCS)

clean:
	$(RM) $(PROGS)

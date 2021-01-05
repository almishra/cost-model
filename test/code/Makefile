CC=clang
CXX=clang++
CFLAGS=-fopenmp
LDFLAGS=-L$(CLANG_LIB)
SRCS = $(wildcard test*.cpp)
PROGS = $(patsubst test%.cpp,test%.out,$(SRCS))

all: $(PROGS)

%.out: test%.cpp
	$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $<

gen: gen.cpp
	$(CXX) gen.cpp -o gen

clean-src: clean
	$(RM) $(SRCS)

clean:
	$(RM) $(PROGS)

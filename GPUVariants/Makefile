CC=clang
CXX=clang++

test: build
	./test.sh

all: first test 

first: clean
	mkdir .build && cd .build && CC=clang CXX=clang++ cmake -DLLVM_ENABLE_RTTI=true ..
	cd ..

build:
	cd .build && $(MAKE)

clean:
	$(RM) -r .build

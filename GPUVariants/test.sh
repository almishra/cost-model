clang -Xclang -load -Xclang .build/src/variants/libgpu-variants.so -Xclang -plugin -Xclang -gpu-variants -fopenmp -c example/test.c

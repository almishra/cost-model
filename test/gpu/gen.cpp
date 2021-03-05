#include <iostream>
#include <string.h>

#define CODE 5
//#define CODE 60

static char code[CODE][800] = {
  { "      for(k=0; k<LIM_1; k++) double_b[k] = 10.5;\n" },
  { "      for(k=0; k<LIM_1; k++) double_c[k] = double_a[k] * double_b[k];\n" },
  { "      for(k=0; k<LIM_2; k++) double_a[k] = double_c[k] / double_b[k];\n" },
  { "      for(k=0; k<LIM_3; k++) double_b[k] = double_a[k] + double_c[k];\n" },
  { "      for(k=0; k<LIM_1; k++) if(double_b[k] < double_a[k]) double_b[k]++;\n" },
};

void create_kernel(FILE *fp, int N)
{
  int num = N;
  std::string kernel_code;
  kernel_code.append("  for(int i=0; i<N1; i++) {\n");
  kernel_code.append("    for(int j=0; j<N2; j++) {\n");
  kernel_code.append("      int k;\n");
  kernel_code.append("      double double_c[N1] = {20.0};\n");
  int k=0;
  bool a = false, b = false;
  while(N>0 && k<CODE) {
    if((N&1)==1) {
      kernel_code.append(code[k]);
      if(strstr(code[k], "double_a") != NULL) a = true;
      if(strstr(code[k], "double_b") != NULL) b = true;
    }
    N>>=1;
    k++;
  }
  kernel_code.append("    }\n");
  kernel_code.append("  }\n");
  fprintf(fp, "void kernel_%d_parallel(FILE *fp, double double_a[N1], double double_b[N1], int num_dev, int dev) {\n", num);
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp parallel for \n");
  fprintf(fp, "%s", kernel_code.c_str());
  fprintf(fp, "  gettimeofday(&t2, NULL);\n");
  fprintf(fp, "  double runtime = (t2.tv_sec - t1.tv_sec);\n");
  fprintf(fp, "  runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;\n");
  fprintf(fp, "#pragma omp critical\n");
  fprintf(fp, "{\n");
  fprintf(fp, "  fprintf(fp, \"kernel_%d_parallel,%%.3f\\n\", runtime);\n", num);
  fprintf(fp, "  fflush(fp);\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "void kernel_%d_parallel_collapse(FILE *fp, double double_a[N1], double double_b[N1], int num_dev, int dev) {\n", num);
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp parallel for collapse(2)\n");
  fprintf(fp, "%s", kernel_code.c_str());
  fprintf(fp, "  gettimeofday(&t2, NULL);\n");
  fprintf(fp, "  double runtime = (t2.tv_sec - t1.tv_sec);\n");
  fprintf(fp, "  runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;\n");
  fprintf(fp, "#pragma omp critical\n");
  fprintf(fp, "{\n");
  fprintf(fp, "  fprintf(fp, \"kernel_%d_parallel_collapse,%%.3f\\n\", runtime);\n", num);
  fprintf(fp, "  fflush(fp);\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "void kernel_%d_target(FILE *fp, double double_a[N1], double double_b[N1], int num_dev, int dev) {\n", num);
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp target teams distribute parallel for ");
  if(a) fprintf(fp, "map(double_a[0:N1]) ");
  if(b) fprintf(fp, "map(double_b[0:N1]) ");
  fprintf(fp, "device(dev)\n");
  fprintf(fp, "%s", kernel_code.c_str());
  fprintf(fp, "  gettimeofday(&t2, NULL);\n");
  fprintf(fp, "  double runtime = (t2.tv_sec - t1.tv_sec);\n");
  fprintf(fp, "  runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;\n");
  fprintf(fp, "#pragma omp critical\n");
  fprintf(fp, "{\n");
  fprintf(fp, "  fprintf(fp, \"kernel_%d_target_%%d,%%.3f\\n\", dev, runtime);\n", num);
  fprintf(fp, "  fflush(fp);\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "void kernel_%d_target_collapse(FILE *fp, double double_a[N1], double double_b[N1], int num_dev, int dev) {\n", num);
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp target teams distribute parallel for collapse(2) ");
  if(a) fprintf(fp, "map(double_a[0:N1]) ");
  if(b) fprintf(fp, "map(double_b[0:N1]) ");
  fprintf(fp, "device(dev)\n");
  fprintf(fp, "%s", kernel_code.c_str());
  fprintf(fp, "  gettimeofday(&t2, NULL);\n");
  fprintf(fp, "  double runtime = (t2.tv_sec - t1.tv_sec);\n");
  fprintf(fp, "  runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;\n");
  fprintf(fp, "#pragma omp critical\n");
  fprintf(fp, "{\n");
  fprintf(fp, "  fprintf(fp, \"kernel_%d_target_collapse_%%d,%%.3f\\n\", dev, runtime);\n", num);
  fprintf(fp, "  fflush(fp);\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n\n");
}

void create(char *filename, int N1, int N2, int LIM_1, int LIM_2, int LIM_3)
{
  FILE *fp = fopen(filename, "w+");
  if(fp == NULL) {
    fprintf(stderr, "Error: Unable to create file %s\n", filename);
    return;
  }
  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <omp.h>\n");
  fprintf(fp, "#include <sys/time.h>\n\n");
  fprintf(fp, "#define N1 %d\n", N1);
  fprintf(fp, "#define N2 %d\n", N2);
  fprintf(fp, "#define LIM_1 %d\n", LIM_1);
  fprintf(fp, "#define LIM_2 %d\n", LIM_2);
  fprintf(fp, "#define LIM_3 %d\n\n", LIM_3);
  for(int i=0; i<1<<CODE; i++)
    create_kernel(fp, i);

  fprintf(fp, "typedef void (*Kernels) (FILE *fp, double a[N1], double b[N1], int num_dev, int dev);\n\n");
  fprintf(fp, "int main(int argc, char **argv)\n{\n");
  fprintf(fp, "  if(argc < 2) {\n");
  fprintf(fp, "    fprintf(stderr, \"Need Output log file name\");\n");
  fprintf(fp, "    return -1;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  FILE *fp = fopen(argv[1], \"w+\");\n");
  fprintf(fp, "  if(fp == NULL) {\n");
  fprintf(fp, "    fprintf(stderr, \"Error: Unable to create file %%s\\n\", argv[1]);\n");
  fprintf(fp, "    return -1;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  double double_a[N1], double_b[N1];\n");
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    double_a[i] = 10.0;\n");
  fprintf(fp, "    double_b[i] = 5.0;\n");
  fprintf(fp, "  }\n\n");
  fprintf(fp, "  Kernels kernel_parallel[] =\n");
  fprintf(fp, "  {\n");
  for(int i=0; i<(1<<CODE)-1; i++) {
    fprintf(fp, "    kernel_%d_parallel,\n", i);
  }
  fprintf(fp, "    kernel_%d_parallel\n", (1<<CODE)-1);
  fprintf(fp, "  };\n\n");
  fprintf(fp, "  Kernels kernel_parallel_collpase[] =\n");
  fprintf(fp, "  {\n");
  for(int i=0; i<(1<<CODE)-1; i++) {
    fprintf(fp, "    kernel_%d_parallel_collapse,\n", i);
  }
  fprintf(fp, "    kernel_%d_parallel_collapse\n", (1<<CODE)-1);
  fprintf(fp, "  };\n\n");
  fprintf(fp, "  Kernels kernel_target[] =\n");
  fprintf(fp, "  {\n");
  for(int i=0; i<(1<<CODE)-1; i++) {
    fprintf(fp, "    kernel_%d_target,\n", i);
  }
  fprintf(fp, "    kernel_%d_target\n", (1<<CODE)-1);
  fprintf(fp, "  };\n\n");
  fprintf(fp, "  Kernels kernel_target_collapse[] =\n");
  fprintf(fp, "  {\n");
  for(int i=0; i<(1<<CODE)-1; i++) {
    fprintf(fp, "    kernel_%d_target_collapse,\n", i);
  }
  fprintf(fp, "    kernel_%d_target_collapse\n", (1<<CODE)-1);
  fprintf(fp, "  };\n\n");
  fprintf(fp, "  int num_dev = omp_get_num_devices();\n");
  fprintf(fp, "  if(num_dev > 0) {\n");
  fprintf(fp, "    omp_set_num_threads(num_dev);\n");
  fprintf(fp, "    for(int i=0; i<num_dev; i++)\n");
  fprintf(fp, "#pragma omp target device(i)\n");
  fprintf(fp, "    {}\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "#pragma omp parallel for\n");
  fprintf(fp, "  for(int i=0; i<1<<%d; i++)\n", CODE);
  fprintf(fp, "    kernel_target[i](fp, double_a, double_b, num_dev, omp_get_thread_num());\n");
  fprintf(fp, "#pragma omp parallel for\n");
  fprintf(fp, "  for(int i=0; i<1<<%d; i++)\n", CODE);
  fprintf(fp, "    kernel_target_collapse[i](fp, double_a, double_b, num_dev, omp_get_thread_num());\n");
  fprintf(fp, "\n  return 0;\n}");

  fclose(fp);
}

int main()
{
/*  for(int N1=100; N1<=5000; N1+=500) {
    for(int N2=100; N2<=5000; N2+=500) {
      for(int L1=1; L1<=N1; L1+=800) {
        for(int L2=5; L2<=N1; L2+=325) {
          for(int L3=9; L3<=N1; L3+=617) {*/
  for(int N1=100; N1<=10000; N1+=1000) {
    for(int N2=100; N2<=10000; N2+=1000) {
      for(int L1=50; L1<=N1; L1+=1000) {
        for(int L2=50; L2<=N1; L2+=1000) {
          for(int L3=50; L3<=N1; L3+=1000) {
            char name[100];
            sprintf(name, "kernel_%d_%d_%d_%d_%d.cpp", N1, N2, L1, L2, L3);
            create(name, N1, N2, L1, L2, L3);
          }
        }
      }
    }
  }
  return 0;
}

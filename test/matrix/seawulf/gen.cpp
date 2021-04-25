#include <iostream>
#include <string.h>

void create_kernel(FILE *fp, bool simd, bool collapse, bool inner)
{
  std::string kernel_code;
  if(inner) {
    kernel_code.append("void inner_parallel_for");
  } else {
    kernel_code.append("void parallel_for");
    if(simd) kernel_code.append("_simd");
    if(collapse) kernel_code.append("_collapse");
  }
  kernel_code.append("(FILE *fp, int ROW1, int COL1, double A[ROW1*COL1], ");
  kernel_code.append("int ROW2, int COL2, double B[COL1*COL2], ");
  kernel_code.append("double C[ROW1*COL2], int num_dev, int dev) {\n");

  kernel_code.append("  struct timeval start, end;\n");
  kernel_code.append("  gettimeofday(&start, NULL);\n");
  kernel_code.append("#pragma omp target data map(to: A[0:ROW1*COL1], B[0:ROW2*COL2]) map(C[0:ROW1*COL2])\n");
  kernel_code.append("#pragma omp target teams distribute ");
  if(!inner) {
    kernel_code.append("parallel for ");
    if(simd)
      kernel_code.append("simd ");
    if(collapse)
      kernel_code.append("collapse(2)");
  }
  kernel_code.append("\n");

  kernel_code.append("  for(int i=0; i<ROW1; i++) {\n");
  if(inner)
    kernel_code.append("#pragma parallel for\n");
  kernel_code.append("    for(int j=0; j<COL2; j++) {\n");
  kernel_code.append("      double sum = 0;\n");
  kernel_code.append("      for(int k=0; k<COL1; k++)\n");
  kernel_code.append("        sum += A[i*COL1+k] * B[k*COL2+j];\n");
  kernel_code.append("      C[i*COL2+j] = sum;\n");
  kernel_code.append("    }\n");
  kernel_code.append("  }\n");
  kernel_code.append("  gettimeofday(&end, NULL);\n\n");
  kernel_code.append("  double runtime = (end.tv_sec  - start.tv_sec);\n");
  kernel_code.append("  runtime += (end.tv_usec - start.tv_usec) / 1000000.0;\n");
  kernel_code.append("#pragma omp critical\n");
  kernel_code.append("  {\n");
  //kernel_code.append("    fprintf(fp, \"matrix_%%d_%%d_%%d_%%d,%d,%d,%d,%%.4f\\n\", ROW1, COL1, COL1, COL2, runtime);\n", simd, collapse, inner);
  kernel_code.append("    fprintf(fp, \"matrix_%d_%d_%d_%d,%.4f\\n\", ROW1, COL1, ROW2, COL2, runtime);\n");
  kernel_code.append("  }\n");
  kernel_code.append("}\n");

  fprintf(fp, "%s\n\n", kernel_code.c_str());
}

void create(char *filename, int MIN, int MAX)
{
  FILE *fp = fopen(filename, "w+");
  if(fp == NULL) {
    fprintf(stderr, "Error: Unable to create file %s\n", filename);
    return;
  }
  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <omp.h>\n");
  fprintf(fp, "#include <sys/time.h>\n\n");
  fprintf(fp, "// %d, %d\n", MIN, MAX);

  create_kernel(fp, false, false, false);
  create_kernel(fp, false, true, false);
  create_kernel(fp, true, true, false);
  create_kernel(fp, false, false, true);
 
  for(int i = MIN; i <= MAX; i+=4000)
    for(int j = MIN; j <= MAX; j+=4000)
      for(int k = MIN; k <= MAX; k+=4000) {
      }

  //fprintf(fp, "typedef void (*Kernels) (FILE *fp, double a[N1], double b[N1], int num_dev, int dev);\n\n");
  fprintf(fp, "typedef void (*Kernels) (FILE *fp, int ROW1, int COL1, double A[ROW1*COL1], int ROW2, int COL2, double B[ROW2*COL2], double C[ROW1*COL2], int num_dev, int dev);\n\n");
  fprintf(fp, "int main(int argc, char **argv)\n{\n");
  fprintf(fp, "  if(argc < 2) {\n");
  fprintf(fp, "    fprintf(stderr, \"Need Output log file name\");\n");
  fprintf(fp, "    return -1;\n");
  fprintf(fp, "  }\n\n");
  fprintf(fp, "  Kernels kernel[] =\n");
  fprintf(fp, "  {\n");
  fprintf(fp, "    parallel_for,\n");
  fprintf(fp, "    parallel_for_collapse,\n");
  fprintf(fp, "    parallel_for_simd_collapse,\n");
  fprintf(fp, "    inner_parallel_for,\n");
  fprintf(fp, "  };\n\n");
  fprintf(fp, "  FILE *fp = fopen(argv[1], \"w+\");\n");
  fprintf(fp, "  if(fp == NULL) {\n");
  fprintf(fp, "    fprintf(stderr, \"Error: Unable to create file %%s\\n\", argv[1]);\n");
  fprintf(fp, "    return -1;\n");
  fprintf(fp, "  }\n");
  //fprintf(fp, "  double A[ROW1*COL1], B[COL1*COL2], C[ROW1*COL2];\n");
  fprintf(fp, "  double A[%d*%d], B[%d*%d], C[%d*%d];\n", MAX, MAX, MAX, MAX, MAX, MAX);
  fprintf(fp, "#pragma omp parallel for collapse(2)\n");
  fprintf(fp, "  for(int i=0; i<%d; i++)\n", MAX);
  fprintf(fp, "    for(int j=0; j<%d; j++)\n", MAX);
  fprintf(fp, "      A[i*%d+j] = random() %% 10;\n\n", MAX);
  fprintf(fp, "#pragma omp parallel for collapse(2)\n");
  fprintf(fp, "  for(int i=0; i<%d; i++)\n", MAX);
  fprintf(fp, "    for(int j=0; j<%d; j++)\n", MAX);
  fprintf(fp, "      B[i*%d+j] = random() %% 10;\n", MAX);
  fprintf(fp, "\n");
  fprintf(fp, "  int num_dev = omp_get_num_devices();\n");
  fprintf(fp, "  if(num_dev > 0) {\n");
  fprintf(fp, "    omp_set_num_threads(num_dev);\n");
  fprintf(fp, "    for(int i=0; i<num_dev; i++)\n");
  fprintf(fp, "#pragma omp target device(i)\n");
  fprintf(fp, "    {}\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "#pragma omp parallel for\n");
  fprintf(fp, "  for(int i=0; i<4; i++)\n");
  fprintf(fp, "    kernel[i](fp, 100, 200, A, 200, 300, B, C, num_dev, omp_get_thread_num());\n");
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
/*  for(int N1=100; N1<=10000; N1+=1000) {
    for(int N2=100; N2<=10000; N2+=1000) {
      for(int L1=50; L1<=N1; L1+=1000) {
        for(int L2=50; L2<=N1; L2+=1000) {
          for(int L3=50; L3<=N1; L3+=1000) {
            char name[100];
            sprintf(name, "matrix %d_%d_%d_%d_%d.cpp", N1, N2, L1, L2, L3);
            create(name, N1, N2, L1, L2, L3);
          }
        }
      }
    }
  }*/
  char name[100];
  sprintf(name, "matrix_%d_%d.cpp", 1000, 6000);
  create(name, 1000, 6000);
  return 0;
}

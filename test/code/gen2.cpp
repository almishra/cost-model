#include <stdio.h>
#include <string.h>

#define CODE 7
//#define CODE 60

static char code[CODE][800] = {
  { "      for(k=0; k<LIM_1; k++) int_c[k] = int_a[k] + int_b[k];" },
  { "      for(k=0; k<LIM_2; k++) int_a[k] = int_c[k] * int_b[k];" },
  { "      for(k=0; k<LIM_3; k++) int_c[k] += int_a[k];" },
  { "      for(k=0; k<LIM_1; k++) float_c[k] = float_a[k] + float_b[k];" },
  { "      for(k=0; k<LIM_2; k++) float_a[k] = float_c[k] * float_b[k];" },
  { "      for(k=0; k<LIM_3; k++) float_c[k] += float_a[k];" },
  { "      for(k=0; k<LIM_1; k++) int_b[k]++;" },
};

void create_kernel(FILE *fp, int N)
{
  fprintf(fp, "void kernel_%d(int int_a[N1], int int_b[N1], float float_a[N1], float float_b[N1]) {\n", N);
  fprintf(fp, "  printf(\"kernel_%d,\");\n", N);
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp parallel for\n");
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    for(int j=0; j<N2; j++) {\n");
  fprintf(fp, "      int k;\n");
  fprintf(fp, "      int int_c[N1] = {100};\n");
  fprintf(fp, "      float float_c[N1] = {100.0f};\n");
  int k=0;
  while(N>0 && k<CODE) {
    if((N&1)==1) fprintf(fp, "%s\n", code[k]);
    N>>=1;
    k++;
  }
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  gettimeofday(&t2, NULL);\n");
  fprintf(fp, "  double runtime = (t2.tv_sec - t1.tv_sec);\n");
  fprintf(fp, "  runtime += (t2.tv_usec - t1.tv_usec) / 1000000.0;\n");
  fprintf(fp, "  printf(\"%%.3f\\n\", runtime);\n\n");
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
  fprintf(fp, "#include <sys/time.h>\n\n");
  fprintf(fp, "#define N1 %d\n", N1);
  fprintf(fp, "#define N2 %d\n", N2);
  fprintf(fp, "#define LIM_1 %d\n", LIM_1);
  fprintf(fp, "#define LIM_2 %d\n", LIM_2);
  fprintf(fp, "#define LIM_3 %d\n", LIM_3);
  for(int i=0; i<1<<CODE; i++)
    create_kernel(fp, i);
  fprintf(fp, "int main()\n{\n");
  fprintf(fp, "  int int_a[N1], int_b[N1];\n");
  fprintf(fp, "  float float_a[N1], float_b[N1];\n");
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    int_a[i] = 10;\n");
  fprintf(fp, "    int_b[i] = 5;\n");
  fprintf(fp, "    float_a[i] = int_a[i] * 2.5;\n");
  fprintf(fp, "    float_b[i] = int_b[i] * 3.5;\n");
  fprintf(fp, "  }\n\n");
  for(int i=0; i<1<<CODE; i++)
    fprintf(fp, "  kernel_%d(int_a, int_b, float_a, float_b);\n", i);
  fprintf(fp, "\n  return 0;\n}");

  fclose(fp);
}

int main()
{

  for(int N1=100; N1<=5000; N1+=500) {
    for(int N2=100; N2<=5000; N2+=500) {
      char name[100];
      sprintf(name, "kernel_%d_%d.cpp", N1, N2);
      create(name, 100, 100, 100, 100, 100);
    }
  }
/*#pragma omp parallel for num_threads(12)
  for(long i=1; i<1L<<(CODE); i++) {
  //for(long i=1; i<10; i++) {
    char name_int[100];
    sprintf(name_int, "test_%lu.cpp", i);
    gen(i, name_int);
  }
  */
/*  for(int i=1; i<1<<(CODE); i++) {
    char name_int[100];
    sprintf(name_int, "test_int_%d.cpp", i);
    gen(i, "int", name_int);
  }
  for(int i=1; i<1<<(CODE); i++) {
    char name_long[100];
    sprintf(name_long, "test_long_%d.cpp", i);
    gen(i, "long", name_long);
  }
  for(int i=1; i<1<<(CODE); i++) {
    char name_char[100];
    sprintf(name_char, "test_char_%d.cpp", i);
    gen(i, "char", name_char);
  }
  for(int i=1; i<1<<(CODE); i++) {
    char name_short[100];
    sprintf(name_short, "test_short_%d.cpp", i);
    gen(i, "short", name_short);
  }
  for(int i=1; i<1<<(CODE-6); i++) {
    char name_float[100];
    sprintf(name_float, "test_float_%d.cpp", i);
    gen(i, "float", name_float);
  }
  for(int i=1; i<1<<(CODE-6); i++) {
    char name_double[100];
    sprintf(name_double, "test_double_%d.cpp", i);
    gen(i, "double", name_double);
  }*/
  return 0;
}

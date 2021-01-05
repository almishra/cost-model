#include <stdio.h>
#include <string.h>

#define CODE 10
void gen(int N, char *type1, char *type2, char *filename)
{
  FILE *fp = fopen(filename, "w+");
  if(fp == NULL) {
    fprintf(stderr, "Error: Unable to create file %s\n", filename);
    return;
  }
  char code[CODE][800];

  sprintf(code[0], "      for(k=0; k<%s_N1; k++) %s_c[k] = %s_a[k] + %s_b[k];", type1, type1, type1, type1);
  sprintf(code[1], "      for(k=0; k<%s_N2; k++) %s_b[k] = %s_a[k] - %s_c[k];", type1, type1, type1, type1);
  sprintf(code[2], "      for(k=0; k<%s_N3; k++) %s_a[k] = %s_c[k] * %s_b[k];", type1, type1, type1, type1);
  sprintf(code[3], "      for(k=0; k<%s_N4; k++) %s_c[k] = %s_a[k] / 179;", type1, type1, type1);
  if(strcmp(type1,"float") != 0 && strcmp(type1,"double") != 0) {
    sprintf(code[4], "      for(k=0; k<%s_N5; k++) %s_b[k] = %s_a[k] | %s_c[k];", type1, type1, type1, type1);
    sprintf(code[5], "      for(k=0; k<%s_N6; k++) %s_c[k] = %s_a[k] ^ %s_b[k];", type1, type1, type1, type1);
    sprintf(code[6], "      for(k=0; k<%s_N7; k++) %s_a[k] = %s_b[k] & %s_c[k];", type1, type1, type1, type1);
    sprintf(code[7], "      for(k=0; k<%s_N8; k++) %s_c[k] = %s_a[k] %% 109;", type1, type1, type1);
    sprintf(code[8], "      for(k=0; k<%s_N9; k++) %s_a[k] = %s_a[k] << 2;", type1, type1, type1);
    sprintf(code[9], "      for(k=0; k<%s_N10; k++) %s_b[k] = %s_b[k] >> 2;", type1, type1, type1);
  } else {
    sprintf(code[4], "");
    sprintf(code[5], "");
    sprintf(code[6], "");
    sprintf(code[7], "");
    sprintf(code[8], "");
    sprintf(code[9], "");
  }
  //printf("N=%d\n", N);
  int N1 = 100;
  int N2 = 100;
  int N3 = 100;
  int N4 = 100;
  int N5 = 100;
  int N6 = 100;
  int N7 = 100;
  int N8 = 100;
  int N9 = 100;
  int N10 = 100;
  fprintf(fp, "#include <stdio.h>\n#include <sys/time.h>\n\n");
  fprintf(fp, "#define N1 100\n");
  fprintf(fp, "#define N2 100\n");
  fprintf(fp, "#define %s_N1 %d\n", type1, N1);
  fprintf(fp, "#define %s_N2 %d\n", type1, N2);
  fprintf(fp, "#define %s_N3 %d\n", type1, N3);
  fprintf(fp, "#define %s_N4 %d\n", type1, N4);
  fprintf(fp, "#define %s_N5 %d\n", type1, N5);
  fprintf(fp, "#define %s_N6 %d\n", type1, N6);
  fprintf(fp, "#define %s_N7 %d\n", type1, N7);
  fprintf(fp, "#define %s_N8 %d\n", type1, N8);
  fprintf(fp, "#define %s_N9 %d\n", type1, N9);
  fprintf(fp, "#define %s_N10 %d\n\n", type1, N10);
  fprintf(fp, "int main()\n{\n");
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  %s %s_a[N1];\n", type1, type1);
  fprintf(fp, "  %s %s_b[N1];\n\n", type1, type1);
  fprintf(fp, "  %s %s_a[N1];\n\n", type2, type2);
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    %s_a[i] = 10;\n    %s_b[i] = 5;\n", type1, type1);
  fprintf(fp, "  }\n\n");
  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp parallel for\n");
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    for(int j=0; j<N2; j++) {\n");
  fprintf(fp, "      int k;\n");
  fprintf(fp, "      %s %s_c[N1] = {100};\n", type1, type1);
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
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n");

  /*while(N>0 && k<5) {
    //printf("%d", N&1);
    if((N&1)==1) printf("%s\n", code[k]);
    N>>=1;
    k++;
  }
  printf("\n");*/
  fclose(fp);
}

int main()
{
  for(int i=1; i<9; i++) {
    char name_int[100];
    sprintf(name_int, "test_int_%d.cpp", i);
    gen(i, "int", "float", name_int);
  }
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

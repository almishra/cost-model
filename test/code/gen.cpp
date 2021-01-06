#include <stdio.h>
#include <string.h>

#define CODE 60
void gen(int N, char *filename)
{
  FILE *fp = fopen(filename, "w+");
  if(fp == NULL) {
    fprintf(stderr, "Error: Unable to create file %s\n", filename);
    return;
  }
  char code[CODE][800];

  sprintf(code[0], "      for(k=0; k<LIM_1; k++) int_c[k] = int_a[k] + int_b[k];");
  sprintf(code[1], "      for(k=0; k<LIM_2; k++) int_b[k] = int_a[k] - int_c[k];");
  sprintf(code[2], "      for(k=0; k<LIM_3; k++) int_a[k] = int_c[k] * int_b[k];");
  sprintf(code[3], "      for(k=0; k<LIM_4; k++) int_c[k] = int_a[k] / 179;");
  sprintf(code[4], "      for(k=0; k<LIM_1; k++) int_c[k] = int_a[k] %% 109;");
  sprintf(code[5], "      for(k=0; k<LIM_2; k++) int_a[k] = int_a[k] << 2;");
  sprintf(code[6], "      for(k=0; k<LIM_3; k++) int_b[k] = int_b[k] >> 2;");
  sprintf(code[7], "      for(k=0; k<LIM_4; k++) int_c[k] += int_a[k];");
  sprintf(code[8], "      for(k=0; k<LIM_1; k++) int_b[k] -= int_a[k];");
  sprintf(code[9], "      for(k=0; k<LIM_2; k++) int_a[k] *= int_c[k];");
  sprintf(code[10], "      for(k=0; k<LIM_3; k++) int_c[k] /= 179;");
  sprintf(code[11], "      for(k=0; k<LIM_4; k++) int_c[k] %%= 109;");
  sprintf(code[12], "      for(k=0; k<LIM_1; k++) int_a[k] <<= 2;");
  sprintf(code[13], "      for(k=0; k<LIM_2; k++) int_b[k] >>= 5;");
  sprintf(code[14], "      for(k=0; k<LIM_1; k++) long_c[k] = long_a[k] + long_b[k];");
  sprintf(code[15], "      for(k=0; k<LIM_2; k++) long_b[k] = long_a[k] - long_c[k];");
  sprintf(code[16], "      for(k=0; k<LIM_3; k++) long_a[k] = long_c[k] * long_b[k];");
  sprintf(code[17], "      for(k=0; k<LIM_4; k++) long_c[k] = long_a[k] / 179;");
  sprintf(code[18], "      for(k=0; k<LIM_1; k++) long_c[k] = long_a[k] %% 109;");
  sprintf(code[19], "      for(k=0; k<LIM_2; k++) long_a[k] = long_a[k] << 2;");
  sprintf(code[20], "      for(k=0; k<LIM_3; k++) long_b[k] = long_b[k] >> 2;");
  sprintf(code[21], "      for(k=0; k<LIM_4; k++) long_c[k] += long_a[k];");
  sprintf(code[22], "      for(k=0; k<LIM_1; k++) long_b[k] -= long_a[k];");
  sprintf(code[23], "      for(k=0; k<LIM_2; k++) long_a[k] *= long_c[k];");
  sprintf(code[24], "      for(k=0; k<LIM_3; k++) long_c[k] /= (long)179;");
  sprintf(code[25], "      for(k=0; k<LIM_4; k++) long_c[k] %%= (long)109;");
  sprintf(code[26], "      for(k=0; k<LIM_1; k++) long_a[k] <<= 2;");
  sprintf(code[27], "      for(k=0; k<LIM_2; k++) long_b[k] >>= 5;");
  sprintf(code[28], "      for(k=0; k<LIM_1; k++) double_c[k] = double_a[k] + double_b[k];");
  sprintf(code[29], "      for(k=0; k<LIM_2; k++) double_b[k] = double_a[k] - double_c[k];");
  sprintf(code[30], "      for(k=0; k<LIM_3; k++) double_a[k] = double_c[k] * double_b[k];");
  sprintf(code[31], "      for(k=0; k<LIM_4; k++) double_c[k] = double_a[k] / (double)179.0;");
  sprintf(code[32], "      for(k=0; k<LIM_1; k++) double_c[k] += double_a[k];");
  sprintf(code[33], "      for(k=0; k<LIM_2; k++) double_b[k] -= double_a[k];");
  sprintf(code[34], "      for(k=0; k<LIM_3; k++) double_a[k] *= double_c[k];");
  sprintf(code[35], "      for(k=0; k<LIM_4; k++) double_c[k] /= (double)179;");
  sprintf(code[36], "      for(k=0; k<LIM_1; k++) float_c2[k] = float_a[k] + float_b[k];");
  sprintf(code[37], "      for(k=0; k<LIM_2; k++) float_b[k] = float_a[k] - float_c2[k];");
  sprintf(code[38], "      for(k=0; k<LIM_3; k++) float_a[k] = float_c2[k] * float_b[k];");
  sprintf(code[39], "      for(k=0; k<LIM_4; k++) float_c2[k] = float_a[k] / (float)179.0;");
  sprintf(code[40], "      for(k=0; k<LIM_1; k++) float_c2[k] += float_a[k];");
  sprintf(code[41], "      for(k=0; k<LIM_2; k++) float_b[k] -= float_a[k];");
  sprintf(code[42], "      for(k=0; k<LIM_3; k++) float_a[k] *= float_c2[k];");
  sprintf(code[43], "      for(k=0; k<LIM_4; k++) float_c2[k] /= (float)179.0;");
  sprintf(code[44], "      for(k=0; k<LIM_1; k++) int_b[k]++;");
  sprintf(code[45], "      for(k=0; k<LIM_2; k++) int_a[k]--;");
  sprintf(code[46], "      for(k=0; k<LIM_2; k++) ++int_b[k];");
  sprintf(code[47], "      for(k=0; k<LIM_3; k++) --int_a[k];");
  sprintf(code[48], "      for(k=0; k<LIM_1; k++) long_b[k]++;");
  sprintf(code[49], "      for(k=0; k<LIM_2; k++) long_a[k]--;");
  sprintf(code[50], "      for(k=0; k<LIM_3; k++) ++long_b[k];");
  sprintf(code[51], "      for(k=0; k<LIM_4; k++) --long_a[k];");
  sprintf(code[52], "      for(k=0; k<LIM_1; k++) --double_b[k];");
  sprintf(code[53], "      for(k=0; k<LIM_2; k++) ++double_a[k];");
  sprintf(code[54], "      for(k=0; k<LIM_3; k++) double_b[k]++;");
  sprintf(code[55], "      for(k=0; k<LIM_4; k++) double_a[k]--;");
  sprintf(code[56], "      for(k=0; k<LIM_1; k++) --float_b[k];");
  sprintf(code[57], "      for(k=0; k<LIM_2; k++) ++float_a[k];");
  sprintf(code[58], "      for(k=0; k<LIM_3; k++) float_b[k]++;");
  sprintf(code[59], "      for(k=0; k<LIM_4; k++) float_a[k]--;");


 // sprintf(code[0], "      for(k=0; k<%s_N1; k++) %s_c[k] = %s_a[k] + %s_b[k];", type1, type1, type1, type1);
 // sprintf(code[1], "      for(k=0; k<%s_N2; k++) %s_b[k] = %s_a[k] - %s_c[k];", type1, type1, type1, type1);
 // sprintf(code[2], "      for(k=0; k<%s_N3; k++) %s_a[k] = %s_c[k] * %s_b[k];", type1, type1, type1, type1);
 // sprintf(code[3], "      for(k=0; k<%s_N4; k++) %s_c[k] = %s_a[k] / 179;", type1, type1, type1);
 // if(strcmp(type1,"float") != 0 && strcmp(type1,"double") != 0) {
 //   sprintf(code[4], "      for(k=0; k<%s_N5; k++) %s_b[k] = %s_a[k] | %s_c[k];", type1, type1, type1, type1);
 //   sprintf(code[5], "      for(k=0; k<%s_N6; k++) %s_c[k] = %s_a[k] ^ %s_b[k];", type1, type1, type1, type1);
 //   sprintf(code[6], "      for(k=0; k<%s_N7; k++) %s_a[k] = %s_b[k] & %s_c[k];", type1, type1, type1, type1);
 //   sprintf(code[7], "      for(k=0; k<%s_N8; k++) %s_c[k] = %s_a[k] %% 109;", type1, type1, type1);
 //   sprintf(code[8], "      for(k=0; k<%s_N9; k++) %s_a[k] = %s_a[k] << 2;", type1, type1, type1);
 //   sprintf(code[9], "      for(k=0; k<%s_N10; k++) %s_b[k] = %s_b[k] >> 2;", type1, type1, type1);
 // } else {
 //   sprintf(code[4], "");
 //   sprintf(code[5], "");
 //   sprintf(code[6], "");
 //   sprintf(code[7], "");
 //   sprintf(code[8], "");
 //   sprintf(code[9], "");
 // }
  //printf("N=%d\n", N);
  int N1 = 4000;
  int N2 = 100;
  int LIM_1 = 100;
  int LIM_2 = 100;
  int LIM_3 = 100;
  int LIM_4 = 100;
  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <sys/time.h>\n\n");
  fprintf(fp, "#define N1 %d\n", N1);
  fprintf(fp, "#define N2 %d\n", N2);
  fprintf(fp, "#define LIM_1 %d\n", LIM_1);
  fprintf(fp, "#define LIM_2 %d\n", LIM_2);
  fprintf(fp, "#define LIM_3 %d\n", LIM_3);
  fprintf(fp, "#define LIM_4 %d\n", LIM_4);
  fprintf(fp, "int main()\n{\n");
  fprintf(fp, "  struct timeval t1, t2;\n");
  fprintf(fp, "  int int_a[N1], int_b[N1];\n");
  fprintf(fp, "  long long_a[N1], long_b[N1];\n");
  fprintf(fp, "  float float_a[N1], float_b[N1];\n");
  fprintf(fp, "  double double_a[N1], double_b[N1];\n");
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    int_a[i] = 10;\n");
  fprintf(fp, "    int_b[i] = 5;\n");
  fprintf(fp, "    long_a[i] = (long)1 << 40;\n");
  fprintf(fp, "    long_b[i] = (long)1 << 55;\n");
  fprintf(fp, "    float_a[i] = int_a[i] * 2.5;\n");
  fprintf(fp, "    float_b[i] = int_b[i] * 3.5;\n");
  fprintf(fp, "    double_a[i] = long_a[i] * 2.5;\n");
  fprintf(fp, "    double_b[i] = long_b[i] * 4.5;\n");
  fprintf(fp, "  }\n\n");

  fprintf(fp, "  gettimeofday(&t1, NULL);\n");
  fprintf(fp, "#pragma omp parallel for\n");
  fprintf(fp, "  for(int i=0; i<N1; i++) {\n");
  fprintf(fp, "    for(int j=0; j<N2; j++) {\n");
  fprintf(fp, "      int k;\n");
  fprintf(fp, "      int int_c[N1] = {100};\n");
  fprintf(fp, "      long long_c[N1] = {100l};\n");
  fprintf(fp, "      float float_c[N1] = {100.0f};\n");
  fprintf(fp, "      double double_c[N1] = {100.0L};\n");
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

  fclose(fp);
}

int main()
{
#pragma omp parallel for num_threads(12)
  for(long i=1; i<1L<<(CODE); i++) {
  //for(long i=1; i<10000; i++) {
    char name_int[100];
    sprintf(name_int, "test_%lu.cpp", i);
    gen(i, name_int);
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

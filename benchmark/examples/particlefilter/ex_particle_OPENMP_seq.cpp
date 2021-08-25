/**
 * @file ex_particle_OPENMP_seq.c
 * @author Michael Trotter & Matt Goodrum
 * @brief Particle filter implementation in C/OpenMP 
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>
#include <limits.h>
#include <string.h>
#include <ctime>
#define PI 3.1415926535897932
/**
  @var M value for Linear Congruential Generator (LCG); use GCC's value
  */
long M = INT_MAX;
/**
  @var A value for LCG
  */
int A = 1103515245;
/**
  @var C value for LCG
  */
int C = 12345;
/*****************************
 *GET_TIME
 *returns a long int representing the time
 *****************************/
long long get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000000) + tv.tv_usec;
}
// Returns the number of seconds elapsed between the two specified times
float elapsed_time(long long start_time, long long end_time) {
  return (float) (end_time - start_time) / (1000 * 1000);
}
/** 
 * Takes in a double and returns an integer that approximates to that double
 * @return if the mantissa < .5 => return value < input value; else return value > input value
 */
double roundDouble(double value) {
  int newValue = (int)(value);
  if(value - newValue < .5)
    return newValue;
  else
    return newValue++;
}
/**
 * Set values of the 3D array to a newValue if that value is equal to the testValue
 * @param testValue The value to be replaced
 * @param newValue The value to replace testValue with
 * @param array3D The image vector
 * @param dimX The x dimension of the frame
 * @param dimY The y dimension of the frame
 * @param dimZ The number of frames
 */
void setIf(int testValue, int newValue, int * array3D, int * dimX, int * dimY, int * dimZ) {
  int x, y, z;
  for(x = 0; x < *dimX; x++) {
    for(y = 0; y < *dimY; y++) {
      for(z = 0; z < *dimZ; z++) {
        if(array3D[x * *dimY * *dimZ+y * *dimZ + z] == testValue)
          array3D[x * *dimY * *dimZ + y * *dimZ + z] = newValue;
      }
    }
  }
}
/**
 * Generates a uniformly distributed random number using the provided seed and GCC's settings for the Linear Congruential Generator (LCG)
 * @see http://en.wikipedia.org/wiki/Linear_congruential_generator
 * @note This function is thread-safe
 * @param seed The seed array
 * @param index The specific index of the seed to be advanced
 * @return a uniformly distributed number [0, 1)
 */
double randu(int * seed, int index)
{
  int num = A*seed[index] + C;
  seed[index] = num % M;
  return fabs(seed[index]/((double) M));
}
/**
 * Generates a normally distributed random number using the Box-Muller transformation
 * @note This function is thread-safe
 * @param seed The seed array
 * @param index The specific index of the seed to be advanced
 * @return a double representing random number generated using the Box-Muller algorithm
 * @see http://en.wikipedia.org/wiki/Normal_distribution, section computing value for normal random distribution
 */
double randn(int * seed, int index) {
  /*Box-Muller algorithm*/
  double u = randu(seed, index);
  double v = randu(seed, index);
  double cosine = cos(2*PI*v);
  double rt = -2*log(u);
  return sqrt(rt)*cosine;
}
/**
 * Sets values of 3D matrix using randomly generated numbers from a normal distribution
 * @param array3D The video to be modified
 * @param dimX The x dimension of the frame
 * @param dimY The y dimension of the frame
 * @param dimZ The number of frames
 * @param seed The seed array
 */
void addNoise(int * array3D, int * dimX, int * dimY, int * dimZ, int * seed) {
  int x, y, z;
  for(x = 0; x < *dimX; x++) {
    for(y = 0; y < *dimY; y++) {
      for(z = 0; z < *dimZ; z++) {
        array3D[x * *dimY * *dimZ + y * *dimZ + z] = array3D[x * *dimY * *dimZ + y * *dimZ + z] + (int)(5*randn(seed, 0));
      }
    }
  }
}
/**
 * Fills a radius x radius matrix representing the disk
 * @param disk The pointer to the disk to be made
 * @param radius  The radius of the disk to be made
 */
void strelDisk(int * disk, int radius)
{
  int diameter = radius*2 - 1;
  int x, y;
  for(x = 0; x < diameter; x++) {
    for(y = 0; y < diameter; y++) {
      double distance = sqrt(pow((double)(x-radius+1),2) + pow((double)(y-radius+1),2));
      if(distance < radius)
        disk[x*diameter + y] = 1;
    }
  }
}
/**
 * Dilates the provided video
 * @param matrix The video to be dilated
 * @param posX The x location of the pixel to be dilated
 * @param posY The y location of the pixel to be dilated
 * @param poxZ The z location of the pixel to be dilated
 * @param dimX The x dimension of the frame
 * @param dimY The y dimension of the frame
 * @param dimZ The number of frames
 * @param error The error radius
 */
void dilate_matrix(int * matrix, int posX, int posY, int posZ, int dimX, int dimY, int dimZ, int error)
{
  int startX = posX - error;
  while(startX < 0)
    startX++;
  int startY = posY - error;
  while(startY < 0)
    startY++;
  int endX = posX + error;
  while(endX > dimX)
    endX--;
  int endY = posY + error;
  while(endY > dimY)
    endY--;
  int x,y;
  for(x = startX; x < endX; x++) {
    for(y = startY; y < endY; y++) {
      double distance = sqrt( pow((double)(x-posX),2) + pow((double)(y-posY),2) );
      if(distance < error)
        matrix[x*dimY*dimZ + y*dimZ + posZ] = 1;
    }
  }
}

/**
 * Dilates the target matrix using the radius as a guide
 * @param matrix The reference matrix
 * @param dimX The x dimension of the video
 * @param dimY The y dimension of the video
 * @param dimZ The z dimension of the video
 * @param error The error radius to be dilated
 * @param newMatrix The target matrix
 */
void imdilate_disk(int * matrix, int dimX, int dimY, int dimZ, int error, int * newMatrix)
{
  int x, y, z;
  for(z = 0; z < dimZ; z++) {
    for(x = 0; x < dimX; x++) {
      for(y = 0; y < dimY; y++) {
        if(matrix[x*dimY*dimZ + y*dimZ + z] == 1) {
          dilate_matrix(newMatrix, x, y, z, dimX, dimY, dimZ, error);
        }
      }
    }
  }
}
/**
 * Fills a 2D array describing the offsets of the disk object
 * @param se The disk object
 * @param numOnes The number of ones in the disk
 * @param neighbors The array that will contain the offsets
 * @param radius The radius used for dilation
 */
void getneighbors(int * se, int numOnes, double * neighbors, int radius)
{
  int x, y;
  int neighY = 0;
  int center = radius - 1;
  int diameter = radius*2 -1;
  for(x = 0; x < diameter; x++) {
    for(y = 0; y < diameter; y++) {
      if(se[x*diameter + y]) {
        neighbors[neighY*2] = (int)(y - center);
        neighbors[neighY*2 + 1] = (int)(x - center);
        neighY++;
      }
    }
  }
  //printf("neighY : %d\n", neighY);
}
/**
 * The synthetic video sequence we will work with here is composed of a
 * single moving object, circular in shape (fixed radius)
 * The motion here is a linear motion
 * the foreground intensity and the backgrounf intensity is known
 * the image is corrupted with zero mean Gaussian noise
 * @param I The video itself
 * @param IszX The x dimension of the video
 * @param IszY The y dimension of the video
 * @param Nfr The number of frames of the video
 * @param seed The seed array used for number generation
 */
void videoSequence(int * I, int IszX, int IszY, int Nfr, int * seed)
{
  int k;
  int max_size = IszX*IszY*Nfr;
  /*get object centers*/
  int x0 = (int)roundDouble(IszY/2.0);
  int y0 = (int)roundDouble(IszX/2.0);
  I[x0 *IszY *Nfr + y0 * Nfr  + 0] = 1;

  /*move point*/
  int xk, yk, pos;
  for(k = 1; k < Nfr; k++) {
    xk = abs(x0 + (k-1));
    yk = abs(y0 - 2*(k-1));
    pos = yk * IszY * Nfr + xk *Nfr + k;
    if(pos >= max_size)
      pos = 0;
    I[pos] = 1;
  }

  /*dilate matrix*/
  int * newMatrix = (int *)malloc(sizeof(int)*IszX*IszY*Nfr);
  imdilate_disk(I, IszX, IszY, Nfr, 5, newMatrix);
  int x, y;
  for(x = 0; x < IszX; x++) {
    for(y = 0; y < IszY; y++) {
      for(k = 0; k < Nfr; k++) {
        I[x*IszY*Nfr + y*Nfr + k] = newMatrix[x*IszY*Nfr + y*Nfr + k];
      }
    }
  }
  free(newMatrix);

  /*define background, add noise*/
  setIf(0, 100, I, &IszX, &IszY, &Nfr);
  setIf(1, 228, I, &IszX, &IszY, &Nfr);
  /*add noise*/
  addNoise(I, &IszX, &IszY, &Nfr, seed);
}
/**
 * Determines the likelihood sum based on the formula: SUM( (IK[IND] - 100)^2 - (IK[IND] - 228)^2)/ 100
 * @param I The 3D matrix
 * @param ind The current ind array
 * @param numOnes The length of ind array
 * @return A double representing the sum
 */
double calcLikelihoodSum(int * I, int * ind, int numOnes)
{
  double likelihoodSum = 0.0;
  int y;
  for(y = 0; y < numOnes; y++)
    likelihoodSum += (pow((I[ind[y]] - 100),2) - pow((I[ind[y]]-228),2))/50.0;
  return likelihoodSum;
}
/**
 * Finds the first element in the CDF that is greater than or equal to the provided value and returns that index
 * @note This function uses sequential search
 * @param CDF The CDF
 * @param lengthCDF The length of CDF
 * @param value The value to be found
 * @return The index of value in the CDF; if value is never found, returns the last index
 */
int findIndex(double * CDF, int lengthCDF, double value) {
  int index = -1;
  int x;
  for(x = 0; x < lengthCDF; x++) {
    if(CDF[x] >= value) {
      index = x;
      break;
    }
  }
  if(index == -1) {
    return lengthCDF-1;
  }
  return index;
}
/**
 * Finds the first element in the CDF that is greater than or equal to the provided value and returns that index
 * @note This function uses binary search before switching to sequential search
 * @param CDF The CDF
 * @param beginIndex The index to start searching from
 * @param endIndex The index to stop searching
 * @param value The value to find
 * @return The index of value in the CDF; if value is never found, returns the last index
 * @warning Use at your own risk; not fully tested
 */
int findIndexBin(double * CDF, int beginIndex, int endIndex, double value)
{
  if(endIndex < beginIndex)
    return -1;
  int middleIndex = beginIndex + ((endIndex - beginIndex)/2);
  /*check the value*/
  if(CDF[middleIndex] >= value)
  {
    /*check that it's good*/
    if(middleIndex == 0)
      return middleIndex;
    else if(CDF[middleIndex-1] < value)
      return middleIndex;
    else if(CDF[middleIndex-1] == value)
    {
      while(middleIndex > 0 && CDF[middleIndex-1] == value)
        middleIndex--;
      return middleIndex;
    }
  }
  if(CDF[middleIndex] > value)
    return findIndexBin(CDF, beginIndex, middleIndex+1, value);
  return findIndexBin(CDF, middleIndex-1, endIndex, value);
}
/**
 * The implementation of the particle filter using OpenMP for many frames
 * @see http://openmp.org/wp/
 * @note This function is designed to work with a video of several frames. In addition, it references a provided MATLAB function which takes the video, the objxy matrix and the x and y arrays as arguments and returns the likelihoods
 * @param I The video to be run
 * @param IszX The x dimension of the video
 * @param IszY The y dimension of the video
 * @param Nfr The number of frames
 * @param seed The seed array used for random number generation
 * @param Nparticles The number of particles to be used
 */
//void particleFilter(int * I, int IszX, int IszY, int Nfr, int * seed, int Nparticles) 
void particleFilter(int * I, int IszX, int IszY, int Nfr, int * seed)//, int Nparticles) 
{
  int max_size = IszX*IszY*Nfr;
  long long start = get_time();
  //original particle centroid
  double xe = roundDouble(IszY/2.0);
  double ye = roundDouble(IszX/2.0);

  //expected object locations, compared to center
  int radius = 5;
  int diameter = radius*2 - 1;
  //int * disk = (int *)malloc(diameter*diameter*sizeof(int));
  int disk[diameter*diameter];
  strelDisk(disk, radius);
  int countOnes = 0;
  int x, y;
  for(x = 0; x < diameter; x++) {
    for(y = 0; y < diameter; y++) {
      if(disk[x*diameter + y] == 1)
        countOnes++;
    }
  }
  //printf("countOnes : %d\n", countOnes);
  //double * objxy = (double *)malloc(countOnes*2*sizeof(double));
  double objxy[countOnes*2];
  getneighbors(disk, countOnes, objxy, radius);

  double weights[Nparticles];
  double likelihood[Nparticles];
  double arrayX[Nparticles];
  double arrayY[Nparticles];
  double xj[Nparticles];
  double yj[Nparticles];
  double CDF[Nparticles];
  double u[Nparticles];
  int ind[countOnes*Nparticles];

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
#pragma omp target teams distribute parallel for map(weights[0:Nparticles])
  for(int x = 0; x < Nparticles; x++) {
    weights[x] = 1/((double)(Nparticles));
  }
  gettimeofday(&t2, NULL);
  printf("Kernel1,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));

  gettimeofday(&t1, NULL);
#pragma omp target teams distribute parallel for map(arrayX[0:Nparticles], arrayY[0:Nparticles], xe, ye)
  for(x = 0; x < Nparticles; x++) {
    arrayX[x] = xe;
    arrayY[x] = ye;
  }
  gettimeofday(&t2, NULL);
  printf("Kernel2,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));

  int k;
  int indX, indY;
  for(k = 1; k < Nfr; k++) {
    //apply motion model
    //draws sample from motion model (random walk). The only prior information
    //is that the object moves 2x as fast as in the y direction
//#pragma omp parallel for shared(arrayX, arrayY, Nparticles, seed) private(x)
#pragma omp parallel for shared(arrayX, arrayY, seed) private(x)
    for(x = 0; x < Nparticles; x++) {
      arrayX[x] += 1 + 5*randn(seed, x);
      arrayY[x] += -2 + 2*randn(seed, x);
    }

    //particle filter likelihood
#pragma omp parallel for shared(likelihood, I, arrayX, arrayY, objxy, ind) private(x, y, indX, indY)
    for(x = 0; x < Nparticles; x++) {
      //compute the likelihood: remember our assumption is that you know
      // foreground and the background image intensity distribution.
      // Notice that we consider here a likelihood ratio, instead of
      // p(z|x). It is possible in this case. why? a hometask for you.		
      //calc ind
      for(y = 0; y < countOnes; y++) {
        indX = roundDouble(arrayX[x]) + objxy[y*2 + 1];
        indY = roundDouble(arrayY[x]) + objxy[y*2];
        ind[x*countOnes + y] = fabs(indX*IszY*Nfr + indY*Nfr + k);
        if(ind[x*countOnes + y] >= max_size)
          ind[x*countOnes + y] = 0;
      }
      likelihood[x] = 0;
      for(y = 0; y < countOnes; y++)
        likelihood[x] += (pow((I[ind[x*countOnes + y]] - 100),2) - pow((I[ind[x*countOnes + y]]-228),2))/50.0;
      likelihood[x] = likelihood[x]/((double) countOnes);
    }
    long long likelihood_time = get_time();
    // update & normalize weights
    // using equation (63) of Arulampalam Tutorial
//#pragma omp parallel for shared(Nparticles, weights, likelihood) private(x)
#pragma omp parallel for shared(weights, likelihood) private(x)
    for(x = 0; x < Nparticles; x++) {
      weights[x] = weights[x] * exp(likelihood[x]);
    }

    double sumWeights = 0;
    gettimeofday(&t1, NULL);
#pragma omp target teams distribute parallel for private(x) reduction(+:sumWeights) map(weights[0:Nparticles]) map(to: sumWeights)
    for(x = 0; x < Nparticles; x++) {
      sumWeights += weights[x];
    }
    gettimeofday(&t2, NULL);
    printf("Kernel3,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));

    gettimeofday(&t1, NULL);
#pragma omp target teams distribute parallel for map(weights[0:Nparticles])
    for(int x = 0; x < Nparticles; x++) {
      weights[x] = weights[x]/sumWeights;
    }
    gettimeofday(&t2, NULL);
    printf("Kernel4,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));

    xe = 0;
    ye = 0;
    // estimate the object location by expected values
    gettimeofday(&t1, NULL);
#pragma omp target teams distribute parallel for private(x) reduction(+:xe, ye) map(weights[0:Nparticles], arrayX[0:Nparticles], arrayY[0:Nparticles])
    for(x = 0; x < Nparticles; x++) {
      xe += arrayX[x] * weights[x];
      ye += arrayY[x] * weights[x];
    }
    gettimeofday(&t2, NULL);
    printf("Kernel5,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));

    CDF[0] = weights[0];
    for(x = 1; x < Nparticles; x++) {
      CDF[x] = weights[x] + CDF[x-1];
    }
    long long cum_sum = get_time();

    double u1 = (1/((double)(Nparticles)))*randu(seed, 0);
    gettimeofday(&t1, NULL);
//#pragma omp target teams distribute parallel for shared(u, u1, Nparticles) private(x) map(u[0:Nparticles])
#pragma omp target teams distribute parallel for map(u[0:Nparticles])
    for(int x = 0; x < Nparticles; x++) {
      u[x] = u1 + x/((double)(Nparticles));
    }
    gettimeofday(&t2, NULL);
    printf("Kernel6,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));

//#pragma omp parallel for shared(CDF, Nparticles, xj, yj, u, arrayX, arrayY)
#pragma omp parallel for shared(CDF, xj, yj, u, arrayX, arrayY)
    for(int j = 0; j < Nparticles; j++) {
      int i = findIndex(CDF, Nparticles, u[j]);
      if(i == -1)
        i = Nparticles-1;
      xj[j] = arrayX[i];
      yj[j] = arrayY[i];

    }

    gettimeofday(&t1, NULL);
//#pragma omp target teams distribute parallel for shared(weights, Nparticles) private(x) map(arrayX[0:Nparticles],arrayY[0:Nparticles], xj[0:Nparticles],yj[0:Nparticles],weights[0:Nparticles])
#pragma omp target teams distribute parallel for map(arrayX[0:Nparticles],arrayY[0:Nparticles], xj[0:Nparticles],yj[0:Nparticles],weights[0:Nparticles])
    for(int x = 0; x < Nparticles; x++) {
      arrayX[x] = xj[x];
      arrayY[x] = yj[x];
      weights[x] = 1/((double)(Nparticles));
    }
    gettimeofday(&t2, NULL);
    printf("Kernel7,%ld\n", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
  }
}

int main(int argc, char * argv[]) {

  char* usage = (char*)"openmp.out -x <dimX> -y <dimY> -z <Nfr> -np <Nparticles>";
  //check number of arguments
  if(argc != 9) {
    printf("%s\n", usage);
    return 0;
  }
  //check args deliminators
  if( strcmp( argv[1], "-x" ) ||  strcmp( argv[3], "-y" ) || strcmp( argv[5], "-z" ) || strcmp( argv[7], "-np" ) ) {
    printf( "%s\n",usage );
    return 0;
  }

  int IszX, IszY, Nfr;//, Nparticles;

  //converting a string to a integer
  if( sscanf( argv[2], "%d", &IszX ) == EOF ) {
    printf("ERROR: dimX input is incorrect");
    return 0;
  }

  if( IszX <= 0 ) {
    printf("dimX must be > 0\n");
    return 0;
  }

  //converting a string to a integer
  if( sscanf( argv[4], "%d", &IszY ) == EOF ) {
    printf("ERROR: dimY input is incorrect");
    return 0;
  }

  if( IszY <= 0 ) {
    printf("dimY must be > 0\n");
    return 0;
  }

  //converting a string to a integer
  if( sscanf( argv[6], "%d", &Nfr ) == EOF ) {
    printf("ERROR: Number of frames input is incorrect");
    return 0;
  }

  if( Nfr <= 0 ) {
    printf("number of frames must be > 0\n");
    return 0;
  }

  //converting a string to a integer
  //if( sscanf( argv[8], "%d", &Nparticles ) == EOF ) {
  //  printf("ERROR: Number of particles input is incorrect");
  //  return 0;
  //}

  if( Nparticles <= 0 ) {
    printf("Number of particles must be > 0\n");
    return 0;
  }

  //establish seed
  int * seed = (int *)malloc(sizeof(int)*Nparticles);
  int i;
  for(i = 0; i < Nparticles; i++)
    seed[i] = time(0)*i;

  //malloc matrix
  int * I = (int *)malloc(sizeof(int)*IszX*IszY*Nfr);
  //call video sequence
  videoSequence(I, IszX, IszY, Nfr, seed);

#pragma omp target
  {} 
  //call particle filter
  //particleFilter(I, IszX, IszY, Nfr, seed, Nparticles);
  particleFilter(I, IszX, IszY, Nfr, seed);

  free(seed);
  free(I);
  return 0;
}

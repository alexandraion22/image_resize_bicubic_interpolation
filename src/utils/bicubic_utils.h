#include "lodepng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CHANNELS 3
#define AREA_SIZE 4
#define OUTPUT_NAME 200
using namespace std;

int min(int a, int b);

unsigned char *readPNG(const char *file, unsigned int &width, unsigned int &height);
int savePNG(const char *file, unsigned char *img, int width, int height);

double bicubicpol(double x, double y, double p[4][4]);

unsigned char **createMatrix(int h, int w);
unsigned char *convert2Dto1D(unsigned char **imgR, unsigned char **imgG, unsigned char **imgB, unsigned int h,unsigned int w);
unsigned char **getChannel2D(unsigned char *img1D, unsigned int h, unsigned int w, unsigned int index);
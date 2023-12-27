#include "bicubic_utils.h"

// Alocare matrice de dimensiunea h*w
unsigned char **createMatrix(int h, int w)
{
	// Alocare a unui spatiu continuu pentru matrice
    unsigned char **mat = (unsigned char **)malloc(h * sizeof(unsigned char *));
	unsigned char *data = (unsigned char *)malloc(h * w * sizeof(unsigned char *));
    for (int i = 0; i < h; i++)
        mat[i] = &data[w*i];
    return mat;
}

// Minimul a doua numere intregi
int min(int a, int b) { return a < b ? a : b; }

// Functie de calcul a pixelului folosind cei 4x4 pixeli din jur cu bicubic interpolation

double bicubicpol(double x, double y, double p[4][4])
{
    double a00, a01, a02, a03, a10, a11, a12, a13, a20, a21, a22, a23, a30, a31, a32, a33;
    double x2 = x * x;
    double x3 = x2 * x;
    double y2 = y * y;
    double y3 = y2 * y;

    a00 = p[1][1];
    a01 = -.5 * p[1][0] + .5 * p[1][2];
    a02 = p[1][0] - 2.5 * p[1][1] + 2 * p[1][2] - .5 * p[1][3];
    a03 = -.5 * p[1][0] + 1.5 * p[1][1] - 1.5 * p[1][2] + .5 * p[1][3];
    a10 = -.5 * p[0][1] + .5 * p[2][1];
    a11 = .25 * p[0][0] - .25 * p[0][2] - .25 * p[2][0] + .25 * p[2][2];
    a12 = -.5 * p[0][0] + 1.25 * p[0][1] - p[0][2] + .25 * p[0][3] +
           .5 * p[2][0] - 1.25 * p[2][1] + p[2][2] - .25 * p[2][3];
    a13 = .25 * p[0][0] - .75 * p[0][1] + .75 * p[0][2] - .25 * p[0][3] -
          .25 * p[2][0] + .75 * p[2][1] - .75 * p[2][2] + .25 * p[2][3];
    a20 = p[0][1] - 2.5 * p[1][1] + 2 * p[2][1] - .5 * p[3][1];
    a21 = -.5 * p[0][0] + .5 * p[0][2] + 1.25 * p[1][0] - 1.25 * p[1][2] -
          p[2][0] + p[2][2] + .25 * p[3][0] - .25 * p[3][2];
    a22 = p[0][0] - 2.5 * p[0][1] + 2 * p[0][2] - .5 * p[0][3] - 2.5 * p[1][0] +
            6.25 * p[1][1] - 5 * p[1][2] + 1.25 * p[1][3] + 2 * p[2][0] -
            5 * p[2][1] + 4 * p[2][2] - p[2][3] - .5 * p[3][0] + 1.25 * p[3][1] -
            p[3][2] + .25 * p[3][3];
    a23 = -.5 * p[0][0] + 1.5 * p[0][1] - 1.5 * p[0][2] + .5 * p[0][3] +
            1.25 * p[1][0] - 3.75 * p[1][1] + 3.75 * p[1][2] - 1.25 * p[1][3] -
            p[2][0] + 3 * p[2][1] - 3 * p[2][2] + p[2][3] + .25 * p[3][0] -
            .75 * p[3][1] + .75 * p[3][2] - .25 * p[3][3];
    a30 = -.5 * p[0][1] + 1.5 * p[1][1] - 1.5 * p[2][1] + .5 * p[3][1];
    a31 = .25 * p[0][0] - .25 * p[0][2] - .75 * p[1][0] + .75 * p[1][2] +
          .75 * p[2][0] - .75 * p[2][2] - .25 * p[3][0] + .25 * p[3][2];
    a32 = -.5 * p[0][0] + 1.25 * p[0][1] - p[0][2] + .25 * p[0][3] +
          1.5 * p[1][0] - 3.75 * p[1][1] + 3 * p[1][2] - .75 * p[1][3] -
          1.5 * p[2][0] + 3.75 * p[2][1] - 3 * p[2][2] + .75 * p[2][3] +
          .5 * p[3][0] - 1.25 * p[3][1] + p[3][2] - .25 * p[3][3];
    a33 = .25 * p[0][0] - .75 * p[0][1] + .75 * p[0][2] - .25 * p[0][3] -
          .75 * p[1][0] + 2.25 * p[1][1] - 2.25 * p[1][2] + .75 * p[1][3] +
          .75 * p[2][0] - 2.25 * p[2][1] + 2.25 * p[2][2] - .75 * p[2][3] -
          .25 * p[3][0] + .75 * p[3][1] - .75 * p[3][2] + .25 * p[3][3];

    return (a00 + a01 * y + a02 * y2 + a03 * y3) +
           (a10 + a11 * y + a12 * y2 + a13 * y3) * x +
           (a20 + a21 * y + a22 * y2 + a23 * y3) * x2 +
           (a30 + a31 * y + a32 * y2 + a33 * y3) * x3;
}

// Salvare vector in imagine PNG
int savePNG(const char *file, unsigned char *img, int width, int height)
{
	unsigned char *png;
	size_t pngsize;
	int error = lodepng_encode24(&png, &pngsize, img, width, height);

	// Tratare eroare
	if (!error)
		lodepng_save_file(png, pngsize, file);
	else
	    printf("\tEroare %u la deschiderea imaginii %s: %s\n", error, file, lodepng_error_text(error));

	free(png);
	return error;
}

// Combina informatiile din trei matrici 2D pentru a forma o matrice 1D pentru o imagine color
unsigned char *convert2Dto1D(unsigned char **imgR, unsigned char **imgG,
                             unsigned char **imgB, unsigned int h,
                             unsigned int w)
{
	unsigned char *img1D = (unsigned char *)malloc(sizeof(unsigned char) * h * w * 3);

	int k,l;

	for (int i = 0; i < h; i++) {
		k = 3 * w * i;
		for (int j = 0; j < w; j++) {
            l = 3 * j + k;
            img1D[l] = imgR[i][j];
            img1D[l + 1] = imgG[i][j];
            img1D[l + 2] = imgB[i][j];
		}
	}
	return (img1D);
}

// Citire date din fisier PNG
unsigned char *readPNG(const char *file, unsigned int &width, unsigned int &height) 
{
	unsigned int error;
	unsigned char *image, *png = 0;
	size_t pngsize;
	LodePNGState state;

	lodepng_state_init(&state);
	error = lodepng_load_file(&png, &pngsize, file);

	// Tratare eroare
	if (!error)
		error = lodepng_decode(&image, &width, &height, &state, png, pngsize);
	if(error)
        printf("Eroare %u: %s\n", error, lodepng_error_text(error));

	// Eliberare spatiu
	free(png);
	lodepng_state_cleanup(&state);
	return image;
}

/*  Calcul canal de culoare din imaginea initiala
 *  index = 0  - canal rosu
 *  index = 1  - canal verde
 *  index = 2  - canal albastru
 */
unsigned char **getChannel2D(unsigned char *img1D, unsigned int h, unsigned int w, unsigned int index)
{
    unsigned int k;
    unsigned char **mat = createMatrix(h, w);

    for (int i = 0; i < h; i++) {
        k = i * w * (CHANNELS + 1);
        for (int j = 0; j < w; j++) 
        mat[i][j] = img1D[j * (CHANNELS+1) + index + k];
    }

    return mat;
}

#include "../utils/bicubic_utils.h"
#include <omp.h>

unsigned char **imageInterpolate(float factor, int h, int w, unsigned char **imgC)
{
	double arr[AREA_SIZE][AREA_SIZE];
	int i, j, k, l;
	unsigned char **result = createMatrix((int)(factor * h), (int)(factor * w));

	// Creare matrice locala
	for (i = 0; i < AREA_SIZE; i++)
		for (j = 0; j < AREA_SIZE; j++)
		arr[i][j] = 0;

	// Calcul pixeli rezulatati in functie de cei 16 pixeli inconjuratori - folosind openmp
	#pragma omp parallel for shared(imgC) private(i,j,k,l,arr) collapse(2)
	for (i = 0; i < (int)(factor * h); i++)
		for (j = 0; j < (int)(factor * w); j++) {
			for (l = 0; l < AREA_SIZE; l++)
				for (k = 0; k < AREA_SIZE; k++)
					arr[l][k] = (double)imgC[(int)(i / factor)][(int)(j / factor)];
			result[i][j] = (unsigned char)bicubicpol(min(((double)i - ((int)(i / factor) * factor)) / factor, h),
												     min(((double)j - ((int)(j / factor) * factor)) / factor, w), arr);
		}

	return result;
}

int main(int argc, char *argv[])
{

	if (argc < 3) {
		printf("Numar gresit de argumente. Programul se utilizeaza astfel: ./resize <nume_imagine> <factor>\n");
		return 0;
	}

	string inPath = "../../input/" + string(argv[1]);
	unsigned char *image1D, **resultR, **resultG, **resultB;
	unsigned int w, h;
	char outFile[OUTPUT_NAME];
	float factor = atof(argv[2]);

	// Citirea datelor imaginii
	image1D = readPNG(inPath.data(), w, h);

	// Obtinerea fiecarui canal de culoare
	unsigned char **imgR = getChannel2D(image1D, h, w, 0);
	unsigned char **imgG = getChannel2D(image1D, h, w, 1);
	unsigned char **imgB = getChannel2D(image1D, h, w, 2);

	// Resize al fiecarui canal de culoare
	resultR = imageInterpolate(factor, h, w, imgR);
	resultG = imageInterpolate(factor, h, w, imgG);
	resultB = imageInterpolate(factor, h, w, imgB);

	// Combina toate cele trei canale de culaore intr-un singur vector pentru a forma o imagine color
	unsigned char *resultImage1D = convert2Dto1D(resultR, resultG, resultB, (int)(factor * h), int(factor * w));

	// Salvarea imaginii
	sprintf(outFile, "%s%.2fx%s", "../../output/openmp/", factor, argv[1]);
	savePNG(outFile, resultImage1D, floor(factor * w), floor(factor * h));

	return 0;
}

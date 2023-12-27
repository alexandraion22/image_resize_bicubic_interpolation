#include "../utils/bicubic_utils.h"
#include <pthread.h>

typedef struct thread_arg {
  unsigned int id, h, w, threadNum;
  unsigned char **oldR, **oldG, **oldB;
  unsigned char **resultR, **resultG, **resultB;
  float factor;
} thread_arg;

// Interpolarea elementelor unei matrici
void interpolate_image(unsigned char **oldImg, unsigned char **resultImg, int start,
                       int end, double arr[AREA_SIZE][AREA_SIZE], int h, int w, float factor)
{
	for (int i = start; i < end; i++)
		for (int j = 0; j < (int)(factor * w); j++) {
			for (int l = 0; l < AREA_SIZE; l++)
				for (int k = 0; k < AREA_SIZE; k++)
					arr[l][k] = (double)oldImg[(int)(i / factor)][(int)(j / factor)];
			resultImg[i][j] = (unsigned char)bicubicpol(min(((double)i - ((int)(i / factor) * factor)) / factor, h),
														min(((double)j - ((int)(j / factor) * factor)) / factor, w), arr);
		}
}

// Functie executata de fiecare thread - interpolarea celor 3 canale de culoare (fiecare thread cate o bucata din fiecare canal)
void *thread_func(void *argument)
{
	thread_arg arg = *(thread_arg *)argument;

	int i, j;
	double arr[AREA_SIZE][AREA_SIZE];

	// Creare matrice locala
	for (i = 0; i < AREA_SIZE; i++)
		for (j = 0; j < AREA_SIZE; j++)
		arr[i][j] = 0;

	// Impartire task-uri in mod egal (pe baza id-ului) -> fiecare thread lucreaza la H/N linii din fiecare canal de culoare
	int start = arg.id * ((double)(arg.factor * arg.h) / arg.threadNum);
	int end = min((arg.id + 1) * ((double)(arg.factor * arg.h) / arg.threadNum), arg.h * arg.factor);

	// Resize image pe fiecare canal de culoare
	interpolate_image(arg.oldR, arg.resultR, start, end, arr, arg.h, arg.w, arg.factor);
	interpolate_image(arg.oldG, arg.resultG, start, end, arr, arg.h, arg.w, arg.factor);
	interpolate_image(arg.oldB, arg.resultB, start, end, arr, arg.h, arg.w, arg.factor);
}


int main(int argc, char *argv[])
{
	if (argc < 4) {
		printf("Numar gresit de argumente. Programul se utilizeaza astfel: ./resize <nume_imagine> <factor> <numar_threaduri>\n");
		return 0;
	}

	string inPath = "../../input/" + string(argv[1]);
	unsigned char *image1D, **resultR, **resultG, **resultB;
	unsigned int w, h;
	char outFile[OUTPUT_NAME];
	float factor = atof(argv[2]);
	int threadNum = atoi(argv[3]);

	// Vector de thread-uri si vector de argumente
  	pthread_t threads[threadNum];
  	thread_arg arguments[threadNum];

	// Citirea datelor imaginii
	image1D = readPNG(inPath.data(), w, h);

	// Obtinerea fiecarui canal de culoare
	unsigned char **imgR = getChannel2D(image1D, h, w, 0);
	unsigned char **imgG = getChannel2D(image1D, h, w, 1);
	unsigned char **imgB = getChannel2D(image1D, h, w, 2);

	// Alocare spatiu pentru fiecare canal de culoare
	resultR = createMatrix((int)(factor * h), (int)(factor * w));
  	resultG = createMatrix((int)(factor * h), (int)(factor * w));
  	resultB = createMatrix((int)(factor * h), (int)(factor * w));

	// Creare thread-uri
	for (int i = 0; i < threadNum; i++) {
		arguments[i].id = i;

		arguments[i].h = h;
		arguments[i].w = w;
		arguments[i].factor = factor;
		arguments[i].threadNum = threadNum;

		arguments[i].oldR = imgR;
		arguments[i].oldG = imgG;
		arguments[i].oldB = imgB;

		arguments[i].resultR = resultR;
		arguments[i].resultG = resultG;
		arguments[i].resultB = resultB;

		pthread_create(&threads[i], NULL, thread_func, &arguments[i]);
  	}

	// Join thread-uri
	for (int i = 0; i < threadNum; i++)
		pthread_join(threads[i], NULL);

	// Combina toate cele trei canale de culoare intr-un singur vector pentru a forma o imagine color
	unsigned char *resultImage1D = convert2Dto1D(resultR, resultG, resultB, (int)(factor * h), int(factor * w));

	// Salvarea imaginii
	sprintf(outFile, "%s%.2fx%s", "../../output/pthread/", factor, argv[1]);
	savePNG(outFile, resultImage1D, floor(factor * w), floor(factor * h));

	return 0;
}

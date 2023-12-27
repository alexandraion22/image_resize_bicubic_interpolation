#include "../utils/bicubic_utils.h"
#include <mpi.h>
#include <pthread.h>
#define MASTER 0

typedef struct thread_arg {
  int id;
  unsigned char **oldR, **oldG, **oldB;
  unsigned char **resultR, **resultG, **resultB;
  int h, w, thread_num;
  float factor;
} thread_arg;

// Interpolarea elementelor unei matrici
void interpolate_image(unsigned char **oldImg, unsigned char **resultImg, int start,
                       int end, double arr[AREA_SIZE][AREA_SIZE], int h, int w, float factor) {

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
void *thread_func(void *arg) {

  thread_arg argument = *(thread_arg *)arg;

  int i, j;
  double arr[AREA_SIZE][AREA_SIZE];

  // Creare matrice locala
  for (i = 0; i < AREA_SIZE; i++)
    for (j = 0; j < AREA_SIZE; j++)
      arr[i][j] = 0;

  // Impartire task-uri in mod egal (pe baza id-ului) -> fiecare thread lucreaza la N/H linii din fiecare canal de culoare
  int start = argument.id * ((double)(argument.factor * argument.h) / argument.thread_num);
  int end = min((argument.id + 1) * ((double)(argument.factor * argument.h) / argument.thread_num), argument.h * argument.factor);

  // Resize image pe fiecare canal de culoare
  interpolate_image(argument.oldR, argument.resultR, start, end, arr, argument.h, argument.w, argument.factor);
  interpolate_image(argument.oldG, argument.resultG, start, end, arr, argument.h, argument.w, argument.factor);
  interpolate_image(argument.oldB, argument.resultB, start, end, arr, argument.h, argument.w, argument.factor);
}

int main(int argc, char *argv[])
{

	int rank, size;

	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	string inPath = "../../input/" + string(argv[1]);
	unsigned char *image1D, **resultR, **resultG, **resultB, **imgR, **imgG, **imgB;
	unsigned int w, h, chunkSize, processHeight, thread_num;
	char outFile[OUTPUT_NAME];
	float factor = atof(argv[2]);

	if(rank == MASTER){
		if (argc < 4) {
			printf("Numar gresit de argumente. Programul se utilizeaza astfel: ./resize <nume_imagine> <factor> <numar_threaduri>\n");
			return 0;
		}

		factor = atof(argv[2]);
		thread_num = atoi(argv[3]);

		// Citirea datelor imaginii
		image1D = readPNG(inPath.data(), w, h);
	}

	// Broadcast al inaltimii si latimii imaginii cat si al factorului de redimensionare
	MPI_Bcast(&w, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&h, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&factor, 1, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&thread_num, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);

	// Calcul inaltime bucata de imagine ce revine fiecarui proces
	processHeight = h / size;
	
	// In cazul in care inaltimea nu se imparte exact, ultimul proces va lua restul imaginii pana la final 
	if(rank == size-1)
		processHeight += h % size;

	chunkSize = processHeight * w;

	// Trimitere/Primire parti din imagine inainte de procesare
	if(rank == MASTER)
		{
			// Obtinerea fiecarui canal de culoare
			imgR = getChannel2D(image1D, h, w, 0);
			imgG = getChannel2D(image1D, h, w, 1);
			imgB = getChannel2D(image1D, h, w, 2);
			
			for (int i = 1; i < size; i++)
            	{
					// Procesul cu ultimul rank va procesa o parte mai mare din imagine daca nu se imparte exact
					if(i == size -1)
						chunkSize = (processHeight + h % size) * w;
					MPI_Send(imgR[i * processHeight], chunkSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
					MPI_Send(imgG[i * processHeight], chunkSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
					MPI_Send(imgB[i * processHeight], chunkSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
				}
		}
	else
		{
			imgR = createMatrix(processHeight, w);
			imgG = createMatrix(processHeight, w);
			imgB = createMatrix(processHeight, w);

 			MPI_Recv(imgR[0], chunkSize, MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD, NULL);
			MPI_Recv(imgG[0], chunkSize, MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD, NULL);
			MPI_Recv(imgB[0], chunkSize, MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD, NULL);
		}

	// Alocare spatiu pentru fiecare canal de culoare
	resultR = createMatrix((int)(factor * processHeight), (int)(factor * w));
  	resultG = createMatrix((int)(factor * processHeight), (int)(factor * w));
  	resultB = createMatrix((int)(factor * processHeight), (int)(factor * w));

	pthread_t threads[thread_num];
  	thread_arg arguments[thread_num];
  	void *status;

	// Creare thread-uri
	for (int i = 0; i < thread_num; i++) {
		arguments[i].id = i;

		arguments[i].h = processHeight;
		arguments[i].w = w;
		arguments[i].factor = factor;
		arguments[i].thread_num = thread_num;

		arguments[i].oldR = imgR;
		arguments[i].oldG = imgG;
		arguments[i].oldB = imgB;

		arguments[i].resultR = resultR;
		arguments[i].resultG = resultG;
		arguments[i].resultB = resultB;

		pthread_create(&threads[i], NULL, thread_func, &arguments[i]);
  	}

	// Join thread-uri
	for (int i = 0; i < thread_num; i++)
		pthread_join(threads[i], &status);
	
	chunkSize = processHeight * w * factor * factor;

	// Trimitere/Primire parti din imagine dupa procesare
    if (rank == MASTER) {

		// Alocare spatiu pentru componenete finale
        unsigned char **resultFinalR = createMatrix(factor*h, factor*w);
        unsigned char **resultFinalG = createMatrix(factor*h, factor*w);
        unsigned char **resultFinalB = createMatrix(factor*h, factor*w);
        
		int newHeight = processHeight *factor;
        
        memcpy(resultFinalR[0], resultR[0], newHeight * factor * w * sizeof(unsigned char));
        memcpy(resultFinalG[0], resultG[0], newHeight * factor * w * sizeof(unsigned char));
        memcpy(resultFinalB[0], resultB[0], newHeight * factor * w * sizeof(unsigned char));
        
		// Primire bucati din imagine
        for (int i = 1; i < size; i++) {
			// Procesul cu ultimul rank va returna o parte mai mare din imagine daca nu se imparte exact
			if(i == size -1)
				chunkSize = (processHeight + h % size) * w * factor * factor;
            MPI_Recv(resultFinalR[i * newHeight], chunkSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, NULL);
            MPI_Recv(resultFinalG[i * newHeight], chunkSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, NULL);
            MPI_Recv(resultFinalB[i * newHeight], chunkSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, NULL);
        }

        // Combina toate cele trei canale de culaore intr-o singura matrice 1D pentru a forma o imagine color
        unsigned char *imgRGB1 = convert2Dto1D(resultFinalR, resultFinalG, resultFinalB,(int)(factor*h),int(factor*w));

        // Salvarea imaginii
        char outFile[50];
        sprintf(outFile, "%s%.2fx%s","../../output/mpi_pthread/",factor,argv[1]);
        savePNG(outFile, imgRGB1, floor(factor*w), floor(factor * h));

    }
	else {
		MPI_Send(resultR[0], chunkSize, MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD);
        MPI_Send(resultG[0], chunkSize, MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD);
        MPI_Send(resultB[0], chunkSize, MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}

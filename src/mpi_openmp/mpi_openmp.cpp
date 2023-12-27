#include "../utils/bicubic_utils.h"
#include <mpi.h>
#include <omp.h>
#define MASTER 0

unsigned char **imageInterpolate(float factor, int h, int w, unsigned char **imgC)
{

	double arr[AREA_SIZE][AREA_SIZE];
	int i, j, k ,l;
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

	int rank, size;

	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	string inPath = "../../input/" + string(argv[1]);
	unsigned char *image1D, **resultR, **resultG, **resultB, **imgR, **imgG, **imgB;
	unsigned int w, h, chunkSize, processHeight;
	char outFile[OUTPUT_NAME];
	float factor = atof(argv[2]);

	if(rank == MASTER) {
		if (argc < 3) {
			printf("Numar gresit de argumente. Programul se utilizeaza astfel: ./resize <nume_imagine> <factor>\n");
			return 0;
		}
		factor = atof(argv[2]);

		// Citirea datelor imaginii
		image1D = readPNG(inPath.data(), w, h);
	}

	// Broadcast al inaltimii si latimii imaginii cat si al factorului de redimensionare
	MPI_Bcast(&w, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&h, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&factor, 1, MPI_FLOAT, MASTER, MPI_COMM_WORLD);

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
					// Procesul cu ultimul rank va calcula o parte mai mare din imagine daca nu se imparte exact
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

	// Resize al fiecarui canal de culoare
	resultR = imageInterpolate(factor, processHeight, w, imgR);
	resultG = imageInterpolate(factor, processHeight, w, imgG);
	resultB = imageInterpolate(factor, processHeight, w, imgB);
	
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

        // Combina toate cele trei canale de culaore intr-un singur vector pentru a forma o imagine color
        unsigned char *imgRGB1 = convert2Dto1D(resultFinalR, resultFinalG, resultFinalB,(int)(factor*h),int(factor*w));

        // Salvarea imaginii
        char outFile[50];
        sprintf(outFile, "%s%.2fx%s","../../output/mpi_openmp/",factor,argv[1]);
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

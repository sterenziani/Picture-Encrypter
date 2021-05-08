#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "image.h"

enum mode{DISTRIBUTE, RECOVER};
typedef struct args{
	enum mode selected_mode;
	image_t image;
	int k;
	char* dir_name;
	DIR* dir;
} args_t;

int parse_args(int argc, char* argv[], args_t* args)
{
	args->dir = NULL;
	args->image.file = NULL;
	if(argc != 5)
	{
		fprintf(stderr, "ERROR. Program expected 4 arguments, but received only %d.\n", argc-1);
		return EXIT_FAILURE;
	}

	// ARG 1 - d or r
	if(strcmp(argv[1], "d") == 0)
	{
		args->selected_mode = DISTRIBUTE;
	}
	else if(strcmp(argv[1], "r") == 0)
	{
		args->selected_mode = RECOVER;
	}
	else
	{
		fprintf(stderr, "ERROR. First argument should be either 'd' or 'r' but received %s.\n", argv[1]);
		return EXIT_FAILURE;
	}

	// ARG 2 - Original image file (Input if d, Output if r)
	switch (args->selected_mode)
	{
		case DISTRIBUTE:
			printf("Loading file %s\n", argv[2]);
			args->image = load_image(argv[2]);
			if(args->image.file == NULL)
			{
				fprintf(stderr, "ERROR. Image does not exist or is not compatible with this program.\n");
				return EXIT_FAILURE;
			}
			break;
		case RECOVER:
			printf("Work in progress. Should check that file doesn't exist so we don't overwrite it\n");
			break;
		default:
			printf("You shouldn't be here. How did you get here?!\n");
			return EXIT_FAILURE;
	}

	// ARG 3 - Amount of shadows
	args->k = atoi(argv[3]);
	if(args->k < 4 && args->k > 6)
	{
		fprintf(stderr, "ERROR. k should be an integer between 4 and 6.\n");
		return EXIT_FAILURE;
	}
	if(args->selected_mode == DISTRIBUTE && args->image.width*args->image.height % args->k != 0)
	{
		fprintf(stderr, "ERROR. Image should have an amount of pixels divisible by k. %d / %d is not a whole number.\n", args->image.width*args->image.height, args->k);
		return EXIT_FAILURE;
	}

	// ARG 4 - Shadow realm. If r, check they're all the same size and there's at least k of them.
	args->dir_name = argv[4];
	args->dir = opendir(argv[4]);
	if(args->dir)
	{
		// check same size and k of them
		printf("IMPLEMENT ME! Check there's at least k images of the same size as the original image!\n");
		return EXIT_SUCCESS;
	}
	else if (ENOENT == errno)
	{
		fprintf(stderr, "ERROR. Directory %s not found.\n", argv[4]);
		return EXIT_FAILURE;
	}
	else
	{
	    fprintf(stderr, "ERROR. Directory %s could not be opened.\n", argv[4]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// LO QUE ENTENDÍ DEL ALGORITMO LEYENDO EL PAPER:
// Tomo el binario de mi imagen original y cada k bytes formo un nuevo bloque Bj
// Usando los k bytes de ese bloque Bj, armo un polinomio de grado k-1 llamado F(x)		(No me queda claro si hay un F(x) por cada Bj o hay uno solo)
// Para cada imagen camuflaje, la separo en bloques de 2x2 XWVU 				(tantos bloques como la cantidad de Bj que tenía la original, CREO??)
// Se calcula F(X) en cada bloque y se reparten los bits del resultado entre W, V y U, pisando los valores originales.
// Para descifrar, tomo los bits "nuevos" de cada bloque, y recupero F(X) de ese bloque.
// Si tomo el mismo bloque para todas las imágenes, obtengo n puntos F(X) que permiten interpolar el polinomio, y recuperar el dato original
int main(int argc, char* argv[])
{
	args_t args;
	if(parse_args(argc, argv, &args) != EXIT_SUCCESS)
	{
		if(args.image.file != NULL && args.selected_mode == DISTRIBUTE)
		{
			free(args.image.file);
		}
		if(args.dir != NULL)
		{
			closedir(args.dir);
		}
		return EXIT_FAILURE;
	}
	printf("Picture is %d x %d pixels, rounded up to %d x %d\n", args.image.real_width, args.image.height, args.image.width, args.image.height);
	print_picture(args.image);
	free(args.image.file);
	closedir(args.dir);
	return EXIT_SUCCESS;
}
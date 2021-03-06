#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "image.h"
#include "galois.h"

enum mode{DISTRIBUTE, RECOVER};
typedef struct args{
	enum mode selected_mode;
	image_t image;
	int k;
	int n;
	char* dir_name;
	DIR* dir;
	image_t* pictures;
	char* filename;
} args_t;

int parse_args(int argc, char* argv[], args_t* args)
{
	args->dir = NULL;
	args->image.file = NULL;
	args->pictures = NULL;
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
	FILE* file;
	args->filename = argv[2];

	switch (args->selected_mode)
	{
		case DISTRIBUTE:
			args->image = load_image(args->filename);
			if(args->image.file == NULL)
			{
				fprintf(stderr, "ERROR. Image does not exist or is not compatible with this program.\n");
				return EXIT_FAILURE;
			}
			break;
		case RECOVER:
		    if(file = fopen(args->filename, "r"))
		    {
				fprintf(stderr, "ERROR. File %s already exists. Please choose another filename to avoid overwriting it!\n", args->filename);
		        fclose(file);
				return EXIT_FAILURE;
		    }
			break;
		default:
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
		args->n = collect_images(args->dir, args->dir_name, args->k, &(args->pictures));
		if(args->k > args->n)
			return EXIT_FAILURE;
		if(args->selected_mode == DISTRIBUTE)
		{
			if(args->image.real_width != args->pictures[0].real_width)
			{
				fprintf(stderr, "ERROR. Secret picture has width %dpx while camouflage pics have width %dpx. Make sure all pictures have the same dimensions.\n", args->image.real_width, args->pictures[0].real_width);
				return EXIT_FAILURE;
			}
			if(args->image.height != args->pictures[0].height)
			{
				fprintf(stderr, "ERROR. Secret picture has height %dpx while camouflage pics have height %dpx. Make sure all pictures have the same dimensions.\n", args->image.height, args->pictures[0].height);
				return EXIT_FAILURE;
			}
		}
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

int main(int argc, char* argv[])
{
	args_t args;
	if(parse_args(argc, argv, &args) != EXIT_SUCCESS)
	{
		if(args.image.file != NULL && args.selected_mode == DISTRIBUTE)
			free(args.image.file);
		if(args.dir != NULL)
			closedir(args.dir);
		if(args.pictures != NULL)
			free_picture_album(args.pictures, args.n);
		return EXIT_FAILURE;
	}
	load_multiplication_table();
	int block_count;
	if(args.selected_mode == DISTRIBUTE)
	{
		block_count = (args.image.height*args.image.width)/args.k;
		uint8_t** B = get_secret_blocks(args.image, args.k);
		uint8_t*** xwvu_album = get_xwvu_blocks(args.pictures, args.k, args.n);
		transform_xwvu_blocks(xwvu_album, B, block_count, args.k, args.n);
		replace_xwvu_blocks(xwvu_album, args.pictures, args.k, args.n);
		for(int i=0; i < args.n; i++)
			save_file(args.pictures[i]);
		free_secret_blocks(B, args.image, args.k);
		free_xwvu_blocks(xwvu_album, args.pictures, args.k, args.n);
	}
	else
	{
		block_count = (args.pictures[0].height*args.pictures[0].width)/args.k;
		uint8_t** points_X = recover_points(args.pictures, args.k, args.n, 0);
		uint8_t** points_Y = recover_points(args.pictures, args.k, args.n, 1);
		//TODO: agregar random a lagrange para q agarre distintos puntos
		uint8_t** polynomials = lagrange_interpolation(args.k, block_count, points_X, points_Y);
		recover_image(args.pictures[0], args.filename, polynomials, args.k, block_count);
		free_points(points_X, block_count);
		free_points(points_Y, block_count);
		free_points(polynomials, block_count);
	}

	// CLEANUP
	free_picture_album(args.pictures, args.n);
	free(args.image.file);
	closedir(args.dir);
	free_multiplication_table();
	return EXIT_SUCCESS;
}

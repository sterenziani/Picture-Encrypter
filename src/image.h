#ifndef IMAGE_H
#define IMAGE_H
#include <stdint.h>
#define BYTES_PER_PIXEL 8
#define MAX_CAMOUFLAGE_BUFFER 25

typedef struct {
	char* filename;
	void* file;
	int width;
	int height;
	int real_width;
    uint8_t* content;
} image_t;

image_t load_image(char* filename);
void free_image(image_t image);
void print_picture(image_t image);
int collect_images(DIR* FD, char* dir_name, int k, image_t** pics);
void free_picture_album(image_t* pictures, int size);
void save_file(image_t image);
void save_file_as(image_t image, char* filename);
uint8_t** get_secret_blocks(image_t image, int k);
void free_secret_blocks(uint8_t** blocks, image_t image, int k);

uint8_t*** get_xwvu_blocks(image_t* images, int k, int n);
void transform_xwvu_blocks(uint8_t*** album, uint8_t** polynomials, int block_count, int k, int n);
void replace_xwvu_blocks(uint8_t*** album, image_t* pictures, int k, int n);
void free_xwvu_blocks(uint8_t*** album, image_t* images, int k, int n);
uint8_t** recover_points(image_t* images, int k, int n, int dim);
uint8_t ** lagrange_interpolation(int k, int block_count, uint8_t** Xs, uint8_t** Ys);
void free_points(uint8_t** points, int block_count);

#endif

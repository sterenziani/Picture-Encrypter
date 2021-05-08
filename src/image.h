#ifndef IMAGE_H
#define IMAGE_H
#include <stdint.h>
#define BYTES_PER_PIXEL 8

typedef struct {
	void* file;
	int width;
	int height;
	int real_width;
    uint8_t* content;
} image_t;

image_t load_image(char* filename);
void print_picture(image_t image);

#endif

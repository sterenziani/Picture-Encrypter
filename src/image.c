#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "image.h"
#include "galois.h"

static int file_size(FILE* in, size_t* size)
{
  if(fseek(in, 0, SEEK_END) == 0)
  {
    long len = ftell(in);
    if(len > 0)
    {
      if(fseek(in, 0, SEEK_SET) == 0)
      {
        *size = (size_t) len;
        return 1;
      }
    }
  }
  return 0;
}

static void* load_binary(const char* filename, size_t* size)
{
  FILE* in;
  void* data = NULL;
  size_t len;

  if((in = fopen(filename, "rb")) != NULL)
  {
    if(file_size(in, &len))
    {
      if((data = malloc(len)) != NULL)
      {
        if(fread(data, 1, len, in) == len)
          *size = len;
        else
        {
          free(data);
          data = NULL;
        }
      }
    }
    fclose(in);
  }
  return data;
}

int read_little_endian_int(uint8_t* bytes)
{
    return bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
}

image_t load_image(char* filename)
{
	image_t image;
	uint8_t* file;
	size_t size;

	if((file = load_binary(filename, &size)) != NULL)
	{
		if(file[28] != BYTES_PER_PIXEL)
        {
            fprintf(stderr, "ERROR. Expected %d bytes per pixel but file provided has %d per pixel.\n", BYTES_PER_PIXEL, (uint16_t)file[28]);
            free(file);
    		image.file = NULL;
            return image;
        }
        //printf("--- Loading file %s\n", filename);
        image.filename = filename;
		image.real_width = read_little_endian_int(file+18);
		image.height = read_little_endian_int(file+22);
		image.width = read_little_endian_int(file+34) / image.height;
		if(image.width == 0)
			image.width = image.real_width;
		image.content = file + read_little_endian_int(file+10);
		image.file = file;
	}
	else
	{
		free(file);
		image.file = NULL;
	}
	return image;
}

void free_image(image_t image)
{
    free(image.filename);
}

void print_picture(image_t image)
{
	for(int i=image.height-1; i >= 0; i--)
	{
		for(int j=0; j < image.width; j++)
		{
			printf("%x\t", image.content[i*(image.width) + j]);
		}
		printf("\n");
	}
}

char* get_image_path(char* directory, char* filename)
{
    char* dest = (char*) calloc(strlen(directory)+strlen(filename)+2, sizeof(char));
    strcat(dest, directory);
    strcat(dest, "/");
    strcat(dest, filename);
    return dest;
}

uint8_t is_file_bmp(char* filename)
{
    return (!strcmp(filename+strlen(filename)-4, ".bmp"));
}

void free_picture_album(image_t* pictures, int size)
{
    for(int i = 0; i < size; i++)
    {
		free(pictures[i].file);
        free_image(pictures[i]);
    }
	free(pictures);
}

// k is the min expected amount of pictures
int collect_images(DIR* FD, char* dir_name, int k, image_t** pics)
{
    struct dirent* in_file;
    image_t* pictures = calloc(MAX_CAMOUFLAGE_BUFFER, sizeof(image_t));
    int image_count = 0;
    while ((in_file = readdir(FD)))
    {
        if (!strcmp(in_file->d_name, ".") || !strcmp (in_file->d_name, ".."))
            continue;
        char* filename = get_image_path(dir_name, in_file->d_name);

        // Only load image if it's a bitmap with the right format
        if(is_file_bmp(filename))
        {
            pictures[image_count] = load_image(filename);
            if(pictures[image_count].file != NULL)
                image_count++;
        }
    }
    // We have found image_count bitmaps. Let's check they're all the same size.
    if(image_count < k)
    {
        fprintf(stderr, "ERROR. Expected to find at least %d images, but only found %d.\n", k, image_count);
        free_picture_album(pictures, image_count);
        return -1;
    }
    if(pictures[0].height % 2 != 0)
    {
        fprintf(stderr, "ERROR. Picture height must be an even number, but at least one camouflage picture is %dpx tall, which is an odd number.\n", pictures[0].height);
        free_picture_album(pictures, image_count);
        return -1;
    }
    for(int i=1; i < image_count; i++)
    {
        if(pictures[i].real_width != pictures[0].real_width)
        {
            fprintf(stderr, "ERROR. One of the pictures has width %dpx while another one has width %dpx. Make sure all pictures in the directory have the same dimensions.\n", pictures[0].real_width, pictures[i].real_width);
            free_picture_album(pictures, image_count);
            return -1;
        }
        if(pictures[i].height != pictures[0].height)
        {
            fprintf(stderr, "ERROR. One of the pictures has height %dpx while another one has height %dpx.  Make sure all pictures in the directory have the same dimensions.\n", pictures[0].height, pictures[i].height);
            free_picture_album(pictures, image_count);
            return -1;
        }
    }
    pictures = realloc(pictures, image_count*sizeof(image_t));
    *pics = pictures;
    return image_count;
}

void save_file(image_t image)
{
    //printf("--- Saving file %s\n", image.filename);
    FILE* file = fopen(image.filename, "w");
    fwrite(image.file, sizeof(uint8_t), read_little_endian_int(image.file+2), file);
    fclose(file);
}

void save_file_as(image_t image, char* filename)
{
    image.filename = filename;
    //printf("--- Saving as file %s\n", filename);
    FILE* file = fopen(filename, "w");
    fwrite(image.file, sizeof(uint8_t), read_little_endian_int(image.file+2), file);
    fclose(file);
}

uint8_t** get_secret_blocks(image_t image, int k)
{
    int block_count = (image.height*image.width)/k;
    uint8_t** blocks = calloc(block_count, sizeof(uint8_t*));
    for(int j=0; j < block_count; j++)
    {
        blocks[j] = calloc(k, sizeof(uint8_t));
        for(int i=0; i < k; i++)
        {
            blocks[j][i] = image.content[j*k + i];
        }
    }
    return blocks;
}

void free_secret_blocks(uint8_t** blocks, image_t image, int k)
{
    int block_count = (image.height*image.width)/k;
    for(int j=0; j < block_count; j++)
    {
        free(blocks[j]);
    }
    free(blocks);
}

uint8_t** get_image_xwvu_blocks(image_t image, int k)
{
    int block_count = (image.height*image.width)/k;
    uint8_t** blocks = calloc(block_count, sizeof(uint8_t*));
    for(int j=0; j < block_count; j++)
    {
        int x = (2*j % image.width);
        int y = 2 * (2*j / image.width); // Keep the 2s separate, since a 4j/width could return an odd number, and we don't want that
        int X_block = (image.height-1)*image.width + x - y*image.width;
        blocks[j] = calloc(4, sizeof(uint8_t));
        blocks[j][0] = image.content[X_block];
        blocks[j][1] = image.content[X_block + 1];
        blocks[j][2] = image.content[X_block - image.width];
        blocks[j][3] = image.content[X_block - image.width + 1];
    }
    return blocks;
}

uint8_t*** get_xwvu_blocks(image_t* images, int k, int n)
{
    uint8_t*** album = calloc(n, sizeof(uint8_t*));
    for(int i=0; i < n; i++)
    {
        album[i] = get_image_xwvu_blocks(images[i], k);
    }
    return album;
}

// Adjusts all X values in XWVU album so all Xs within a block set are unique
void adjust_xwvu_blocks(uint8_t*** album, int block_count, int n)
{
    for(int j=0; j < block_count; j++)
    {
        for(int i=1; i < n; i++)
        {
            for(int index=0; index < i; index++)
            {
                if(album[index][j][0] == album[i][j][0])
                {
                    album[i][j][0] = (album[i][j][0] + 1) % 256;
                    index = -1; // Reset cycle to make sure this new value is unique
                }
            }
        }
    }
}

// Adjusts XWVU blocks and applies the F(X) transformation to every block
void transform_xwvu_blocks(uint8_t*** album, uint8_t** polynomials, int block_count, int k, int n)
{
    adjust_xwvu_blocks(album, block_count, n);
    for(int i=0; i < n; i++)
    {
        for(int j=0; j < block_count; j++)
        {
            T(album[i][j], polynomials[j], k);
        }
    }
}

void replace_xwvu_blocks_image(image_t image, uint8_t** xwvu, int k)
{
    int block_count = (image.height*image.width)/k;
    for(int j=0; j < block_count; j++)
    {
        int x = (2*j % image.width);
        int y = 2 * (2*j / image.width);
        int X_block = (image.height-1)*image.width + x - y*image.width;
        image.content[X_block] = xwvu[j][0];
        image.content[X_block + 1] = xwvu[j][1];
        image.content[X_block - image.width] = xwvu[j][2];
        image.content[X_block - image.width + 1] = xwvu[j][3];
    }
}

void replace_xwvu_blocks(uint8_t*** album, image_t* pictures, int k, int n)
{
    for(int i=0; i < n; i++)
        replace_xwvu_blocks_image(pictures[i], album[i], k);
}

void free_image_xwvu_blocks(uint8_t** blocks, image_t image, int k)
{
    int block_count = (image.height*image.width)/k;
    for(int j=0; j < block_count; j++)
        free(blocks[j]);
    free(blocks);
}

void free_xwvu_blocks(uint8_t*** album, image_t* images, int k, int n)
{
    for(int j=0; j < n; j++)
        free_image_xwvu_blocks(album[j], images[j], k);
    free(album);
}

// Returns points[block][image]
// Use dim=0 if you want values for X
// Use dim=1 if you want values for Y
uint8_t** recover_points(image_t* images, int k, int n, int dim)
{
    dim = dim % 2;
    uint8_t*** album = get_xwvu_blocks(images, k, n);
    int block_count = (images[0].height*images[0].width)/k;
    uint8_t** points = calloc(block_count, sizeof(uint8_t*));
    for(int j=0; j < block_count; j++)
    {
        points[j] = calloc(n, sizeof(uint8_t));
        for(int i=0; i < n; i++)
        {
            int value;
            if(dim == 0)
                value = album[i][j][0];
            else
            {
                value = T_inverse(album[i][j]);
                if(value < 0)
                {
                    for(int a=0; a <=j; a++)
                        free(points[a]);
                    free(points);
                    return NULL;
                }
            }
            points[j][i] = (uint8_t) value;
        }
    }
    free_xwvu_blocks(album, images, k, n);
    return points;
}

uint8_t ** lagrange_interpolation(int k, int block_count, uint8_t** Xs, uint8_t** Ys) {
    uint8_t** polynomials = calloc(block_count, sizeof(uint8_t*));

    for(int block=0; block < block_count; block++) {
        
        polynomials[block] = calloc(k, sizeof(uint8_t));
        for(int m = 0; m<k; m++) {
            polynomials[block][m] = 0;
        }

        for(int i = 0; i<k; i++){
            uint8_t poli[k];
            uint8_t det = 1;
            int n = 0;

            for (int j = 0; j<k; j++){
                if(i != j){
                    det = galois_multiply(det,galois_sum(Xs[block][i], Xs[block][j]));
                    
                    if (n == 0) {
                        poli[0] = galois_multiply(Xs[block][j], Ys[block][i]);
                        poli[1] =  Ys[block][i];
                    }

                    else {
                        for(int m=n; m>=0; m--) {
                            poli[m+1] = poli[m];
                        }
                        poli[0] = galois_multiply(poli[0], Xs[block][j]);
                        for(int m=1; m<=n; m++) {
                            poli[m] = galois_sum(poli[m], galois_multiply(poli[m+1], Xs[block][j]));
                        }
                    }
                    n++;
                }

            }
            for(int m = 0; m<k; m++) {
                polynomials[block][m] = galois_sum(polynomials[block][m], galois_divide(poli[m],det));
            }

        }
    }
    return polynomials;
}

void recover_image(image_t img, char* filename, uint8_t**polynomials, int k, int block_count) {
    for(int i =0; i<block_count; i++){
        for(int j =0; j<k; j++){
            img.content[i*k+j] = polynomials[i][j];
        }
    }
    save_file_as(img, filename);
}

void free_points(uint8_t** points, int block_count)
{
    for(int j=0; j < block_count; j++)
        free(points[j]);
    free(points);
}

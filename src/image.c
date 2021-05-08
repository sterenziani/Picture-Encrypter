#include <stdlib.h>
#include <stdio.h>
#include "image.h"

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
		// Print whole bitmap, including headers
		//for(unsigned int i=0; i < size; i++)
		//	printf("%d\t", file[i]);
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

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "image.h"
#include "galois.h"
#define PRIMITIVE 0x163 // x8 + x6 + x5 + x1 + 1

uint8_t** matrix = NULL;
uint8_t* inverses = NULL;

uint8_t galois_sum(uint8_t a, uint8_t b) {
	return a ^ b;
}

// Russian Peasant Multiplication algorithm
// The idea is to double the first number and halve the second number repeatedly until the second number doesn’t become 1
// Whenever the second number becomes odd, we add the first number to the result
uint8_t full_galois_multiply(uint8_t a, uint8_t b)
{
	uint8_t p = 0;
	while (a != 0 && b != 0)
    {
        if (b%2 != 0)
            p = galois_sum(p, a);

        //If a >= 128, then it will overflow when shifted left, so reduce by XOR-ing with m(x)
        if(a >= 128)
            a = (a << 1) ^ PRIMITIVE; // XOR with the primitive polynomial
        else
            a = a*2;
        b = b/2;
	}
	return p;
}

uint8_t galois_multiply(uint8_t a, uint8_t b)
{
    if(matrix == NULL)
        return full_galois_multiply(a, b);
    return matrix[a][b];
}

uint8_t galois_inverse(uint8_t x)
{
    if(inverses == NULL)
        load_multiplication_table();
    return inverses[x];
}

// Inverso = n es inverso de m y viceversa si n*m = 1
void load_multiplication_table()
{
    // Wipe previous tables
    if(matrix != NULL)
    {
        for(int i=0; i < 256; i++)
            free(matrix[i]);
        free(matrix);
    }
    if(inverses != NULL)
        free(inverses);

    // Build the matrix
    matrix = calloc(256, sizeof(uint8_t*));
    inverses = calloc(256, sizeof(uint8_t));
    for(int i=0; i < 256; i++)
    {
        matrix[i] = calloc(256, sizeof(uint8_t));
        for(int j=0; j <= i; j++)
        {
            matrix[i][j] = full_galois_multiply(i, j);
            matrix[j][i] = matrix[i][j];
            if(matrix[i][j] == 1)
            {
                inverses[i] = j;
                inverses[j] = i;
            }
        }
    }
}

void free_multiplication_table()
{
    for(int i=0; i < 256; i++)
        free(matrix[i]);
    free(matrix);
    free(inverses);
}

void print_multiplication_table()
{
    printf("  |\t");
    for(int j=0; j < 256; j++)
        printf("%d\t", j);
    for(int i=0; i < 256; i++)
    {
        printf("\n%d |\t", i);
        for(int j=0; j < 256; j++)
            printf("%d\t", matrix[i][j]);
    }
    printf("\n");
}

void print_inverses()
{
    printf("Number\t->\tG(256) Inverse\n");
    for(int i=0; i < 256; i++)
        printf("%d\t->\t%d\n", i, inverses[i]);
}

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

uint8_t galois_divide(uint8_t a, uint8_t b)
{
	return galois_multiply(a, galois_inverse(b));
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

uint8_t F(uint8_t x, uint8_t* s, int k)
{
	int result = 0;
	for(int i=0; i < k; i++)
	{
		int aux = s[i];
		for(int j=0; j < i; j++)
			aux = galois_multiply(aux, x);
		result = galois_sum(result, aux);
	}
	return result;
}

// Returns 0 if num has an even amount of 1s.
// Returns 1 if num has an odd amount of 1s.
int parity_bit(uint8_t num)
{
	int sum = 0;
	for (int i = 0; i < 8; i++)
	{
		int bit = num & 0x01;
		sum += bit;
		num = num >> 1;
	}
	return sum % 2;
}

// Applies F(X) transformation to a XWVU block using the polynomial "s"
void T(uint8_t* xwvu, uint8_t* s, int k)
{
	uint8_t t = F(xwvu[0], s, k);
	uint8_t first_three = (t & 0xE0) >> 5;
	uint8_t middle_three = (t & 0x1C) >> 2;
	uint8_t last_two = t & 0x03;
	// Add parity bit in the sixth position
	uint8_t last_three = last_two;
	if(parity_bit(t) == 1)
		last_three = last_two | 0x04;

	xwvu[1] = (xwvu[1] & 0xF8) | first_three;
	xwvu[2] = (xwvu[2] & 0xF8) | middle_three;
	xwvu[3] = (xwvu[3] & 0xF8) | last_three;
}

// Extracts and returns the value of F(X) given a transformed XWVU block
// Returns -1 if inconsistency was detected.
int T_inverse(uint8_t* xwvu)
{
	uint8_t first_three = (xwvu[1] & 0x07) << 5;
	uint8_t middle_three = (xwvu[2] & 0x07) << 2;
	uint8_t last_two = xwvu[3] & 0x03;
	uint8_t number = first_three | middle_three | last_two;
	uint8_t parity = (xwvu[3] & 0x04) >> 2;
	if(parity != parity_bit(number))
	{
		fprintf(stderr, "ERROR. One of the images is corrupted.\n");
		return -1;
	}
	return number;
}

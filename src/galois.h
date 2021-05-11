#ifndef GALOIS_H
#define GALOIS_H
#include <stdint.h>

uint8_t galois_sum(uint8_t a, uint8_t b);
uint8_t galois_multiply(uint8_t a, uint8_t b);
uint8_t galois_inverse(uint8_t x);
void load_multiplication_table();
void free_multiplication_table();
void print_multiplication_table();
void print_inverses();

#endif

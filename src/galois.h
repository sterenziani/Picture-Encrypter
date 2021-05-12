#ifndef GALOIS_H
#define GALOIS_H
#include <stdint.h>

uint8_t galois_sum(uint8_t a, uint8_t b);
uint8_t galois_multiply(uint8_t a, uint8_t b);
uint8_t galois_divide(uint8_t a, uint8_t b);
uint8_t galois_inverse(uint8_t x);
uint8_t F(uint8_t x, uint8_t* s, int k);
void T(uint8_t* xwvu, uint8_t* s, int k);
int T_inverse(uint8_t* xwvu);
void load_multiplication_table();
void free_multiplication_table();
void print_multiplication_table();
void print_inverses();

#endif

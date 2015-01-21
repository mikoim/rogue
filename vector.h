#ifndef HEADER_VECTOR_H
#define HEADER_VECTOR_H

#include <sys/types.h>

typedef struct {
    int64_t x, y, z;
} Vector;

/* This vector functions is not a vector. */

Vector vector_assignment(int64_t x, int64_t y, int64_t z);

Vector vector_addition(const Vector *a, const Vector *b);

Vector vector_subtraction(const Vector *a, const Vector *b);

Vector vector_multiplication(const Vector *a, const Vector *b);

Vector vector_division(const Vector *a, const Vector *b);

#endif
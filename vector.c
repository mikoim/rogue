#include <sys/types.h>
#include "vector.h"

Vector vector_assignment(int64_t x, int64_t y, int64_t z) {
    Vector r = {x, y, z};

    return r;
}

Vector vector_addition(const Vector *a, const Vector *b) {
    Vector r;

    r.x = a->x + b->x;
    r.y = a->y + b->y;
    r.z = a->z + b->z;

    return r;
}

Vector vector_subtraction(const Vector *a, const Vector *b) {
    Vector r;

    r.x = a->x - b->x;
    r.y = a->y - b->y;
    r.z = a->z - b->z;

    return r;
}

Vector vector_multiplication(const Vector *a, const Vector *b) {
    Vector r;

    r.x = a->x * b->x;
    r.y = a->y * b->y;
    r.z = a->z * b->z;

    return r;
}

Vector vector_division(const Vector *a, const Vector *b) {
    Vector r;

    r.x = a->x / b->x;
    r.y = a->y / b->y;
    r.z = a->z / b->z;

    return r;
}
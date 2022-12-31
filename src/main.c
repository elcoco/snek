#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "snake.h"

#define XSIZE 10
#define YSIZE 10

#define NFOOD 1

int main()
{
    struct Snake s;
    struct Matrix m;

    snake_init(&s);
    matrix_init(&m, XSIZE, YSIZE, NFOOD);

    while (matrix_next(&m, &s, VEL_W) >= 0) {
        snake_debug(&s);
        matrix_debug(&m, &s);
        sleep(1);
    }
}

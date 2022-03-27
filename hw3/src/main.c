#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    double* ptr = sf_malloc(sizeof(double));

    sf_free(ptr);

    return EXIT_SUCCESS;
}


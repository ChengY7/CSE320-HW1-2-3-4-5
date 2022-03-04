#include <stdlib.h>
#include <stdio.h>

extern int original_main(int argc, char **argv);

int main(int argc, char **argv) {
    original_main(argc, argv);
}

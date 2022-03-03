#include <stdlib.h>
#include <stdio.h>

extern int original_main(int argc, char *const *argv);

int main(int argc, char *const *argv) {
    original_main(argc, argv);
}

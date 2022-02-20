#include <stdio.h>
#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv)) //if validargs==-1 then we failed so EXIT_FAILURE
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    // TO BE IMPLEMENTED
    if(global_options == VALIDATE_OPTION) {
        struct argo_value *test = argo_read_value(stdin);
        if(test!=NULL){
            return EXIT_SUCCESS;
        }
    }
    if(global_options >= CANONICALIZE_OPTION) {
        struct argo_value *test = argo_read_value(stdin);
        if(test!=NULL) {
            if(argo_write_value(test, stdout)==0)
                return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */

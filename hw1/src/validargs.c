#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"
#include "function.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    if (argc==1) {
        return -1;
    }
    argv++;
    char *h="-h";
    char *v="-v";
    char *c="-c";
    char *p="-p";
    if (compare_string(h, *argv)) {
        global_options=HELP_OPTION;
        return 0;
    }
    else if(compare_string(v, *argv)) {
        if(argc>2) {
            global_options=0;
            return -1;
        }
        global_options=VALIDATE_OPTION;
        return 0;
    }
    else if(compare_string(c, *argv)) {
        if(argc>4) {
            global_options=0;
            return -1;
        }
        if(argc==2) {
            global_options=CANONICALIZE_OPTION;
            return 0;
        }
        if(argc==3) {
            argv++;
            if(compare_string(p, *argv)) {
                global_options=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION+4;
                return 0;
            }
            else {
                global_options=0;
                return -1;
            }
        }
        if(argc==4) {
            argv++;
            if(compare_string(p, *argv)) {
                argv++;
                if(is_int(*argv)) {
                    if(str_to_int(*argv)<256 && str_to_int(*argv)>-1) {
                        global_options=CANONICALIZE_OPTION+PRETTY_PRINT_OPTION+str_to_int(*argv);
                        return -0;
                    } 
                    else {
                        global_options=0;
                        return -1;
                    }
                }
                else {
                    global_options=0;
                    return -1;
                }
            }
            else {
                global_options=0;
                return -1;
            }
        }
    }
    global_options=0;
    return -1;
}
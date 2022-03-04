/*********************/
/* errmsg.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"  /* Makes sure we're consistent with the declarations. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char * errmsg=NULL;

void set_error(char *msg) {
    errmsg = strdup(msg);
}
int is_error() {
    if (!errmsg)
        return 0;
    return 1;
}
int report_error(FILE *file) {
    if(!(is_error())) {
        return 0;
    }
    if(is_error()) {
        if (fputs(errmsg, file)==EOF)
            return -1;
        else
            return 0;
    }
    return -1;
}
void clear_error() {
    if(is_error()) {
        errmsg=NULL;
        free(errmsg);
    }
}

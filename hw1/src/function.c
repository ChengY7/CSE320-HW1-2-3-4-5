#include "function.h"
#include <stdlib.h>
#include <stdio.h>
int is_int(char *str) {
    while(*str!='\0') {
        if(*str<48 || *str>57) {
            return 0;
        }
        str++;
    }
    return 1;
}
int str_to_int(char *str) {
    int number=0;
    while(*str!='\0') {
        number*=10;
        number+=*str-48;
        str++;
    }
    return number;
}
int str_length(char *str) {
    int len=0;
    while (*str!='\0') {
        len++;
        str++;
    }
    return len;
}
int compare_string(char *one, char *two) {
    if (str_length(one) != str_length(two)) {
        return 0;
    }
    while(*one!='\0') {
        if(*one != *two) {
            return 0;
        }
        one++;
        two++;
    }
    return 1;
    
}

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
        argo_read_value(stdin);
    }
    if(global_options == CANONICALIZE_OPTION) {
        struct argo_value *test = argo_read_value(stdin);
        // printf("%p\n", test);
        // printf("%d\n", test->type);
        struct argo_object obj = (*test).content.object;
        struct argo_object *objp = &obj;   //Points to the datastructure
        struct argo_value *objectPointer = objp->member_list;  //Points to the head of the content
        while(objectPointer->next != objp-> member_list) {
            //printf("%p\n", objectPointer);
            objectPointer=objectPointer->next; //Points to members starting with the first element
            struct argo_string name = objectPointer->name;
            struct argo_string *namep = &name;
            printf("%ls\n", name.content);
            //printf("%d\n", *namep->content);
            struct argo_number num = objectPointer->content.number;
            printf("%ld\n", num.int_value);
        }
        
     
       


        /*
        struct argo_array arr = (*test).content.array;
        struct argo_array *arrp = &arr;
        struct argo_value *arrayPointer = arrp->element_list;
        printf("%p\n", arrayPointer);
        arrayPointer = arrayPointer->next;
        printf("%p\n", arrayPointer);
        */
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */

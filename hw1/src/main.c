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
            printf("-Name: ");
            printf("%ls\n", name.content);
            printf("%s", "     ->type: ");
            printf("%d\n", objectPointer->type);
            if(objectPointer->type==2) {
                struct argo_number num = objectPointer->content.number;
                printf("%s", "     ->Valid_String: ");
                printf("%d", num.valid_string);
                printf("%s", "     String value: ");
                printf("%ls\n", (num.string_value.content));
                printf("%s", "     ->Valid_Int: ");
                printf("%d", num.valid_int);
                printf("%s", "        Int value: ");
                printf("%ld\n", num.int_value);
                printf("%s", "     ->Valid_Float: ");
                printf("%d", num.valid_float);
                printf("%s", "      Float value: ");
                printf("%f\n", num.float_value);
            }
            else if(objectPointer->type==3) {
                struct argo_string str = objectPointer->content.string;
                printf("%s%ls\n", "     ->String value: ", str.content);
            }
            else if(objectPointer->type==1) {
                int t = objectPointer->content.basic;
                if (t==0) {
                    printf("%s\n", "     ->Basic value: null");
                }
                else if (t==1) {
                    printf("%s\n", "     ->Basic value: true");
                }
                else if (t==2) {
                    printf("%s\n", "     ->Basic value: false");
                }
            }
            
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

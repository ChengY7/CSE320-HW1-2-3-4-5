#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "program.h"
static struct d_storage store_sentinal;
int digit_len(long num) {
    if(num==0)
        return 1;
    int i=0;
    while(num!=0) {
        num=num/10;
        i++;
    }
    return i;
}
int is_num(char *str) {
    if(*str=='-')
        str++;
    while(*str!='\0') {
        if(*str<48 || *str>57) {
            return 0;
        }
        str++;
    }
    return 1;
}
long strToNum(char *str) {
    long i=0;
    int neg=0;
    if (*str=='-') {
        str++;
        neg=1;
    }
    while(*str!='\0') {
        i=i*10;
        i=i+*str-48;
        str++;
    }
    if (neg)
        i=i*-1;
    return i;
}
/*
 * This is the "data store" module for Mush.
 * It maintains a mapping from variable names to values.
 * The values of variables are stored as strings.
 * However, the module provides functions for setting and retrieving
 * the value of a variable as an integer.  Setting a variable to
 * an integer value causes the value of the variable to be set to
 * a string representation of that integer.  Retrieving the value of
 * a variable as an integer is possible if the current value of the
 * variable is the string representation of an integer.
 */

/**
 * @brief  Get the current value of a variable as a string.
 * @details  This function retrieves the current value of a variable
 * as a string.  If the variable has no value, then NULL is returned.
 * Any string returned remains "owned" by the data store module;
 * the caller should not attempt to free the string or to use it
 * after any subsequent call that would modify the value of the variable
 * whose value was retrieved.  If the caller needs to use the string for
 * an indefinite period, a copy should be made immediately.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @return  A string that is the current value of the variable, if any,
 * otherwise NULL.
 */
char *store_get_string(char *var) {
    struct d_storage *pointer = store_sentinal.next;
    while(pointer!=&store_sentinal) {
        if(strcmp(pointer->data->var, var)==0) {
            if(pointer->data->val==NULL)
                return NULL;
            else 
                return pointer->data->val;
        }
        else 
            pointer=pointer->next;
    }
    return NULL;
}

/**
 * @brief  Get the current value of a variable as an integer.
 * @details  This retrieves the current value of a variable and
 * attempts to interpret it as an integer.  If this is possible,
 * then the integer value is stored at the pointer provided by
 * the caller.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @param  valp  Pointer at which the returned value is to be stored.
 * @return  If the specified variable has no value or the value
 * cannot be interpreted as an integer, then -1 is returned,
 * otherwise 0 is returned.
 */
int store_get_int(char *var, long *valp) {
    struct d_storage *pointer = store_sentinal.next;
    while(pointer!=&store_sentinal) {
        if(strcmp(pointer->data->var, var)==0) {
            if(pointer->data->val==NULL)
                return -1;
            if(is_num(pointer->data->val)==0)
                return -1;
            *valp=strToNum(pointer->data->val);
                return 0;
        }
        else 
            pointer=pointer->next;
    }
    return -1;
}

/**
 * @brief  Set the value of a variable as a string.
 * @details  This function sets the current value of a specified
 * variable to be a specified string.  If the variable already
 * has a value, then that value is replaced.  If the specified
 * value is NULL, then any existing value of the variable is removed
 * and the variable becomes un-set.  Ownership of the variable and
 * the value strings is not transferred to the data store module as
 * a result of this call; the data store module makes such copies of
 * these strings as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set, or NULL if the variable is to become
 * un-set.
 */
int store_set_string(char *var, char *val) {
    if(store_sentinal.next==NULL || store_sentinal.prev==NULL) {
        store_sentinal.next=&store_sentinal;
        store_sentinal.prev=&store_sentinal;
    }
    struct d_storage *pointer = store_sentinal.next;
    while(pointer!=&store_sentinal) {
        if(strcmp(pointer->data->var, var)==0) {
            if(val==NULL) {
                free(pointer->data->val);
                pointer->data->val=NULL;
                return 0;
            }
            if(pointer->data->val!=NULL) {
                free(pointer->data->val);
            }
            char *newVal = (char*) malloc(strlen(val));
            strcpy(newVal, val);
            pointer->data->val=newVal;
            return 0;
        }
        else
            pointer=pointer->next;
    }
    char *newVar = (char*) malloc(strlen(var));
    char *newVal;
    if(val!=NULL) {
        newVal = (char*) malloc(strlen(val));
        strcpy(newVal, val);
    }
    strcpy(newVar, var);
    struct d_storage *newNode = malloc(sizeof(struct d_storage));
    struct data *dataPointer = malloc(sizeof(struct data));
    dataPointer->var=newVar;
    if(val!=NULL)
        dataPointer->val=newVal;
    else    
        dataPointer->val=NULL;
    newNode->data=dataPointer;
    store_sentinal.prev->next=newNode;
    newNode->prev=store_sentinal.prev;
    store_sentinal.prev=newNode;
    newNode->next=&store_sentinal;
    return 0;
}

/**
 * @brief  Set the value of a variable as an integer.
 * @details  This function sets the current value of a specified
 * variable to be a specified integer.  If the variable already
 * has a value, then that value is replaced.  Ownership of the variable
 * string is not transferred to the data store module as a result of
 * this call; the data store module makes such copies of this string
 * as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set.
 */
int store_set_int(char *var, long val) {
    if(store_sentinal.next==NULL || store_sentinal.prev==NULL) {
        store_sentinal.next=&store_sentinal;
        store_sentinal.prev=&store_sentinal;
    }
    struct d_storage *pointer = store_sentinal.next;
    while(pointer!=&store_sentinal) {
        if(strcmp(pointer->data->var, var)==0) {
            free(pointer->data->val);
            char *newVal = (char*) malloc(digit_len(val));
            sprintf(newVal, "%ld", val);
            pointer->data->val=newVal;
            return 0;
        }
        pointer=pointer->next;
    }
    char *newVar = (char*) malloc(strlen(var));
    strcpy(newVar, var);
    char *newVal = (char*) malloc(digit_len(val));
    sprintf(newVal, "%ld", val);
    struct d_storage *newNode = malloc(sizeof(struct d_storage));
    struct data *dataPointer = malloc(sizeof(struct data));
    dataPointer->var=newVar;
    dataPointer->val=newVal;
    newNode->data=dataPointer;
    store_sentinal.prev->next=newNode;
    newNode->prev=store_sentinal.prev;
    store_sentinal.prev=newNode;
    newNode->next=&store_sentinal;
    return 0;
}

/**
 * @brief  Print the current contents of the data store.
 * @details  This function prints the current contents of the data store
 * to the specified output stream.  The format is not specified; this
 * function is intended to be used for debugging purposes.
 *
 * @param f  The stream to which the store contents are to be printed.
 */
void store_show(FILE *f) {
    if (store_sentinal.next==NULL || store_sentinal.prev==NULL) {
        fprintf(f, "{}");
        return;
    }
    fprintf(f, "{");
    struct d_storage *pointer = store_sentinal.next;
    while(pointer!=&store_sentinal) {
        if(pointer->next==&store_sentinal)
            fprintf(f, "%s=%s", pointer->data->var, pointer->data->val);
        else
            fprintf(f, "%s=%s,", pointer->data->var, pointer->data->val);
        pointer=pointer->next;
    }
    fprintf(f, "}");
}

#include <stdio.h>
#include "mush.h"
typedef struct p_storage {
    STMT *statement;
    struct p_storage *next;
    struct p_storage *prev;
} P_STORAGE;
typedef struct data {
    char* var;
    char* val;
} DATA;
typedef struct d_storage {
    DATA *data;
    struct d_storage *next;
    struct d_storage *prev;
} D_STORAGE;             
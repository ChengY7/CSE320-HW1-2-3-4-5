#include <stdio.h>
typedef struct p_storage {
    STMT *statement;
    struct p_storage *next;
    struct p_storage *prev;
} P_STORAGE;
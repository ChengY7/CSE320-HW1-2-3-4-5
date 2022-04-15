#include <stdio.h>
#include "mush.h"
#define NEW "new"
#define RUNNING "running"
#define CANCELED "canceled"
#define COMPLETED "completed"
#define ABORTED "aborted"
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
typedef enum {
    new,
    running,
    canceled,
    completed,
    aborted
} STATUS;  
typedef struct job {
    int jobid;
    pid_t pid;
    STATUS status;
    PIPELINE *pipeline;
} JOB;     
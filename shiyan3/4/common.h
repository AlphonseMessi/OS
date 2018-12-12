#ifndef COMMOM_H_
#define COMMOM_H_
#include <sys/sem.h>   
#define check_error(err)                                                 \
    do                                                                   \
    {                                                                    \
        if (err < 0)                                                     \
        {                                                                \
            fprintf(stderr, "warning is in %d ,warin code is %d\n", __LINE__, err); \
            exit(1);                                                     \
        }                                                                \
    } while (0);

#define check_NULL(err)                                             \
    do                                                              \
    {                                                               \
        if (err == NULL)                                            \
        {                                                           \
            fprintf(stderr, "warning is in %d,it can not be NULL\n", __LINE__); \
            exit(1);                                                \
        }                                                           \
    } while (0);

#define name "mutex"
#define MAX_SIZE 512
#define key 1024

typedef struct
{
    char text[MAX_SIZE];
} message;

#endif

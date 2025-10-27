#include <stdio.h>
#include <stdlib.h>

#define LOG(...)                                \
    do {                                        \
        printf("%s:%d: ", __FILE__, __LINE__);  \
        printf(__VA_ARGS__);                    \
        puts("");                               \
    } while(0)

#define FATAL(...)                                      \
    do {                                                \
        fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                   \
        fprintf(stderr, "\n");                          \
        exit(1);                                        \
    } while(0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
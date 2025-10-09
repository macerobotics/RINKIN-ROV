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

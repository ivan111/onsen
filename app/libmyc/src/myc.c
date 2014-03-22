#ifndef HEADER_MYC
#define HEADER_MYC

#include <stdarg.h>

#define ASSERT(cond, fmt, ...) do {                                             \
    if (!(cond)) {                                                              \
        printf("**** ASSERT ****\n");                                           \
        printf("FILE: %s, FUNC: %s, LINE: %d\n", __FILE__, __func__, __LINE__); \
        printf("COND: %s\n", #cond);                                            \
        printf(fmt "\n", ##__VA_ARGS__);                                        \
    }                                                                           \
} while (0)

#define ASSERT_RET(ret, cond, fmt, ...) do {                                    \
    if (!(cond)) {                                                              \
        printf("**** ASSERT ****\n");                                           \
        printf("FILE: %s, FUNC: %s, LINE: %d\n", __FILE__, __func__, __LINE__); \
        printf("COND: %s\n", #cond);                                            \
        printf(fmt "\n", ##__VA_ARGS__);                                        \
        return ret;                                                             \
    }                                                                           \
} while (0)


#define STRCMP(a, R, b)  (strcmp(a,b) R 0)
#define STRNCMP(a, R, b, n)  (strncmp(a,b,n) R 0)
#define MAX(x,y)  (((x) > (y)) ? (x) : (y))
#define MIN(x,y)  (((x) < (y)) ? (x) : (y))


int  strlen(const char *s);
char *strcpy(char *s, const char *t);
char *strncpy(char *s, const char *t, int n);
char *strcat(char *s, const char *t);
int  strcmp(const char *s, const char *t);
int  strncmp(const char *s, const char *t, int n);

int  atoi(const char *s);

int  printf(const char *fmt, ...);
int  snprintf(char *s, unsigned int n, const char *fmt, ...);
int  vsnprintf(char *s, unsigned int n, const char *fmt, va_list ap);

int   memcmp(const void *buf1, const void *buf2, unsigned int);
void *memcpy(void *dst, const void *src, unsigned int);
void *memmove(void *dst, const void *src, unsigned int);
void *memset(void *dst, int c, unsigned int count);

void *sbrk(int sz);
void *malloc(unsigned int size_B);
int free(void *p);

#endif


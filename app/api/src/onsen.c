#ifndef HEADER_ONSEN
#define HEADER_ONSEN

#include <stdarg.h>
#include "keycode.h"
#include "msg.h"

struct MSG;

typedef struct _POINT {
    int x, y;
} POINT;

#define RGB(r, g, b) ((unsigned int) (((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF))

void exit(int status);
int chopsticks(void);
int get_message(struct MSG *msg);
int getkey(int wait);

int write(int fd, const void *buf, int cnt);


//-----------------------------------------------------------------------------
// タイマ
//-----------------------------------------------------------------------------

int  timer_new(void);
void timer_free(int tid);
void timer_start(int tid, unsigned int timeout_ms);


//-----------------------------------------------------------------------------
// グラフィック
//-----------------------------------------------------------------------------

int  create_window(int w, int h, const char *name);
void update_surface(int sid);
void fill_surface(int sid, unsigned int color);
void draw_pixel(int sid, int x, int y, unsigned int color);
void draw_line(int sid, POINT *pt0, POINT *pt1, unsigned int color);
void draw_text(int sid, int x, int y, unsigned int color, const char *s);


//-----------------------------------------------------------------------------
// ライブラリ
//-----------------------------------------------------------------------------

int  strlen(const char *s);
char *strcpy(char *s, const char *t);
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


#endif


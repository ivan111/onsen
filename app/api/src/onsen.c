#ifndef HEADER_ONSEN
#define HEADER_ONSEN

#include <stdarg.h>
#include "keycode.h"
#include "msg.h"

struct MSG;

struct POINT {
    int x, y;
};

struct RECT{
    int x, y;
    int w, h;
};

#define RGB(r, g, b) ((unsigned int) (((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF))

void exit(int status);
int fork(void);
int write(int fd, const void *buf, int cnt);
int execve(const char *fname, char *const argv[], char *const envp[]);
int getpid(void);
int brk(void *new_brk);

int get_message(struct MSG *msg);
int getkey(int wait);

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
void update_char(int sid, int x, int y);
void fill_surface(int sid, unsigned int color);
void fill_rect(int sid, int x, int y, int w, int h, unsigned int color);
void scroll_surface(int sid, int cx, int cy);
void draw_pixel(int sid, int x, int y, unsigned int color);
void draw_line(int sid, int x0, int y0, int x1, int y1, unsigned int color);
void draw_text(int sid, int x, int y, unsigned int color, const char *s);
void draw_text_bg(int sid, int x, int y, unsigned int fg_color, unsigned int bg_color, const char *s);
void erase_char(int sid, int x, int y, unsigned long color, int update);


#endif


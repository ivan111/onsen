// Generated by mkhdr

#ifndef HEADER_HARIBOTE
#define HEADER_HARIBOTE

void api_putchar(char ch);
void api_putstr0(const char *s);
void api_putstr1(const char *s, int cnt);
void api_end(void);
int  api_openwin(char *buf, int w, int h, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
int  api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
int  api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_closewin(int win);
int  api_getkey(int mode);

void api_putintx(int n);

#endif

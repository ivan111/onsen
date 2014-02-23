#ifndef HEADER_HARIBOTE
#define HEADER_HARIBOTE

#define API_PUTCHAR  0x01
void api_putchar(char ch);


#define API_PUTSTR0  0x02
void api_putstr0(const char *s);


#define API_PUTSTR1  0x03
void api_putstr1(const char *s, int cnt);


#define API_END      0x04
void api_end(void);

#endif




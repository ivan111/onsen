#ifndef HEADER_ONSEN
#define HEADER_ONSEN

#include "keycode.h"
#include "msg.h"

#define API_EXIT_APP      1
void exit_app(int status);


#define API_GET_MESSAGE   9
struct MSG;

int get_message(struct MSG *msg);


//-----------------------------------------------------------------------------
// グラフィック
//-----------------------------------------------------------------------------

#define RGB(R, G, B) ((unsigned int) (((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

#define COL_BLACK    RGB(  0,   0,   0)
#define COL_RED      RGB(255,   0,   0)
#define COL_GREEN    RGB(  0, 255,   0)
#define COL_BLUE     RGB(  0,   0, 255)
#define COL_WHITE    RGB(255, 255, 255)


#define API_GET_SCREEN  1000
int get_screen(void);


#define API_UPDATE_SCREEN  1001
void update_screen(int sid);


#define API_FILL_SURFACE  1002
void fill_surface(int sid, unsigned int color);

#define API_DRAW_TEXT     1005
void draw_text(int sid, int x, int y, unsigned int color, const char *s);


//-----------------------------------------------------------------------------
// デバッグ
//-----------------------------------------------------------------------------

#define API_DBG_STR     9000
void dbg_str(const char *s);


#define API_DBG_INT     9001
void dbg_int(int val);


#define API_DBG_INTX    9002
void dbg_intx(unsigned int val);


#define API_DBG_NEWLINE 9003
void dbg_newline(void);




#endif




// Generated by mkhdr

#ifndef HEADER_GRAPHIC
#define HEADER_GRAPHIC

#include <stdbool.h>

#define ERROR_SID      (-1)
#define NO_PARENT_SID  (-2)

typedef unsigned short COLOR;

#define RGB(R, G, B) ((COLOR) (((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))
#define RGB2(rgb)    ((COLOR) (((rgb) & 0xF80000) >> 8) | (((rgb) & 0xFC00) >> 5) | (((rgb) & 0xF8) >> 3))

#define GET_RED(RGB)   (((RGB) & 0xF800) >> 8)
#define GET_GREEN(RGB) (((RGB) & 0x07E0) >> 3)
#define GET_BLUE(RGB)  (((RGB) & 0x001F) << 3)

#define COL_BLACK    RGB(  0,   0,   0)
#define COL_RED      RGB(255,   0,   0)
#define COL_GREEN    RGB(  0, 255,   0)
#define COL_BLUE     RGB(  0,   0, 255)
#define COL_WHITE    RGB(255, 255, 255)

#define BORDER_WIDTH      (3)
#define TITLE_BAR_HEIGHT  (19)
#define WINDOW_EXT_WIDTH  (BORDER_WIDTH * 2)
#define WINDOW_EXT_HEIGHT (TITLE_BAR_HEIGHT + (BORDER_WIDTH * 2))

#define CLIENT_X  (BORDER_WIDTH)
#define CLIENT_Y  (TITLE_BAR_HEIGHT + BORDER_WIDTH)

#define HANKAKU_W 8   ///< 半角フォントの幅
#define HANKAKU_H 16  ///< 半角フォントの高さ



enum {
    OP_SRC_COPY,
    OP_SRC_INVERT
};


extern int g_vram_sid;
extern int g_dt_sid;

extern const int g_w;
extern const int g_h;


void graphic_init(COLOR *vram);

int  new_window(int x, int y, int cw, int ch, char *title);

int  new_surface(int parent_sid, int w, int h);
int  new_surface_from_buf(int parent_sid, int w, int h, COLOR *buf);
void free_surface(int sid);
void free_surface_task(int pid);
int  get_screen(void);

void set_sprite_pos(int sid, int x, int y);
void move_sprite(int sid, int dx, int dy);

void update_surface(int sid);
void update_rect(int sid, int x, int y, int w, int h);
void update_char(int sid, int x, int y);

void draw_sprite(int src_sid, int dst_sid, int op);
void blit_surface(int src_sid, int src_x, int src_y, int w, int h,
        int dst_sid, int dst_x, int dst_y, int op);

void fill_surface(int sid, COLOR color);
void fill_rect(int sid, int x, int y, int w, int h,
        COLOR color);
void draw_text(int sid, int x, int y, COLOR color,
        const char *text);
void draw_text_bg(int sid, int x, int y, COLOR color,
        COLOR bg_color, const char *text);

void draw_pixel(int sid, unsigned int x, unsigned int y, COLOR color);

void erase_char(int sid, int x, int y, COLOR color, bool update);

void scroll_surface(int sid, int cx, int cy);

void set_colorkey(int sid, COLOR colorkey);
void clear_colorkey(int sid);
void set_alpha(int sid, unsigned char alpha);
void clear_alpha(int sid);

void set_mouse_pos(int x, int y);

int  get_screen_pid(void);
void switch_screen(void);
void switch_debug_screen(void);

void graphic_dbg(void);

#endif

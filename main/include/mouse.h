// Generated by mkhdr

#ifndef HEADER_MOUSE
#define HEADER_MOUSE

#include <stdbool.h>


typedef struct MOUSE_DECODE {
    // ---- デコード結果
    int x;   ///< マウス位置X
    int y;   ///< マウス位置Y
    bool btn_left;
    bool btn_right;
    bool btn_center;
    bool change_pos;

    // ---- デコード処理用
    int phase;
    unsigned char buf[3];
    int dx;  ///< マウス位置の変化分X
    int dy;  ///< マウス位置の変化分Y
} MOUSE_DECODE;


void mouse_init(void);
void int2C_handler(int *esp);
MOUSE_DECODE *mouse_decode(unsigned char data);

#endif

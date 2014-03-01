// Generated by mkhdr

#ifndef HEADER_GBUFFER
#define HEADER_GBUFFER

#include <stdbool.h>
#include "color.h"

#define GBF_FLG_HAS_COLORKEY    (1 << 0)
#define GBF_FLG_HAS_ALPHA       (1 << 1)

struct _GBUFFER_METHOD;

typedef struct _GBUFFER {
    void *buf;

    int w;
    int h;
    int color_width;

    unsigned int flags;

    COLOR32 colorkey;  // 透明にする色。SRF_FLG_HAS_COLORKEYで有効。
    unsigned char alpha;

    struct _GBUFFER_METHOD *m;
} GBUFFER;


typedef struct _GBUFFER_METHOD {
    COLOR32 (*get)(GBUFFER *self, int x, int y);
    void (*put)(GBUFFER *self, int x, int y, COLOR32 color);
    void (*fill_rect)(GBUFFER *self, int x, int y, int w, int h, COLOR32 color);
    void (*blit)(GBUFFER *self, int src_x, int src_y, int w, int h, GBUFFER *dst, int x, int y, int op);
} GBUFFER_METHOD;


extern struct _GBUFFER_METHOD *g_gbuf_method8;
extern struct _GBUFFER_METHOD *g_gbuf_method16;

#endif

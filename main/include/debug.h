// Generated by mkhdr

#ifndef HEADER_DEBUG
#define HEADER_DEBUG

#include <stdarg.h>

//-----------------------------------------------------------------------------
// メイン

void debug_main(void);


//-----------------------------------------------------------------------------
// 画面出力

// pushal でスタックに格納されるレジスタ
typedef struct REGISTER {
    int edi, esi, ebp, ebx, edx, ecx, eax;
} REGISTER;


#define DBGF(fmt, ...)  dbgf("%s %d %s : " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)


void dbgf(const char *fmt, ...);
void dbg_clear(void);
void dbg_reg(const REGISTER *r);
void dbg_seg(void);

void dbg_fault(const char *msg, int *esp);

#endif

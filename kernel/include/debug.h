// Generated by mkhdr

#ifndef HEADER_DEBUG
#define HEADER_DEBUG

#include <stdarg.h>
#include "asmfunc.h"
#include "file.h"
#include "stacktrace.h"

//-----------------------------------------------------------------------------
// メイン

int debug_main(void);


//-----------------------------------------------------------------------------
// 画面出力

struct INT_REGISTERS {
    // asm_inthander.S で積まれたスタックの内容
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  // pushal
    unsigned int ds, es;

    // 以下は、例外発生時にCPUが自動でpushしたもの
    unsigned int err_code;
    unsigned int eip, cs, eflags, app_esp, app_ss;
};


void temp_dbgf(const char *fmt, ...);
void dbgf(const char *fmt, ...);
void dbg_clear(void);
void dbg_seg(void);

void blue_screen(void);
void blue_screen_f(int line_no, const char *fmt, ...);

void dbg_fault(const char *msg, int no, struct INT_REGISTERS *regs);

extern struct FILE_T *f_debug;
extern struct FILE_T *f_dbg_temp;

extern int g_dbg_temp_flg;


#define ASSERT(cond, fmt, ...) do {                                           \
    if (!(cond)) {                                                            \
        dbgf("**** ASSERT ****\n");                                           \
        dbgf("FILE: %s, FUNC: %s, LINE: %d\n", __FILE__, __func__, __LINE__); \
        dbgf("COND: %s\n", #cond);                                            \
        dbgf(fmt "\n", ##__VA_ARGS__);                                        \
        stacktrace(5, f_debug);                                               \
        cli(); hlt();                                                         \
    }                                                                         \
} while (0)


// graphic_init前でも使えるASSERT
#define ASSERT2(cond, fmt, ...) do {                                                    \
    if (!(cond)) {                                                                      \
        blue_screen();                                                                  \
        blue_screen_f(0, "**** ASSERT ****");                                           \
        blue_screen_f(1, "FILE: %s, FUNC: %s, LINE: %d", __FILE__, __func__, __LINE__); \
        blue_screen_f(2, "COND: %s", #cond);                                            \
        blue_screen_f(3, fmt, ##__VA_ARGS__);                                           \
        cli(); hlt();                                                                   \
    }                                                                                   \
} while (0)


#define ERROR(fmt, ...) do {                                              \
    dbgf("**** ERROR ****\n");                                            \
    dbgf("FILE: %s, FUNC: %s, LINE: %d\n", __FILE__, __func__, __LINE__); \
    dbgf(fmt "\n", ##__VA_ARGS__);                                        \
    stacktrace(5, f_debug);                                               \
    for (;;) { hlt(); }                                                   \
} while (0)


#define DBGF(fmt, ...)  dbgf("%s %d %s : " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)


#endif

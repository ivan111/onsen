// Generated by mkhdr

#ifndef HEADER_DEBUG
#define HEADER_DEBUG


//-----------------------------------------------------------------------------
// メイン

void debug_main(void);


//-----------------------------------------------------------------------------
// 画面出力

// pushal でスタックに格納されるレジスタ
typedef struct REGISTER {
    int edi, esi, ebp, ebx, edx, ecx, eax;
} REGISTER;


#define DBG_HEADER do {                \
    dbg_str(__FILE__ " ");             \
    dbg_int(__LINE__);                 \
    dbg_str(" ");                      \
    dbg_str(__func__);                 \
    dbg_str(" : ");                    \
} while (0)


#define DBG_STR(str) do {              \
    DBG_HEADER;                        \
    dbg_str(str);                      \
    dbg_str("\n");                     \
} while (0)


#define DBG_INT(var) do {              \
    DBG_HEADER;                        \
    dbg_str(#var " = ");               \
    dbg_int(var);                      \
    dbg_str("\n");                     \
} while (0)


#define DBG_INTX(var) do {             \
    DBG_HEADER;                        \
    dbg_str(#var " = 0x");             \
    dbg_intx(var);                     \
    dbg_str("\n");                     \
} while (0)



void dbg_newline(void);
void dbg_char(char ch);
void dbg_str(const char *s);
void dbg_strln(const char *s);
void dbg_int(int n);
void dbg_intln(int n);
void dbg_intx(int n);
void dbg_intxln(int n);
void dbg_reg(const REGISTER *r);
void dbg_seg(void);

void dbg_seg_reg(unsigned short seg_reg);

void dbg_fault(const char *msg, int *esp);

#endif

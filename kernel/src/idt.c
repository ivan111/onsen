/**
 * IDT
 */

//=============================================================================
// 公開ヘッダ

#ifndef HEADER_IDT
#define HEADER_IDT

// API(システムコール)を呼び出すときの割り込み番号
#define IDT_API_NO  (0x44)


void idt_init(void);
void set_gate_desc(int no, unsigned short sel, void (*f)(void),
        int count, int type, int dpl);

#endif


//=============================================================================
// 非公開ヘッダ

#include "asm_inthandler.h"
#include "asmapi.h"
#include "memory.h"
#include "gdt.h"


// SET_INTR_GATE(0A) での展開なら以下のようになる。
// set_gate_desc(0x0A, KERNEL_CS, asm_int0A_handler, 0, SEG_TYPE_INTR_GATE, DPL0)

#define SET_INTR_GATE(NO) set_gate_desc(0x ## NO, KERNEL_CS,\
        asm_int ## NO ## _handler, 0, SEG_TYPE_INTR_GATE, DPL0)


struct GATE_DESC {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char count;
    unsigned char type;
    unsigned short offset_high;
} __attribute__ ((__packed__));


static struct GATE_DESC *idt = (struct GATE_DESC *) VADDR_IDT;


struct IDTR {
    unsigned short limit;
    unsigned long base;
} __attribute__ ((__packed__));


inline static void load_idtr(struct IDTR *idtr)
{
    __asm__ __volatile__ ("lidt (%0)" : : "q" (idtr));
}


//=============================================================================
// 関数

void idt_init(void)
{
    // 初期化
    for (int i = 0; i <= LIMIT_IDT / 8; i++) {
        set_gate_desc(i, 0, 0, 0, 0, 0);
    }

    struct IDTR idtr;
    idtr.limit = LIMIT_IDT;
    idtr.base = VADDR_IDT;
    load_idtr(&idtr);

    SET_INTR_GATE(00);
    SET_INTR_GATE(05);
    SET_INTR_GATE(06);
    SET_INTR_GATE(07);
    SET_INTR_GATE(0A);
    SET_INTR_GATE(0B);
    SET_INTR_GATE(0C);
    SET_INTR_GATE(0D);
    SET_INTR_GATE(0E);

    SET_INTR_GATE(20);
    SET_INTR_GATE(21);
    SET_INTR_GATE(2C);


    // 割り込み時点での割り込み許可フラグ(IF)の状態
    // 割り込みゲート : 割り込み禁止になる
    // トラップゲート : IF は変わらない
    // ２つの違いはこれのみ

    // システムコールのためのゲートを設定
    set_gate_desc(IDT_API_NO, KERNEL_CS, asm_api, 0, SEG_TYPE_TRAP_GATE, DPL3);
}


void set_gate_desc(int no, unsigned short sel, void (*f)(void),
        int count , int type, int dpl)
{
    struct GATE_DESC *gate = idt + no;

    int offset = (int) f;

    gate->offset_low = offset & 0xFFFF;
    gate->selector = sel;
    gate->count = count;
    gate->type = type | (dpl << 5);
    gate->offset_high = (offset >> 16) & 0xFFFF;
}


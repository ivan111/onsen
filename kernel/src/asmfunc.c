/**
 * アセンブラ関数
 *
 * C 言語で書けない命令を、インラインアセンブラで記述
 */

#ifndef HEADER_ASMFUNC
#define HEADER_ASMFUNC

#include <stdint.h>


inline __attribute__ ((always_inline))
static void hlt(void)
{
    __asm__ __volatile__ ("hlt");
}


inline __attribute__ ((always_inline))
static void stihlt(void)
{
    __asm__ __volatile__ ("sti\n    hlt");
}


inline __attribute__ ((always_inline))
static void cli(void)
{
    __asm__ __volatile__ ("cli");
}


inline __attribute__ ((always_inline))
static void sti(void)
{
    __asm__ __volatile__ ("sti");
}


static void inline __attribute__ ((always_inline))
out8 (uint16_t port,  uint8_t data)
{
    __asm__ __volatile__ (
        "outb %0, %1"

        :
        : "a" (data), "Nd" (port)  // N : 0 から 255 の範囲の定数
    );
}


static uint8_t inline __attribute__ ((always_inline))
in8 (uint16_t port)
{
    uint8_t ret;

    __asm__ __volatile__ (
        "inb %1, %0"

        : "=a" (ret)
        : "Nd" (port)  // N : 0 から 255 の範囲の定数
    );

    return ret;
}


static void inline __attribute__ ((always_inline))
out16 (uint16_t port,  uint16_t data)
{
    __asm__ __volatile__ (
        "outw %0, %1"

        :
        : "a" (data), "d" (port)
    );
}


static uint16_t inline __attribute__ ((always_inline))
in16 (uint16_t port)
{
    uint16_t ret;

    __asm__ __volatile__ (
        "inw %1, %0"

        : "=a" (ret)
        : "d" (port)
    );

    return ret;
}


static void inline __attribute__ ((always_inline))
out32 (uint16_t port,  uint32_t data)
{
    __asm__ __volatile__ (
        "outl %0, %1"

        :
        : "a" (data), "d" (port)
    );
}


static uint32_t inline __attribute__ ((always_inline))
in32 (uint16_t port)
{
    uint32_t ret;

    __asm__ __volatile__ (
        "inl %1, %0"

        : "=a" (ret)
        : "d" (port)
    );

    return ret;
}


inline __attribute__ ((always_inline))
static void outb(uint16_t port, uint8_t data)
{
    __asm__ __volatile__ (
        "outb %0, %1"

        :
        : "a" (data), "Nd" (port)  // N : 0 から 255 の範囲の定数
    );
}


inline __attribute__ ((always_inline))
static uint8_t inb(uint16_t port)
{
    uint8_t ret;

    __asm__ __volatile__ (
        "inb %1, %0"

        : "=a" (ret)
        : "Nd" (port)  // N : 0 から 255 の範囲の定数
    );

    return ret;
}


inline __attribute__ ((always_inline))
static void outw(uint16_t port, uint16_t data)
{
    __asm__ __volatile__ (
        "outw %0, %1"

        :
        : "a" (data), "d" (port)
    );
}


inline __attribute__ ((always_inline))
static uint16_t inw(uint16_t port)
{
    uint16_t ret;

    __asm__ __volatile__ (
        "inw %1, %0"

        : "=a" (ret)
        : "d" (port)
    );

    return ret;
}


inline __attribute__ ((always_inline))
static void outl(uint16_t port, uint32_t data)
{
    __asm__ __volatile__ (
        "outl %0, %1"

        :
        : "a" (data), "d" (port)
    );
}


inline __attribute__ ((always_inline))
static uint32_t inl(uint16_t port)
{
    uint32_t ret;

    __asm__ __volatile__ (
        "inl %1, %0"

        : "=a" (ret)
        : "d" (port)
    );

    return ret;
}


inline __attribute__ ((always_inline))
static int load_cr0(void)
{
    int cr0;

    __asm__ __volatile__ ("movl %%cr0, %0" : "=r" (cr0));

    return cr0;
}


inline __attribute__ ((always_inline))
static void store_cr0(int cr0)
{
    __asm__ __volatile__ ("movl %0, %%cr0" : : "r" (cr0));
}


inline __attribute__ ((always_inline))
static unsigned long load_cr3(void)
{
    unsigned long cr3;

    __asm__ __volatile__ ("movl %%cr3, %0" : "=r" (cr3));

    return cr3;
}


inline __attribute__ ((always_inline))
static void store_cr3(unsigned long cr3)
{
    __asm__ __volatile__ ("movl %0, %%cr3" : : "r" (cr3));
}


inline __attribute__ ((always_inline))
static void enable_paging(void)
{
    unsigned long temp;

    __asm__ __volatile__ (
        "movl  %%cr0, %0\n"
        "orl   $0x80000000, %0\n"
        "movl  %0, %%cr0"

        : "=r" (temp)
    );
}


inline __attribute__ ((always_inline))
static void flush_tlb(void)
{
    __asm__ __volatile__ (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"

        :
        :
        : "%eax"
    );
}


inline __attribute__ ((always_inline))
static void flush_tlb_single(unsigned long addr)
{
    __asm__ __volatile__ ("invlpg (%0)" : : "r" (addr) : "memory");
}


inline __attribute__ ((always_inline))
static int load_eflags(void)
{
    int eflags = 0;

    __asm__ __volatile__ (
        "pushf\n"
        "popl %0"

        : "=r" (eflags)
    );

    return eflags;
}


inline __attribute__ ((always_inline))
static void store_eflags(int eflags)
{
    __asm__ __volatile__ (
        "pushl %0\n"
        "popf"

        :
        : "a" (eflags)
    );
}


inline __attribute__ ((always_inline))
static void ltr(int tr)
{
    __asm__ __volatile__ ("ltr %0" : : "m" (tr));
}


inline __attribute__ ((always_inline))
static void far_jmp(unsigned short cs, unsigned long eip)
{
    struct {
        unsigned long eip;
        unsigned short cs;
    } __attribute__((__packed__)) FARJMP;

    FARJMP.cs = cs;
    FARJMP.eip = eip;

    __asm__ __volatile__ ("ljmp *(%0)" : : "q" (&FARJMP) : "memory");
}


inline __attribute__ ((always_inline))
static void far_call(unsigned short cs, unsigned long eip)
{
    struct {
        unsigned long eip;
        unsigned short cs;
    } __attribute__((__packed__)) FARCALL;

    FARCALL.cs = cs;
    FARCALL.eip = eip;

    __asm__ __volatile__ ("lcall *(%0)" : : "q" (&FARCALL) : "memory");
}


// GDT を変更したあとにセグメントを読みなおすのに使用する
inline __attribute__ ((always_inline))
static void reload_segments(unsigned short cs, unsigned short ds)
{
    // 「&&ラベル名」 はラベルのアドレスを取得できるgcc拡張
    far_jmp(cs, (unsigned long) &&reload_cs);

reload_cs:

    __asm__ __volatile__ (
        "movw %0, %%ds\n"
        "movw %0, %%es\n"
        "movw %0, %%fs\n"
        "movw %0, %%gs\n"
        "movw %0, %%ss"

        :
        : "q" (ds)
    );
}


#endif

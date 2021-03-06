// Generated by mkhdr

#ifndef HEADER_MEMORY
#define HEADER_MEMORY

#include <stdint.h>
#include "sysinfo.h"

//-----------------------------------------------------------------------------
// メモリマップ

#define SZ4MB  4 * 1024 * 1024  // 4MB のサイズ

#define VADDR_BASE          (0xC0000000)  // 論理アドレスのベースアドレス

#define _PA(vaddr)  ((vaddr) - VADDR_BASE)

#define VADDR_USER_ESP      (0xBFFFF000)

/* FREE LOW ADDR START      (0x00000500) */
#define VADDR_MBR           (0xC0000500)
#define VADDR_PARTITION_TBL (0xC0000500 + 0x1BE)  // パーティションテーブル
/* OS_PDTはCR3レジスタに設定するので物理アドレスでなければいけない */
#define MADDR_OS_PDT        (0x00001000)  // 4KB(0x1000)境界であること
#define VADDR_OS_PDT        (0xC0001000)  // 4KB(0x1000)境界であること
#define VADDR_SYS_INFO      (0xC0002000)  // システム情報が格納されているアドレス
#define VADDR_MMAP_TBL      (0xC0003000)  // 使用可能メモリ情報のテーブル
#define VADDR_VBR           (0xC0007C00)
#define VADDR_BITMAP_START  (0xC0010000)  // 物理アドレス管理用ビットマップ
#define VADDR_BITMAP_END    (0xC0030000)
#define VADDR_GDT           (0xC0030000)
#define LIMIT_GDT           (0x0000FFFF)
#define VADDR_IDT           (0xC0040000)
#define LIMIT_IDT           (0x000007FF)
#define VADDR_BMEM_MNG      (0xC0050000)
#define VADDR_VMEM_MNG      (0xC0060000)
/* FREE LOW ADDR END        (0x0009FFFF) */

#define VADDR_OS_TEXT       (0xC0100000)
#define VADDR_OS_STACK      (0xC0200000)

// FREE 2MB

#define MADDR_FREE_START    (0x00400000)
#define VADDR_MEM_START     (0xC0400000)
#define VADDR_VRAM          (0xE0000000)
#define VADDR_MEM_END       (VADDR_VRAM)
#define VADDR_PD_SELF       (0xFFFFF000)


extern struct SYSTEM_INFO *g_sys_info;


//-----------------------------------------------------------------------------
// メモリ管理

struct USER_PAGE {
    unsigned long vaddr;
    unsigned long end_vaddr;  // end_vaddr自体は含まれない
    int refs;
};


void  mem_init(void);

void *mem_alloc(unsigned int size_B);
void *mem_alloc_str(const char *s);
struct USER_PAGE *mem_alloc_user_page(unsigned long vaddr, int size_B, int flags);
int   mem_expand_user_page(struct USER_PAGE *page, unsigned long new_end);
int   mem_expand_stack(struct USER_PAGE *stack, unsigned long new_stack);
void *mem_alloc_maddr(void);

int mem_free(void *vp_vaddr);
int mem_free_user(struct USER_PAGE *page);
int mem_free_maddr(void *vp_maddr);

void  mem_dbg(void);

//-----------------------------------------------------------------------------
// メモリ容量確認

struct MEMORY_MAP_ENTRY {
    uint32_t base_low;     // base address QWORD
    uint32_t base_high;
    uint32_t length_low;   // length QWORD
    uint32_t length_high;
    uint16_t type;         // entry Ttpe
    uint16_t acpi;         // exteded
} __attribute__ ((packed));


unsigned int mem_total_B(void);
unsigned int mem_total_mfree_B(void);
unsigned int mem_total_vfree_B(void);

#endif

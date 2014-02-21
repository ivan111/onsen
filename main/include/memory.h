// Generated by mkhdr

#ifndef HEADER_MEMORY
#define HEADER_MEMORY

//-----------------------------------------------------------------------------
// メモリマップ

#define SZ4MB  4 * 1024 * 1024  // 4MB のサイズ

#define VADDR_BASE          (0xC0000000)  // 論理アドレスのベースアドレス

#define VADDR_SYS_INFO      (0xC0000A00)  // システム情報が格納されているアドレス
/* FREE START               (0x00001000) */
/* OS_PDTはCR3レジスタに設定するので物理アドレスでなければいけない */
#define MADDR_OS_PDT        (0x00001000)  // 4KB(0x1000)境界であること
#define VADDR_BITMAP_START  (0xC0010000)  // 物理アドレス管理用ビットマップ
#define VADDR_BITMAP_END    (0xC0030000)
#define VADDR_BMEM_MNG      (VADDR_BITMAP_END)
#define VADDR_VMEM_MNG      (VADDR_BMEM_MNG + 0x10000)
/* FREE END                 (0x0009FFFF) */
#define VADDR_DISK_IMG      (0xC0100000)
#define VADDR_IDT           (0xC026F800)
#define LIMIT_IDT           (0x000007FF)
#define VADDR_GDT           (0xC0270000)
#define LIMIT_GDT           (0x0000FFFF)
#define VADDR_OS            (0xC0280000)  // 最大 1,152 KB
#define VADDR_MEM_START     (0xC0400000)
#define VADDR_VRAM          (0xE0000000)
#define VADDR_MEM_END       (VADDR_VRAM)
#define VADDR_PD_SELF       (0xFFFFF000)

/* 0x4000境界であること。bitmapの初期化がそのことを前提に書かれているから */
#define MADDR_FREE_START    (0x00400000)



//-----------------------------------------------------------------------------
// メモリ管理

void  mem_init(void);
void *mem_alloc(unsigned int size_B);
void *mem_alloc_user(void *vp_vaddr, int size_B);
void *mem_alloc_maddr(void);
int   mem_free(void *vp_vaddr);
void  mem_dbg(void);

//-----------------------------------------------------------------------------
// メモリ容量確認

unsigned int mem_total_B(void);
unsigned int mem_total_mfree_B(void);
unsigned int mem_total_vfree_B(void);

#endif

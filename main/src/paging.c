/**
 * ページング
 *
 * @note
 * @par 用語定義
 * - ページ : 4KB 単位のメモリ
 * - 論理アドレス : セグメントアドレスとオフセットアドレスの組
 * - リニアアドレス : セグメント機構により論理アドレスから変換されたアドレス。
 *                    プログラム中では名前に vaddr を使用する。
 * - 物理アドレス : 物理的なメモリのアドレス。
 *                  プログラム中では名前に maddr を使用する。
 * - PD : ページディレクトリ。
 *        ページテーブルのアドレスを取得するためのテーブル。
 * - PDE : PD のエントリ。上位20ビットは１つのページテーブルのアドレスを表す。
 * - PT : ページテーブル。
 *        ページのアドレスを取得するためのテーブル。
 * - PTE : PT のエントリ。上位20ビットは１つのページのアドレスを表す。
 *
 * @par ページングとは
 * ページ単位を管理する仕組み。
 * 仮想記憶などで使われる。
 *
 * @par 論理アドレスが物理アドレスに変換されるまでの流れ
 * -# セグメント機構によって論理アドレスからリニアアドレスに変換される。
 * -# ページング機構によってリニアアドレスから物理アドレスに変換される。
 *
 * @par OnSen OS でのセグメント
 * OnSen OS では 「論理アドレス = リニアアドレス」 になるようにセグメントを
 * 設定している。これは Linux と同じ。ページングは有効・無効を設定できるが、
 * セグメントはオフにできない。だから、最低限の設定をしている(gdt.c 参照)。
 *
 * @par リニアアドレス（32ビット）の構成
 * - PD インデックス（上位 10 ビット）
 * - PT インデックス（10 ビット）
 * - ページ内オフセット（下位 12 ビット）
 *
 * @par リニアアドレスを物理アドレスに変換する仕組み
 * -# CR3 レジスタから PD アドレスを取得する。
 * -# PD アドレス + (PD インデックス * 4) = PT アドレス
 * -# PT アドレス + (PT インデックス * 4) = ページアドレス
 * -# ページアドレス + ページ内オフセット = 物理アドレス
 *
 * @par 各タスクでのメモリマップ（アドレスはリニアアドレス）
 * - 0x00000000 - 0xBFFFFFFF : ユーザ空間
 * - 0xC0000000 - 0xFFFFFFFF : カーネル空間（特権レベル0）
 *
 * @file paging.c
 * @author Ivan Ivanovich Ivanov
 */


//=============================================================================
// 公開ヘッダ

#ifndef HEADER_PAGING
#define HEADER_PAGING


//-----------------------------------------------------------------------------
// ページング

#define PAGE_SIZE_B (4096)  ///< ページサイズ

#define IS_4KB_ALIGN(byte) (((byte) & 0xFFF) == 0)  ///< 4KB 境界であるか確認
#define CEIL_4KB(byte)  (((byte) + 0xFFF) & ~0xFFF) ///< 4KB 単位で切り上げ
#define FLOOR_4KB(byte) ((byte) & ~0xFFF)           ///< 4KB 単位で切り捨て

#define BYTE_TO_PAGE(byte) ((CEIL_4KB(byte)) >> 12)


void paging_init(void);
void paging_map(void *vp_vaddr, void *vp_maddr);
void *paging_get_maddr(void *vp_vaddr);
unsigned long *get_os_pd(void);

void paging_dbg(void);

#endif


//=============================================================================
// 非公開ヘッダ

#include "asmfunc.h"
#include "debug.h"
#include "graphic.h"
#include "memory.h"
#include "str.h"


//-----------------------------------------------------------------------------
// ページング

#define NUM_PDE     (1024)  // １つの PD 内の PDE の数
#define NUM_PTE     (1024)  // １つの PT 内の PTE の数

#define PTE_PRESENT (0x01)  // 1ならページがメモリ上に存在する
#define PTE_RW      (0x02)  // 0なら特権レベル3では書き込めない
#define PTE_US      (0x04)  // 0なら特権レベル3ではアクセスできない
#define PTE_ACCESS  (0x20)  // このエントリのページをアクセスするとCPUが1にする
#define PTE_DIRTY   (0x40)  // このエントリのページに書き込むとCPUが1にする
#define PTE_4MB     (0x80)  // 4MBページ


#define VADDR_TO_PD_INDEX(vaddr)  (((unsigned long) vaddr) >> 22)
#define VADDR_TO_PT_INDEX(vaddr)  ((((unsigned long) vaddr) >> 12) & 0x3FF)

#define MAKE_PTE(maddr, flg)  (((unsigned long) (maddr) & ~0xFFF) | (flg))

typedef unsigned long PDE;
typedef unsigned long PTE;


static PDE *get_pt(int i_pd);
static PTE *get_pte(void *vp_vaddr);
static void map_page(void *vp_vaddr, void *vp_maddr, int flg);


static PDE *l_pd = (PDE *) VADDR_PD_SELF;
static PDE *l_os_pd = (PDE *) MADDR_OS_PDT;


//=============================================================================
// 関数

void paging_init(void)
{
    // 仮につくっておいた先頭のページングを無効にする
    l_pd[0] = 0;

    flush_tlb();
}


void paging_map(void *vp_vaddr, void *vp_maddr)
{
    int flg = PTE_RW | PTE_US | PTE_PRESENT;
    map_page(vp_vaddr, vp_maddr, flg);
}


void *paging_get_maddr(void *vp_vaddr)
{
    int i_pd = VADDR_TO_PD_INDEX(vp_vaddr);
    int i_pt = VADDR_TO_PT_INDEX(vp_vaddr);
    PTE *pt = (PTE *) (0xFFC00000 | (i_pd << 12));

    return (void *) ((pt[i_pt] & ~0xFFF) + ((unsigned long) vp_vaddr & 0xFFF));
}


PDE *get_os_pd(void)
{
    return l_os_pd;
}


static PDE *get_pt(int i_pd)
{
    PDE *pt = (PDE *) (l_pd[i_pd] & ~0xFFF);

    if (pt == 0) {
        return 0;
    }

    return (PTE *) (0xFFC00000 | (i_pd << 12));
}


static PTE *get_pte(void *vp_vaddr)
{
    int i_pd = VADDR_TO_PD_INDEX(vp_vaddr);
    PTE *pt = get_pt(i_pd);

    if (pt == 0) {
        return 0;
    }

    int i_pt = VADDR_TO_PT_INDEX(vp_vaddr);

    return &pt[i_pt];
}


/**
 * ページの物理アドレスをリニアアドレスに対応づける。
 * PT がなければ自動でつくる
 */
static void map_page(void *vp_vaddr, void *vp_maddr, int flg)
{
    PTE *pte = get_pte(vp_vaddr);

    if (pte == 0) {
        // PT がなかったのでつくる

        PTE *pt_maddr = (PTE *) mem_alloc_maddr();

        int i_pd = VADDR_TO_PD_INDEX(vp_vaddr);
        l_pd[i_pd] = MAKE_PTE(pt_maddr, flg | PTE_PRESENT);

        PTE *pt_vaddr = (PTE *) (0xFFC00000 | (i_pd << 12));

        memset(pt_vaddr, 0, PAGE_SIZE_B);

        int i_pt  = VADDR_TO_PT_INDEX(vp_vaddr);
        pte = &pt_vaddr[i_pt];
    }

    *pte = MAKE_PTE(vp_maddr, flg | PTE_PRESENT);
}


void paging_dbg(void)
{
    DBG_STR("DEBUG PAGING");

    for (int i_pd = 0; i_pd < NUM_PDE; i_pd++) {
        if (l_pd[i_pd] & PTE_4MB) {
            dbg_int(i_pd);
            dbg_str(": 4MB Page = 0x");
            dbg_intx(i_pd * (4 * 1024 * 1024));
            dbg_newline();
            continue;
        }

        PTE *pt = get_pt(i_pd);

        if (pt == 0) {
            continue;
        }

        dbg_int(i_pd);
        dbg_str(": ");
        dbg_addr(pt);
        dbg_strln(":");

        int num_pages = 0;

        pt = (PTE *) (0xFFC00000 | (i_pd << 12));

        dbg_str("    ");
        for (int i_pt = 0; i_pt < NUM_PTE; i_pt++) {
            PTE pte = pt[i_pt];

            if (pte == 0) {
                continue;
            }

            if (num_pages < 3) {
                if (num_pages != 0) {
                    dbg_str(", ");
                }

                dbg_int(i_pt);
                dbg_str(": ");
                dbg_addr(pte);
            }

            num_pages++;
        }

        dbg_str(", page = ");
        dbg_intln(num_pages);
    }

    dbg_newline();
}


/**
 * タスク
 *
 * タスクの優先度機能はない。タイムスライスは設定できる。
 */

//=============================================================================
// 公開ヘッダ

#ifndef HEADER_TASK
#define HEADER_TASK

#include <stdbool.h>
#include "api.h"
#include "file.h"
#include "paging.h"

#define ERROR_PID        (-1)

#define TASK_MAX         (32)  // 最大タスク数

#define TASK_FLG_FREE     (0)   // 割り当てなし
#define TASK_FLG_ALLOC    (1)   // 割り当て済み
#define TASK_FLG_RUNNING  (2)   // 動作中

#define TSS_REG_SIZE (104)  // TSS のレジスタ保存部のサイズ

#define EFLAGS_INT_ENABLE   (0x0202)  // 割り込みが有効
#define EFLAGS_INT_DISABLE  (0x0002)  // 割り込みが無効

#define TASK_NAME_MAX  (16)   // タスク名の最大長 + '\0'

#define DEFAULT_STACK0_SIZE  (8 * 1024)
#define DEFAULT_TIMESLICE_MS  (20)


typedef struct TSS {
    // ---- レジスタ保存部

    short backlink; short f1;
    long esp0; unsigned short ss0; short f2;
    long esp1; unsigned short ss1; short f3;
    long esp2; unsigned short ss2; short f4;
    unsigned long cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    unsigned short es, f5, cs, f6, ss, f7, ds, f8, fs, f9, gs, f10;
    unsigned short ldt, f11, t, iobase;


    // ---- OS 用タスク管理情報部

    int pid;   // プロセス ID
    int ppid;  // 親 PID

    unsigned int flags;

    /* ページディレクトリ。CR3に入っているのは
     * 物理アドレスなのでアクセスできない。こっちは論理アドレス */
    PDE *pd;

    char name[TASK_NAME_MAX];
    int timeslice_ms;

    // メモリ
    unsigned long code;
    unsigned long data;
    unsigned long stack;
    // OSタスクなら普通のスタック。アプリならOS権限時のスタック
    unsigned long stack0;

    // ファイルテーブル
    FTE *file_tbl;

    bool is_os_task;  // TODO: フラグに移す
} __attribute__ ((__packed__)) TSS;


typedef struct TASK_MNG {
    // 記憶領域の確保用。
    // tss のインデックスと PID は同じ
    TSS tss[TASK_MAX];

    int num_running;
    int cur_run;  // 現在実行しているタスクの run でのインデックス
    TSS *run[TASK_MAX];
} TASK_MNG;


void task_init(void);
int  task_new(const char *name);
int  task_free(int pid, int exit_status);
int  task_copy(API_REGISTERS *regs, int flg);
void task_run(int pid);
int  task_run_os(const char *name, void (*main)(void));
void task_switch(int ts_tid);
void task_sleep(int pid);
void task_wakeup(int pid);
const char *task_get_name(int pid);
void task_set_pt(int i_pd, unsigned long pt);

void task_dbg(void);

int is_os_task(int pid);

void set_app_tss(int pid, PDE vaddr_pd, void (*f)(void), unsigned long esp, unsigned long esp0);
TSS *pid2tss(int pid);

extern TSS *g_cur;
extern int g_pid;

extern int g_root_pid;
extern int g_idle_pid;

#endif


//=============================================================================
// 非公開ヘッダ

#include <stdbool.h>

#include "asmapi.h"
#include "asmfunc.h"
#include "console.h"
#include "debug.h"
#include "gdt.h"
#include "graphic.h"
#include "memory.h"
#include "msg_q.h"
#include "str.h"
#include "timer.h"


static int pid2tss_sel(int pid);


extern int timer_ts_tid(void);

static void idle_main(void);

static void init_tss_seg(void);
static void set_os_tss(int pid, void (*f)(void), unsigned long esp);
static TSS *set_tss(int pid, int cs, int ds, PDE cr3, PDE pd,
        void (*f)(void), unsigned long eflags, unsigned long esp,
        int ss, unsigned long esp0, int ss0);


TSS *g_cur;
int g_pid;

int g_root_pid;
int g_idle_pid;

static TASK_MNG l_mng;


//=============================================================================
// 関数

static int create_root_task(void);

void task_init(void)
{
    // ---- タスクの初期化

    l_mng.num_running = 0;
    l_mng.cur_run = 0;

    for (int pid = 0; pid < TASK_MAX; pid++) {
        TSS *t = &l_mng.tss[pid];

        t->pid = pid;
        t->flags = TASK_FLG_FREE;
        t->ppid = 0;
        memset(t->name, 0, TASK_NAME_MAX);
        t->file_tbl = create_file_tbl();

        // TODO
        t->file_tbl[0].flags = FILE_FLG_USED;
        t->file_tbl[0].file = f_console;
        t->file_tbl[1].flags = FILE_FLG_USED;
        t->file_tbl[1].file = f_console;
        t->file_tbl[2].flags = FILE_FLG_USED;
        t->file_tbl[2].file = f_console;
    }


    // ---- タスクを作成

    init_tss_seg();

    g_root_pid = create_root_task();
    g_idle_pid = task_run_os("idle", idle_main);

    ltr(pid2tss_sel(g_root_pid));

    // ---- タスク切り替え用タイマをスタート

    int ts_tid = timer_ts_tid();
    timer_start(ts_tid, DEFAULT_TIMESLICE_MS);
}


static int create_root_task(void)
{
    int pid = task_new("root");

    // 最初のtssはタスクスイッチした時に設定される？
    set_os_tss(pid, /* eip = */ 0, /* esp = */ 0);

    task_run(pid);

    return pid;
}


int task_new(const char *name)
{
    for (int pid = 0; pid < TASK_MAX; pid++) {
        TSS *t = &l_mng.tss[pid];

        if (t->flags == TASK_FLG_FREE) { // 未割り当て領域を発見
            msg_q_init(pid);

            memcpy(t->name, name, TASK_NAME_MAX - 1);
            t->flags = TASK_FLG_ALLOC;
            t->ppid = g_pid;

            return pid;
        }
    }

    // もう全部使用中
    DBGF("could not allocate new task.");
    return ERROR_PID;
}


int task_free(int pid, int exit_status)
{
    if (is_os_task(pid)) {
        return -1;
    }

    TSS *t = pid2tss(pid);

    if (t == 0 || t->flags == TASK_FLG_FREE) {
        return -2;
    }

    task_sleep(pid);

    app_area_copy(t->pd);

    mem_free_user((void *) t->code);
    mem_free_user((void *) t->data);
    mem_free_user((void *) t->stack);
    mem_free_user((void *) t->stack0);

    for (int i = 0; i < BASE_PD_I; i++) {
        if (t->pd[i]) {
            mem_free_maddr((void *) t->pd[i]);
        }
    }

    free_task_timer(pid);
    free_task_surface(pid);
    free_file_tbl(t->file_tbl);

    mem_free(t->pd);

    app_area_clear();

    t->flags = TASK_FLG_FREE;

    // 親タスクにメッセージで終了を通知
    TSS *parent = pid2tss(t->ppid);
    if (parent != 0) {
        MSG msg;
        msg.message = MSG_NOTIFY_CHILD_EXIT;
        msg.u_param = pid;
        msg.l_param = exit_status;

        msg_q_put(t->ppid, &msg);
    }

    return 0;
}


/* 今実行されているタスクの複製を作る */
int task_copy(API_REGISTERS *regs, int flg)
{
    int pid = task_new(g_cur->name);

    if (pid == ERROR_PID) {
        dbgf("could not copy\n");
        return ERROR_PID;
    }

    TSS *tss = &l_mng.tss[pid];

    memcpy(tss, g_cur, sizeof(TSS));

    tss->flags = TASK_FLG_ALLOC;
    tss->pid   = pid;
    tss->ppid  = g_cur->pid;

    if ( ! g_cur->is_os_task) {
        // ---- スタック領域をコピー

        int size = VADDR_USER_ESP - g_cur->stack;
        unsigned long temp_stack = (unsigned long) mem_alloc(size);
        memcpy((void *) temp_stack, (void *) g_cur->stack, size);

        PDE *src_pd = copy_pd();
        paging_clear_pd_range(g_cur->stack, VADDR_USER_ESP);

        unsigned long stack = (unsigned long) mem_alloc_user_page(g_cur->stack, size, PTE_RW);
        memcpy((void *) stack, (void *) temp_stack, size);

        tss->stack = stack;

        mem_free(temp_stack);

        // ----

        tss->cr3 = load_cr3();
        tss->cs = KERNEL_CS;
        tss->ds = KERNEL_DS;
        tss->es = KERNEL_DS;
        tss->ss = KERNEL_DS;

        store_cr3((unsigned long) paging_get_maddr(src_pd));
    }

    unsigned long stack0 = (unsigned long) mem_alloc(DEFAULT_STACK0_SIZE);
    unsigned long esp0 = stack0 + DEFAULT_STACK0_SIZE;
    tss->stack0 = stack0;
    tss->esp0 = esp0;

    API_REGISTERS *new_regs  = (API_REGISTERS *) (esp0 - sizeof(API_REGISTERS));
    *new_regs = *regs;

    new_regs->eax = 0;  // 子の戻り値は0

    tss->eip = (unsigned long) new_task_start;
    tss->esp = (unsigned long) new_regs;

    task_run(pid);

    return pid;
}


void task_run(int pid)
{
    TSS *t = pid2tss(pid);

    if (t == 0 || t->flags == TASK_FLG_RUNNING) {
        return;
    }

    t->flags = TASK_FLG_RUNNING;
    t->timeslice_ms = DEFAULT_TIMESLICE_MS;
    l_mng.run[l_mng.num_running] = t;
    l_mng.num_running++;
}


int task_run_os(const char *name, void (*main)(void))
{
    int pid = task_new(name);

    unsigned long stack0 = (unsigned long) mem_alloc(DEFAULT_STACK0_SIZE);
    unsigned long esp0 = stack0 + DEFAULT_STACK0_SIZE;

    set_os_tss(pid, main, esp0);

    TSS *tss = &l_mng.tss[pid];
    tss->stack0 = stack0;

    task_run(pid);

    return pid;
}


void task_switch(int ts_tid)
{
    if (l_mng.num_running <= 1) {
        // idle プロセスのみ
        timer_start(ts_tid, DEFAULT_TIMESLICE_MS);
    } else {
        l_mng.cur_run++;
        if (l_mng.cur_run >= l_mng.num_running) {
            l_mng.cur_run = 0;
        }

        g_cur = l_mng.run[l_mng.cur_run];
        g_pid = g_cur->pid;

        timer_start(ts_tid, g_cur->timeslice_ms);

        far_jmp(pid2tss_sel(g_pid), 0);
    }
}


// 実行中タスクリスト(run)からタスクをはずす
void task_sleep(int pid)
{
    TSS *t = pid2tss(pid);

    if (t == 0) {
        return;
    }

    if (t->flags != TASK_FLG_RUNNING) {
        return;
    }

    char ts = 0;  // タスクスイッチフラグ
    if (t == g_cur) {
        // 現在実行中のタスクをスリープさせるので、あとでタスクスイッチする
        ts = 1;
    }

    int i_rt;  // l_mng.run での t のインデックス
    for (i_rt = 0; i_rt < l_mng.num_running; i_rt++) {
        if (l_mng.run[i_rt] == t) {
            break;
        }
    }

    if (i_rt >= l_mng.num_running) {
        // ここに来たらOSのバグ
        DBGF("[task_sleep] FOUND BUG!!");
        i_rt = l_mng.num_running - 1;
    }

    // cur_run をずらす
    if (i_rt < l_mng.cur_run) {
        l_mng.cur_run--;
    }

    l_mng.num_running--;

    // run をずらす
    for ( ; i_rt < l_mng.num_running; i_rt++) {
        l_mng.run[i_rt] = l_mng.run[i_rt + 1];
    }

    t->flags = TASK_FLG_ALLOC;

    if (ts == 1) {
        // **** タスクスイッチする

        if (l_mng.cur_run >= l_mng.num_running) {
            l_mng.cur_run = 0;
        }

        g_cur = l_mng.run[l_mng.cur_run];
        g_pid = g_cur->pid;

        far_jmp(pid2tss_sel(g_pid), 0);
    }
}


// タスクが寝てたら起こす
void task_wakeup(int pid)
{
    TSS *t = pid2tss(pid);

    if (t == 0) {
        return;
    }

    if (t->flags != TASK_FLG_ALLOC) {
        return;
    }

    task_run(pid);
}


const char *task_get_name(int pid)
{
    TSS *t = pid2tss(pid);

    if (t == 0) {
        return 0;
    }

    if (t->flags == TASK_FLG_FREE) {
        return 0;
    }

    return t->name;
}


void task_set_pt(int i_pd, PDE pt)
{
    for (int pid = 0; pid < TASK_MAX; pid++) {
        TSS *t = &l_mng.tss[pid];

        if (t->flags != TASK_FLG_FREE && t->pd != 0) {
            t->pd[i_pd] = pt;
        }
    }
}


void task_dbg(void)
{
    dbgf("\nnum running : %d\n", l_mng.num_running);

    for (int pid = 0; pid < TASK_MAX; pid++) {
        TSS *t = &l_mng.tss[pid];

        if (t->flags != TASK_FLG_FREE) {
            dbgf("%d %s, pd : %p, cs : %X, ds : %X, ss : %X\n",
                    pid, t->name, t->pd, t->cs, t->ds, t->ss);
        }
    }

    dbgf("\n");
}


int is_os_task(int pid)
{
    TSS *t = pid2tss(pid);

    if (t)
        return t->is_os_task;
    else
        return 0;
}


static void idle_main(void)
{
    for (;;) {
        hlt();
    }
}


static void init_tss_seg(void)
{
    for (int pid = 0; pid < TASK_MAX; pid++) {
        unsigned long tss = (unsigned long) &l_mng.tss[pid];

        set_seg_desc(SEG_TSS + pid, tss, TSS_REG_SIZE,
                SEG_TYPE_TSS, 0, /* dpl = */ 0);
    }
}


static void set_os_tss(int pid, void (*f)(void), unsigned long esp)
{
    TSS *tss = set_tss(pid, KERNEL_CS, KERNEL_DS, MADDR_OS_PDT, VADDR_OS_PDT, f,
            EFLAGS_INT_ENABLE, esp, KERNEL_DS, 0, 0);

    tss->is_os_task = true;
}


void set_app_tss(int pid, PDE vaddr_pd, void (*f)(void), unsigned long esp, unsigned long esp0)
{
    PDE maddr_pd = (PDE) paging_get_maddr((void *) vaddr_pd);

    // | 3 は要求者特権レベルを3にするため
    TSS *tss = set_tss(pid, USER_CS | 3, USER_DS | 3, maddr_pd, vaddr_pd, f,
            EFLAGS_INT_ENABLE, esp, USER_DS | 3, esp0, KERNEL_DS);

    tss->is_os_task = false;
}



static TSS *set_tss(int pid, int cs, int ds, PDE cr3, PDE pd,
        void (*f)(void), unsigned long eflags, unsigned long esp,
        int ss, unsigned long esp0, int ss0)
{
    if (pid < 0 || TASK_MAX <= pid) {
        return 0;
    }

    TSS *tss = &l_mng.tss[pid];
    memset(tss, 0, TSS_REG_SIZE);

    tss->cr3 = cr3;
    tss->pd = (PDE *) pd;

    // タスク実行開始時のレジスタ内容を設定する
    tss->cs = cs;
    tss->eip = (unsigned long) f;
    tss->eflags = eflags;
    tss->esp = esp;
    tss->ds = tss->es = tss->fs = tss->gs = ds;
    tss->ss = ss;

    // 特権レベル0に移行したときのスタック領域を設定する
    tss->esp0 = esp0;
    tss->ss0 = ss0;

    return tss;
}


TSS *pid2tss(int pid)
{
    if (pid < 0 || TASK_MAX <= pid) {
        return 0;
    }

    return &l_mng.tss[pid];
}


static int pid2tss_sel(int pid)
{
    return (SEG_TSS + pid) << 3;
}

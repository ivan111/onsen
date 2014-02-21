/**
 * タスク
 *
 * @note
 * タスクの優先度機能はない。タイムスライスは設定できる。
 *
 * @file task.c
 * @author Ivan Ivanovich Ivanov
 */

//=============================================================================
// 公開ヘッダ

#ifndef HEADER_TASK
#define HEADER_TASK

#include <stdbool.h>

#define ERROR_PID   -1

#define TASK_MAX    32  ///< 最大タスク数


extern int g_root_pid;
extern int g_idle_pid;
extern int g_dbg_pid;
extern int g_con_pid;
extern int g_world_pid;


void task_init(void);
int  task_new(char *name);
int  task_free(int pid, int exit_status);
void task_run(int pid, int timeslice_ms);
int  task_run_app(void *p, unsigned int size, const char *name);
void task_switch(int ts_tid);
void task_sleep(int pid);
void task_wakeup(int pid);
int  get_pid(void);
const char *task_get_name(int pid);

void task_dbg(void);


inline __attribute__ ((always_inline))
static int is_os_task(int pid)
{
    return (pid == g_root_pid || pid == g_idle_pid || pid == g_con_pid ||
            pid == g_dbg_pid  || pid == g_world_pid);
}


#endif


//=============================================================================
// 非公開ヘッダ

#include <stdbool.h>

#include "asmfunc.h"
#include "console.h"
#include "debug.h"
#include "gdt.h"
#include "graphic.h"
#include "memory.h"
#include "msg_q.h"
#include "paging.h"
#include "str.h"
#include "timer.h"


#define TASK_FLG_FREE     0   ///< 割り当てなし
#define TASK_FLG_ALLOC    1   ///< 割り当て済み
#define TASK_FLG_RUNNING  2   ///< 動作中

#define TSS_REG_SIZE 104  ///< TSS のレジスタ保存部のサイズ

#define EFLAGS_INT_ENABLE   0x0202  ///< 割り込みが有効
#define EFLAGS_INT_DISABLE  0x0002  ///< 割り込みが無効

#define TASK_NAME_MAX  16   ///< タスク名の最大長 + '\0'

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

    int pid;  ///< プロセス ID
    int flags;

    int ppid;  ///< 親 PID

    char name[TASK_NAME_MAX];
    int timeslice_ms;

    // メモリ解放用
    void *code;
    void *data;
    void *stack0;
} __attribute__ ((__packed__)) TSS;


typedef struct TASK_MNG {
    /// 記憶領域の確保用。
    /// tss のインデックスと PID は同じ
    TSS tss[TASK_MAX];

    int num_running;
    int cur_run;  ///< 現在実行しているタスクの run でのインデックス
    TSS *run[TASK_MAX];
} TASK_MNG;


int g_root_pid;
int g_idle_pid;
int g_dbg_pid;
int g_con_pid;
int g_world_pid;


static TASK_MNG l_mng;


inline __attribute__ ((always_inline))
static TSS *pid2tss(int pid)
{
    if (pid < 0 || TASK_MAX <= pid) {
        return 0;
    }

    return &l_mng.tss[pid];
}


inline __attribute__ ((always_inline))
int pid2tss_sel(int pid)
{
    return (SEG_TSS + pid) << 3;
}


extern int timer_ts_tid(void);

static int run_os_task(char *name, unsigned long pd,
        void (*main)(void), int timeslice_ms, bool create_esp);
static void idle_main(void);

static void init_tss_seg(void);
static void set_os_tss(int pid, unsigned long pd, void (*f)(void), void *esp);
static void set_app_tss(int pid, unsigned long pd, void (*f)(void), void *esp, void *esp0);
static void set_tss(int pid, int cs, int ds, unsigned long cr3,
        void (*f)(void), unsigned long eflags, void *esp,
        int ss, void *esp0, int ss0);

//=============================================================================
// 公開関数

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
    }


    // ---- タスクを作成

    unsigned long pd = (unsigned long) get_os_pd();

    init_tss_seg();

    g_root_pid = run_os_task("root",    pd, 0,            20, false);
    g_idle_pid = run_os_task("idle",    pd, idle_main,    10, true);
    g_dbg_pid  = run_os_task("debug",   pd, debug_main,   20, true);
    g_con_pid  = run_os_task("console", pd, console_main, 20, true);

    ltr(pid2tss_sel(g_root_pid));


    // ---- タスク切り替え用タイマをスタート

    int ts_tid = timer_ts_tid();
    timer_start(ts_tid, 20);
}


int task_run_app(void *p, unsigned int size, const char *name)
{
    unsigned long p_addr = (unsigned long) p;
    //char *p_code = (char *) p;

    int stack_and_data_size  = *((int *) (p_addr + 0x0000));
    int esp                  = *((int *) (p_addr + 0x000C));
    int data_size            = *((int *) (p_addr + 0x0010));
    int data_addr            = *((int *) (p_addr + 0x0014));

    char app_name[9];
    memcpy(app_name, name, 8);
    app_name[8] = 0;

    int pid = task_new(app_name);

    //memcpy(0x350000, p_code, 0x400);
    char *p_code = 0;
    mem_alloc_user(p_code, size);
    memcpy(p_code, p, size);
    //char *p_data = (char *) mem_alloc(stack_and_data_size); FIXME
    //char *p_data = (char *) 0x350400;
    char *p_data = (char *) mem_alloc_user(esp, stack_and_data_size);

    dbg_str("\nsize: 0x");
    dbg_intx(size);
    dbg_str("\nstack_and_data_size: 0x");
    dbg_intx(stack_and_data_size);
    dbg_str("\nesp: 0x");
    dbg_intx(esp);
    dbg_str("\ndata_size: 0x");
    dbg_intx(data_size);
    dbg_str("\ndata_addr: 0x");
    dbg_intx(data_addr);
    dbg_str("\np_data: 0x");
    dbg_intx(p_data);
    dbg_str("\np_code: 0x");
    dbg_intx(p_code);
    dbg_newline();
    //memcpy(p_data + esp, p_code + data_addr, data_size); FIXME
    //memcpy(esp, p_code + data_addr, data_size);
    memcpy(p_code + esp, p + data_addr, data_size);
    dbg_strln((char *) (esp));
    dbg_intx(p_code + data_addr);
    dbg_newline();

    //p_code = (char *) 0x350000;

    /*
    unsigned char *stack0 = mem_alloc(64 * 1024);
    unsigned char *esp0 = stack0 + (64 * 1023);

    unsigned long pd = (unsigned long) get_os_pd();  // FIXME
    //set_app_tss(pid, pd, (void *) 0x1B, (unsigned char *) esp, esp0); FIXME
    set_app_tss(pid, pd, (void *) 0x35001B, (unsigned char *) esp, esp0);

    TSS *t = pid2tss(pid);
    t->code   = p_code;
    t->data   = p_data;
    t->stack0 = stack0;

    task_run(pid, 20);
    */

    return pid;
}


int task_new(char *name)
{
    for (int pid = 0; pid < TASK_MAX; pid++) {
        TSS *t = &l_mng.tss[pid];

        if (t->flags == TASK_FLG_FREE) { // 未割り当て領域を発見
            msg_q_init(pid);

            memcpy(t->name, name, TASK_NAME_MAX - 1);
            t->flags = TASK_FLG_ALLOC;
            t->ppid = get_pid();

            return pid;
        }
    }

    // もう全部使用中
    DBG_STR("could not allocate new task.");
    return ERROR_PID;
}


int task_free(int pid, int exit_status)
{
    if (is_os_task(pid)) {
        return -1;
    }

    TSS *t = pid2tss(pid);

    if (t == 0) {
        return -2;
    }

    task_sleep(pid);

    mem_free(t->code);
    mem_free(t->data);
    mem_free(t->stack0);

    timer_task_free(pid);

    int screen_pid = get_screen_pid();

    if (pid == screen_pid) {
        switch_screen();
    }

    surface_task_free(pid);

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


void task_run(int pid, int timeslice_ms)
{
    TSS *t = pid2tss(pid);

    if (t == 0 || t->flags == TASK_FLG_RUNNING) {
        return;
    }

    t->flags = TASK_FLG_RUNNING;
    t->timeslice_ms = timeslice_ms;
    l_mng.run[l_mng.num_running] = t;
    l_mng.num_running++;
}


void task_switch(int ts_tid)
{
    if (l_mng.num_running <= 1) {
        // idle プロセスのみ
        timer_start(ts_tid, 20);
    } else {
        l_mng.cur_run++;
        if (l_mng.cur_run >= l_mng.num_running) {
            l_mng.cur_run = 0;
        }

        TSS *t = l_mng.run[l_mng.cur_run];

        timer_start(ts_tid, t->timeslice_ms);

        far_jmp(pid2tss_sel(t->pid), 0);
    }
}


/// 実行中タスクリスト(run)からタスクをはずす
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
    if (t == l_mng.run[l_mng.cur_run]) {
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
        DBG_STR("[task_sleep] FOUND BUG!!");
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

        TSS *t = l_mng.run[l_mng.cur_run];
        far_jmp(pid2tss_sel(t->pid), 0);
    }
}


/// タスクが寝てたら起こす
void task_wakeup(int pid)
{
    TSS *t = pid2tss(pid);

    if (t == 0) {
        return;
    }

    if (t->flags != TASK_FLG_ALLOC) {
        return;
    }

    task_run(pid, t->timeslice_ms);
}


int get_pid(void)
{
    TSS *t = l_mng.run[l_mng.cur_run];

    /* task_init が呼ばれる前はpid=0とする */
    if (t == 0) {
        return 0;
    }

    return t->pid;
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


void task_dbg(void)
{
    dbg_int(l_mng.num_running);
    dbg_newline();

    for (int pid = 0; pid < TASK_MAX; pid++) {
        TSS *t = &l_mng.tss[pid];

        if (t->flags != TASK_FLG_FREE) {
            dbg_int(pid);
            dbg_str(" ");
            dbg_str(t->name);
            dbg_str(" cs : ");
            dbg_seg_reg(t->cs);
            dbg_str(", ds : ");
            dbg_seg_reg(t->ds);
            dbg_str(", ss : ");
            dbg_seg_reg(t->ss);
            dbg_newline();
        }
    }
    dbg_newline();
}


//=============================================================================
// 非公開関数

static int run_os_task(char *name, unsigned long pd,
        void (*main)(void), int timeslice_ms, bool create_esp)
{
    int pid = task_new(name);

    unsigned char *esp = 0;

    if (create_esp) {
        esp = mem_alloc(64 * 1024);
        esp += 64 * 1024;
    }

    set_os_tss(pid, pd, main, esp);

    task_run(pid, timeslice_ms);

    return pid;
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


static void set_os_tss(int pid, unsigned long pd, void (*f)(void), void *esp)
{
    set_tss(pid, KERNEL_CS, KERNEL_DS, pd, f, EFLAGS_INT_ENABLE,
            esp, KERNEL_DS, 0, 0);
}


static void set_app_tss(int pid, unsigned long pd, void (*f)(void), void *esp, void *esp0)
{
    // | 3 は要求者特権レベルを3にするため
    set_tss(pid, USER_CS | 3, USER_DS, pd, f, EFLAGS_INT_ENABLE,
            esp, USER_DS | 3, esp0, KERNEL_DS);
}



static void set_tss(int pid, int cs, int ds, unsigned long cr3,
        void (*f)(void), unsigned long eflags, void *esp,
        int ss, void *esp0, int ss0)
{
    if (pid < 0 || TASK_MAX <= pid) {
        return;
    }

    TSS *tss = &l_mng.tss[pid];
    memset(tss, 0, TSS_REG_SIZE);

    tss->cr3 = cr3;

    // タスク実行開始時のレジスタ内容を設定する
    tss->cs = cs;
    tss->eip = (unsigned long) f;
    tss->eflags = eflags;
    tss->esp = (unsigned long) esp;
    tss->ds = tss->es = tss->fs = tss->gs = ds;
    tss->ss = ss;

    // 特権レベル0に移行したときのスタック領域を設定する
    tss->esp0 = (unsigned long) esp0;
    tss->ss0 = ss0;
}



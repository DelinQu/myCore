#ifndef __KERN_PROCESS_PROC_H__
#define __KERN_PROCESS_PROC_H__

#include <defs.h>
#include <list.h>
#include <trap.h>
#include <memlayout.h>
#include <skew_heap.h>


// process's state in his life cycle
enum proc_state {
    PROC_UNINIT = 0,  // uninitialized
    PROC_SLEEPING,    // sleeping
    PROC_RUNNABLE,    // runnable(maybe running)
    PROC_ZOMBIE,      // almost dead, and wait parent proc to reclaim his resource
};

// Saved registers for kernel context switches.
// Don't need to save all the %fs etc. segment registers,
// because they are constant across kernel contexts.
// Save all the regular registers so we don't need to care
// which are caller save, but not the return register %eax.
// (Not saving %eax just simplifies the switching code.)
// The layout of context must match code in switch.S.
struct context {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};

#define PROC_NAME_LEN               15
#define MAX_PROCESS                 4096
#define MAX_PID                     (MAX_PROCESS * 2)

extern list_entry_t proc_list;

struct proc_struct {                        //进程控制块
    enum proc_state state;                  //进程状态
    int pid;                                //进程ID
    int runs;                               //运行时间
    uintptr_t kstack;                       //内核栈位置
    volatile bool need_resched;             //是否需要调度，只对当前进程有效
    struct proc_struct *parent;             //父进程
    struct mm_struct *mm;                   //进程的虚拟内存
    struct context context;                 //进程上下文
    struct trapframe *tf;                   //当前中断帧的指针
    uintptr_t cr3;                          //当前页表地址
    uint32_t flags;                         //进程
    char name[PROC_NAME_LEN + 1];           //进程名字
    list_entry_t list_link;                 //进程链表       
    list_entry_t hash_link;                 //进程哈希表
    int exit_code;                          //退出码(发送到父进程)
    uint32_t wait_state;                    //等待状态
    struct proc_struct *cptr, *yptr, *optr; //进程间的一些关系
    struct run_queue *rq;                   //运行队列中包含进程
    list_entry_t run_link;                  //该进程的调度链表结构，该结构内部的连接组成了 运行队列 列表
    int time_slice;                         //该进程剩余的时间片，只对当前进程有效
    skew_heap_entry_t lab6_run_pool;        //该进程在优先队列中的节点，仅在 LAB6 使用
    uint32_t lab6_stride;                   //该进程的调度步进值，仅在 LAB6 使用
    uint32_t lab6_priority;                 //该进程的调度优先级，仅在 LAB6 使用
};

#define PF_EXITING                  0x00000001      // getting shutdown

#define WT_CHILD                    (0x00000001 | WT_INTERRUPTED)
#define WT_INTERRUPTED               0x80000000                    // the wait state could be interrupted


#define le2proc(le, member)         \
    to_struct((le), struct proc_struct, member)

extern struct proc_struct *idleproc, *initproc, *current;

void proc_init(void);
void proc_run(struct proc_struct *proc);
int kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags);

char *set_proc_name(struct proc_struct *proc, const char *name);
char *get_proc_name(struct proc_struct *proc);
void cpu_idle(void) __attribute__((noreturn));

struct proc_struct *find_proc(int pid);
int do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf);
int do_exit(int error_code);
int do_yield(void);
int do_execve(const char *name, size_t len, unsigned char *binary, size_t size);
int do_wait(int pid, int *code_store);
int do_kill(int pid);
//FOR LAB6, set the process's priority (bigger value will get more CPU time) 
void lab6_set_priority(uint32_t priority);

#endif /* !__KERN_PROCESS_PROC_H__ */


typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NPROC       17
#define SSIZE       512
#define NULL        0
#define null        0

/******* PROC status ********/
#define FREE      0    /* PROC status */
#define READY     1 
#define RUNNING   2
#define SLEEP     2
#define ZOMBIE    3
#define BLOCK     4
#define READ_PIPE   4
#define WRITE_PIPE  5
#define INT_CTL     0X20
#define ENABLE      0X20

#define NOFT        20
#define NFD         10
#define PSIZE       10
#define NPIPE       10


typedef struct oft {
    int   mode;
    int   refCount;
    struct pipe *pipe_ptr;
} OFT;

typedef struct proc{
    struct proc *next;
    int    *ksp;               // at offset 2

    int    uss, usp;           // at offsets 4,6
    int    inkmode;            // at offset 8

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];           // name string of PROC

    OFT    *fd[NFD];

    int    kstack[SSIZE];      // per proc stack area
}PROC;

typedef struct pipe {
    char  buf[PSIZE];
    int   head, tail, data, room;
    int   nreader, nwriter;
    int   busy;
}PIPE;

// define bool
typedef enum { false, true } bool;

int procSize = sizeof(PROC);
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList, *zombieList;
int nproc, rflag;                   // number of procs, re-schedule flag

PIPE pipe[NPIPE];
OFT oft[NOFT];

/******* kernel.c ********/
int do_kfork();
PROC *kfork();
int do_tswitch();
int do_exit();
int do_stop();
int do_continue();
int do_sleep();
int do_wakeup();
int reschedule();
int chpriority();
int do_chpriority();
int body();
int geti();


/******* main.c ********/
void enqueue(PROC **queue, PROC *p);
PROC *dequeue(PROC **queue);
PROC *get_proc();
int put_proc(PROC **list, PROC *p);
void printList(char *name, PROC *queue);
int sleep(int event);
int wakeup(int event);
int init();
int scheduler();

/******* wait.c ********/
int ksleep (int event);
int kwakeup (int event);
int removeProc (PROC **queue, PROC *p);
int ready (PROC *p);
int kexit (int exitValue);
int kwait (int *status);
int do_wait();

/******* pipe.c ********/
int show_pipe(PIPE *ptr);
int pfd();
int read_pipe(int ifd, char *buf, int n);
int write_pipe(int ifd, char *buf, int n);
int kpipe(int pd[2]);
int close_pipe(int fd);

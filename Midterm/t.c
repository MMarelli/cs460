#include "type.h"


int body();
char *pname[]={"2000", "2001", "2002", "2003",  "2004", "2005", "2006", "2007", "2008" , 
               "2009", "2010", "2011", "2012",  "2013", "2014", "2015", "2016" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
#include "pipe.c"

int color;

int init()
{
    PROC *p; int i,j;
    color = 0x0C;
    printf("\nIn 5...4...3...2...1...\n\n");

    for(i = 0; i < NOFT; i++)
    {
      oft[i].mode = 0;
      oft[i].refCount = 0;
      oft[i].pipe_ptr = 0;
    }

    for(i = 0; i < NPIPE; i++)
    {
      pipe[i].buf[0] = 0;
      pipe[i].head = 0;
      pipe[i].tail = 0;
      pipe[i].data = 0;
      pipe[i].room = PSIZE;
      pipe[i].nreader = 0;
      pipe[i].nwriter = 0;
      pipe[i].busy = 0;
    }

    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;
        for (j=0; j < NFD; j++)
          p->fd[j] = 0;
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];
    }

    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("And we're live!\n\n");
} 

int scheduler()
{
    if (running->status == READY)
        enqueue(&readyQueue, running);
     running = dequeue(&readyQueue);
     color = running->pid + 0x0A;
}

int int80h();
int set_vector(u16 segment, u16 handler)
{
     // put_word(word, segment, offset)
     put_word(handler, 0, segment<<2);
     put_word(0x1000,  0,(segment<<2) + 2);
}
            
main()
{
    printf("\n\nWelcome to Wayne's World!\n");
    init();      // initialize and create P0 as running
    set_vector(80, int80h);

    kfork("/bin/u1");     // P0 kfork() P1

    while(1){
      printf("P0 running\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      tswitch();         // P0 switch to run P1
   }
}

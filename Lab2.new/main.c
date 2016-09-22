#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList, *zombieList;
int procSize = sizeof(PROC);
int nproc, rflag;

/*#include "io.c" //possible optional may be in mtxlib*/
#include "queue.c"



int body()
{
	char c;
	printf("\nproc %d starts from body()\n", running->pid);
	while(1)
	{
		printf("******************************************\n");
		printList("freeList ", freeList); /*/show freelist*/
		printList("readyQueue ", readyQueue); /*/show readyqueue*/
		printList("sleepList ", sleepList);
		printList("zombieList", zombieList);
		printf("******************************************\n");
		printf("proc %d running: priority= %d parent= %d enter a char [s|f|t|c|z|a|p|w|q](only [s|f|q|w] some work)", running->pid, running->priority, running->ppid);
		c = getc();
		printf("%c\n", c);
		switch(c)
		{
			case 's' : do_tswitch();	break;
			case 'f' : do_kfork();		break;
			case 'q' : do_exit();		break;
			/*case 't' : do_stop();		break;
			case 'c' : do_continue();	break;
			case 'z' : do_sleep();		break;
			case 'a' : do_wakeup();		break;
			case 'p' : do_chpriority();	break;*/
			case 'w' : do_wait();		break;
			printf("Invalid Command\n");
		}
		printf("\n");
	}
}

void tswitch();

#include "wait.c"
#include "kernel.c"

int init()
{
	PROC *p;
	int i;
	for(i = 0; i<NPROC; i++)
	{
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		p->next = &proc[i+1];
	}

	proc[NPROC-1].next = 0;
	freeList = &proc[0];
	readyQueue = sleepList = zombieList = 0;

	/******Create P0 as running*******/
	p = get_proc(&freeList);

	p->ppid = 0;
	p->status = READY;
	running = p;
}

int scheduler()
{
	if(running->status == READY)
	{
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	rflag = 0;
}

main()
{
	printf("MTX starts in main()\n");
	init();
	kfork();
	while(1)
	{
		while(!readyQueue)
		{
			kfork();
		}
		tswitch();
	}
}
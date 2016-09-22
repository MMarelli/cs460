/******* MTX4.5 kernel t.c file *******/

#define SSIZE 1024					/*/ 2KB stack size per PROC*/
#define NPROC 9						/*/ proc kstack size = 2KB*/

/******* PROC status *******/
#define FREE	0
#define READY	1
#define STOP	2
#define DEAD	3

typedef struct proc{				/*/ process structure*/
	struct proc *next;				/*/ next PROC pointer*/
	int *ksp;						/*/ saved sp when PROC is not running*/
	int pid;						/*/ add pid for identifying the proc*/
	int ppid;						/*/ parent pid;*/
	int status;						/*/ status = FREE | READY | STOPPED | DEAD, etc*/
	int priority;					/*/ scheduling priority*/
	int kstack[SSIZE];				/*/ process kernel mode stack*/
} PROC;								/*/ PROC is a type*/

int procSize = sizeof(PROC);
PROC proc[NPROC], *running, *freeList, *readyQueue;

#include "io.c"

int body ()
{
	char c;
	myprintf("proc %d starts from body()\n", running->pid);
	while(1)
	{
		printList("freeList", freeList);
		printList("readyQueue", readyQueue);
		myprintf("proc %d running: parent=%d\n", running->pid, running->ppid);
		myprintf("enter a char [s | f | w | q] : ");
		c = getc();
		myprintf("%c\n", c);
		switch(c)
		{
			case 'f' : do_kfork();		break;
			case 's' : do_tswitch();	break;
			case 'w' : do_wait();		break;
			case 'q' : do_exit();		break;
		}
	}
}

void do_exit()
{
	running.status = DEAD;
	tswitch();
}
int do_wait()
{
	int i = 0;
	for(i = 0; i < NPROC; i++)
	{
		if(running.pid == proc[i].ppid && running.pid != proc[i].pid)
		{
			break;
		}
	}

	if (i >= NPROC) /* Callser has no children */
	{
		printf("Process %d has no CHILD to wait on", running.pid);
		return -1;
	}

	while(1) /*caller does have children */
	{

	}
}

PROC *kfork()  /*/ create a child process, begin from body()*/
{
	int i;
	PROC *p = get_proc(&freeList);
	if (!p) {
		myprintf("no more PROC, kfork() failed\n");
		return 0;
	}
	p->status = READY;
	p->priority = 1;				/* priority = 1 for all proc except P0*/
	p->ppid = running->pid;			/*/ parent = running
	 initialize new proc's kstack[]  */
	for (i=1; i<10; i++)			/*/ saved CPU registers*/
		p->kstack[SSIZE-i]= 0;		/*/ all 0's*/
	p->kstack[SSIZE-1] = (int)body;	/*/ resume point=address of body()*/
	p->ksp = &p->kstack[SSIZE-9];	/*/ proc saved sp*/
	enqueue(&readyQueue, p);		/*/ enter p into readyQueue by priority*/
	return p;						/*/ return child PROC pointer*/
}

int do_kfork()
{
	PROC *p = kfork();
	if (p == 0)
	{
		myprintf("kfork failed\n");
		return -1;
	}
	myprintf("PROC %d kfork a child %d\n", running ->pid, p->pid);
	return p->pid;
}

int do_tswitch()
{
	tswitch();
}

void enqueue(PROC **quue, PROC *p)
{
	PROC *current, *next;

	/*/ Empty queue*/
	if ((*queue) == 0)
	{
		*queue = p;
		(*queue)->next = 0;
	}

	/*/ new proc goes at head of queue*/
	else if (p->priority > (*queue)->priority)
	{
		p->next = (*queue);
		(*queue) = p;
	}

	/*/ new process inserted somewhere within queue according to priority*/
	else
	{
		current = (*queue);
		next = current->next;

		while (current != 0 && p->priority <= next->priority)
		{
			current = next;
			next = current->next;
		}

		current->next = p;
		p->next = next;
	}
}

PROC *dequeue (PROC **queue)
{
	/*/ find correct process*/
	PROC *p = *queue;

	if (*queue != 0)
		*queue = (*queue)->next;

	return p;
}

PROC *get_proc(PROC **list)			/*/ return a FREE PROC pointer from list*/
{
	if (freeList != 0)
		return dequeue(&freeList);
	return 0;
}

int put_proc (PROC **list, PROC *p)	/*/ enter p into list*/
{
	/*/ Free the process*/
	p->status = FREE;

	/*/ Check list for other items, otherwise list = p*/
	if (freeList == 0)
	{
		freeList = p;
		p->next = 0;
	}
	else
	{
		p->next = freeList->next;
		freeList->next = p;
	}
}


void printList(char *name, PROC *queue)
{
    PROC *p;
    
    /*/ check for empty queue*/
    if (queue == 0)
    {
    	myprintf("%s: ", name);
        myprintf("There are no processes in this queue.");
    }
    /*/ if not empty, print the queue!*/
    else
    {
        p = queue->next;
        myprintf("%s: ", name);
        myprintf("[%d]", queue->pid);
        while (p != 0)
        {
            myprintf(" -> [%d]", p->pid);
            p = p->next;
        }
        myprintf(" -> NULL\n");
    }
}

int init()							/*/ initialize PROC structures*/
{
	PROC *p; int i;

	myprintf("init ....\n");

	for (i=0; i<NPROC; i++) {	 	/*/ initialize all PROCs*/
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		p->next = &proc[i+1];
	}
	proc[NPROC-1].next = 0;
	freeList = &proc[0];
	readyQueue = 0;
	/******* create P0 as running *******/
	p = get_proc(&freeList);
	p->ppid = 0;
	p->status = READY;
	running = p;					/*/ P0 is now running*/
	myprintf("init complete\n");
}
			
int scheduler ()
{
	if (running->status == READY)		/*/ if running is still READY*/
		enqueue(&readyQueue, running);	/*/ enter it into readyQueue*/
	running = dequeue(&readyQueue);		/*/new running*/
}

void main ()
{
	myprintf("MTX starts in main()\n");
	init();								/*/ initialize and create P0 as running*/
	kfork();							/*/ P0 creates child P1*/
	while(1)							/*/ P0 switches if readyQueue not empty*/
	{
		if (readyQueue)
			tswitch();
	}
}
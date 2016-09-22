int do_tswitch()
{
	tswitch();
}

PROC *kfork()  /*/ create a child process, begin from body()*/
{
	int i;
	PROC *p = get_proc(&freeList);
	if (!p) {
		printf("no more PROC, kfork() failed\n");
		return 0;
	}
	p->status = READY;
	p->priority = 1;				/* priority = 1 for all proc except P0*/
	p->ppid = running->pid;			/*/ parent = running
	 initialize new proc's kstack[]  */
	for (i=1; i<12; i++)			/*/ saved CPU registers changed to i < 12 from i < 10*/
		p->kstack[SSIZE-i]= 0;		/*/ all 0's*/
	p->kstack[SSIZE-1] = (int)body;	/*/ resume point=address of body()*/
	p->ksp = &p->kstack[SSIZE-11];	/*/ proc saved sp changed to ssize-9 from ssize -9*/
	p->kstack[SSIZE - 11] = 0x1000;
	p->kstack[SSIZE - 10] = 0x1000;

	enqueue(&readyQueue, p);		/*/ enter p into readyQueue by priority*/
	return p;						/*/ return child PROC pointer*/
}

int do_kfork()
{
	PROC *p = kfork();
	if (p == 0)
	{
		printf("kfork failed\n");
		return -1;
	}
	printf("PROC %d kfork a child %d\n", running ->pid, p->pid);
	return p->pid;
}


int do_exit()
{
	int code;
	printf("Enter exit code:");
	code = getc() - '0';
	printf("Exiting with code: %d\n", code);
	kexit(code);
}

int do_stop()
{
	running->status == STOPPED;
	tswitch();
}

int do_continue()
{
	PROC *p;
	int pid, i;

	printf("Enter pid to continue:");
	pid = getc() - '0';
	if(pid < 0 || pid >= NPROC)
	{
		printf("pid out of range");
		return -1;
	}
	for(i = 0; i < NPROC; i++)
	{
		p = &proc[i];
		if(p->pid == pid)
		{
			if(p->status == SLEEP)
			{
				p->status == READY;
				enqueue(&readyQueue, p);
			}
			break;
		}	
	}
}

int do_sleep()
{
	int event;
	printf("Enter sleep event code:");
	event = getc() - '0';

	ksleep(event);
}

int do_wakeup()
{
	int event;
	printf("Enter wakeup event code:");
	event = getc() - '0';

	wakeup(event);
}

/*/added scheduling functions in MTX 4.5*/
int reschedule()
{
	PROC *p, *tempQ = 0;
	while ((p = dequeue(&readyQueue))) /*/reorder readyQueue*/
	{
		enqueue(&tempQ, p);
	}
	readyQueue = tempQ;
	rflag = 0;  /*/global rechedule flag*/
	if(running->priority < readyQueue->priority)
	{
		rflag = 1;
	}
}

int chpriority(int pid, int pri)
{
	PROC *p;
	int i, ok = 0, reQ = 0;
	if (pid == running->pid)
	{
		running->priority = pri;
		if(pri < readyQueue->priority)
		{
			rflag = 1;
		}
		return 1;
	}
	/*/if not for running, for both READY and SLEEP procs*/
	for(i = 1; i<NPROC; i++)
	{
		p = &proc[i];
		if(p->pid == pid && p->status != FREE)
		{
			p->priority = pri;
			ok = 1;
			if(p->status == READY)  /*/in readyQueue ==>redo readyQueue*/
			{
				reQ = 1;
			}
		}
	}

	if(!ok)
	{
		printf("chpriority failed\n");
		return -1;
	}

	if(reQ)
	{
		reschedule(p);
	}
}

int do_chpriority()
{
	int pid, pri;
	printf("input pid ");
	pid = getc() - '0';
	if(pri<1)
	{
		pri = 1;
		chpriority(pid, pri);
	}
}

int do_wait()
{
	int pid, status;
	pid = kwait(&status);
	if(pid == -1)
	{
		printf("Process has no children to wait on\n");
		return -1;
	}
	printf("pid=%d exits with status=%d\n", pid, status);
	return 1;
}

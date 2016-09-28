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
		printf("******************************************\n");
		printf("proc %d running: priority= %d parent= %d enter a char [s|f|t|c|z|a|p|w|q|u]", running->pid, running->priority, running->ppid);
		c = getc();
		printf("%c\n", c);
		switch(c)
		{
			case 's' : do_tswitch();	break;
			case 'f' : do_kfork();		break;
			case 'q' : do_exit();		break;
			case 't' : do_stop();		break;
			case 'c' : do_continue();	break;
			case 'z' : do_sleep();		break;
			case 'a' : do_wakeup();		break;
			case 'p' : do_chpriority();	break;
			case 'w' : do_wait();		break;
			case 'u' : goUmode();		break;
			printf("Invalid Command\n");
		}
		printf("\n");
	}
}

int do_tswitch()
{
	tswitch();
}

int copyImage(u16 pseg, u16 cseg, u16 size)
{
	u16 i;
	for(i = 0; i < size; i++)
	{
		put_word(get_word(pseg, 2*i), cseg, 2*i);
	}
}
PROC *kfork(char *filename)  /*/ create a child process, begin from body()*/
{
	int i, segment;
	PROC *p = get_proc(&freeList);
	if (!p) {
		printf("no more PROC, kfork() failed\n");
		return -1;
	}
	nproc++;
	p->status = READY;
	p->priority = 1;				/* priority = 1 for all proc except P0*/
	p->ppid = running->pid;			/* parent = running*/
	p->parent = RUNNING;
	/*initialize new proc's kstack[]  */
	for (i=1; i<12; i++)			/* saved CPU registers changed to i < 12 from i < 10*/
		p->kstack[SSIZE-i]= 0;		/* all 0's*/
	p->kstack[SSIZE-1] = (int)body;	/* resume point=address of body()*/
	p->ksp = &(p->kstack[SSIZE-11]);	/* proc saved sp changed to ssize-11 from ssize -9*/
	p->kstack[SSIZE - 11] = 0x1000;
	p->kstack[SSIZE - 10] = 0x1000;

	segment = (p->pid + 1) * 0x1000;
	if(filename)
	{
		load(filename, segment);
	}

	/*initialize user stack*/
	put_word(segment, segment, segment - 12 * 2); /*set uDS*/
	put_word(segment, segment, segment - 11 * 2); /*set uES*/

	for(i = 3; i < 11; i++) /*Set CPU registers in user stack and PC to 0*/
	{
		put_word(0, segment, segment - i * 2);
	}
	put_word(segment, segment, segment - 2 * 2); /*set uCS*/
	put_word(0, segment, segment - 1 * 2); /*set flag to 0*/
	p->uss = segment;
	p->usp = segment-24;

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
	code = geti();
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
	pid = geti();
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
	event = geti();

	ksleep(event);
}

int do_wakeup()
{
	int event;
	printf("Enter wakeup event code:");
	event = geti();

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
	rflag = 0;  /*global rechedule flag*/
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
	pid = geti();
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

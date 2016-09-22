int ksleep(int event)
{
	running->event = event; /*/record event in PROC.event*/
	running->status = SLEEP; /*/change status to sleep*/
	put_proc(&sleepList, running);
	tswitch();
}

int wakeup(int event)
{
	int i;
	PROC *p;
	for(i = 1; i<NPROC; i++)  /*not applicable to P0*/
	{
		p = &proc[i];
		if(p->status == SLEEP && p->event == event)
		{
			p->event = 0;  /*/cancel PROC's event*/
			p->status = READY; /*/make it ready to run again*/
			enqueue(&readyQueue, p);
		}
	}
}

int kexit(int exitValue)
{
	PROC *p;
	int i, wakeupP1 = 0;
	if(running->pid ==1 && nproc > 2)
	{
		printf("other procs still exist, P1 can't die yet\n");
		return -1;
	}
	/*send children (dead or alive) to P1's orphanage*/
	for(i = 0; i < NPROC; i++)
	{
		p = &proc[i];
		if (p->status != FREE && p->ppid == running->pid)
		{
			p->ppid = 1;
			p->parent = &proc[1];
			wakeupP1++;
		}	
	}

	/*record exitValue and become a ZOMBIE*/
	running->exitCode = exitValue;
	running->status = ZOMBIE;
	/*wakeup parent and also P1 if necesary*/
	wakeup(running->parent); /*/parent sleeps on its PROC address*/
	if(wakeupP1)
	{
		wakeup(&proc[1]);
	}
	tswitch(); /*/Give up CPU*/
}

int kwait(int *status) /*/wait for ZOMBIE child*/
{
	PROC *p;
	int i, hasChild = 0;
	while(1)
	{
		for(i = 0; i<NPROC; i++)
		{
			p = &proc[i];
			if(p->status != FREE && p->ppid == running->pid)
			{
				hasChild = 1;  /*/flag for child*/
				if(p->status == ZOMBIE)  /*/lay the dead child to rest*/
				{
					*status = p->exitCode;  /*collect it's exit code*/
					p->status = FREE;   /*free its PROC*/
					put_proc(&freeList, p);   /*/to freeList*/
					nproc--;
					return p->pid; /*/return its pid*/
				}
			}
		}
	}
	if (!hasChild)
	{
		return -1; /*/no child return error*/
	}

	ksleep(running); /*still has kids alive*/
}
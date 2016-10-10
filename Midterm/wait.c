int removeProc(PROC **queue, PROC *p)
{
  PROC *cur = *queue, *prev = *queue;

  if(!*queue) //empty queue
  {
    return 0;
  }

  if(*queue == p) //proc p is first in list
  {
    *queue = p->next;
    p->next = 0;
    return p;
  }
  cur = (*queue)->next;
  while(cur)
  {
    if(cur == p) //found proc to remove, remove and return
    {
      	prev->next = cur->next;
      	p->next = 0;
      	return cur;
    }
    prev = cur;
    cur = cur->next;
  }
  return 0;
}

int ksleep (int event)
{
	running->event = event;
	running->status = SLEEP;
	enqueue(&sleepList, running);
	proc[running->ppid].status = READY;
	tswitch();
}

int kwakeup (int event)
{
	int i, counter = 0;
	PROC *p = sleepList, *next;
	printf("wakeup!\n");
	if (p == 0)
	{
		printf("There are no sleeping procs to wakeup.\n");
		return -1;
	}

	else
	{
		//printList("Sleeping", &sleepList);
		while (p)
		{
			next = p->next;
			//printf("Comparing event code: %x with wakup code: %x", p->event, event);
			if (p->event == event)
			{
				removeProc(&sleepList, p);
				p->status = READY;
				enqueue(&readyQueue, p);
				counter++;
			}
			p = next;
		}
	}
	return counter;
}

int ready (PROC *p)
{
	p->status = READY;
	p->next = 0;
	enqueue(&readyQueue, p);
}

// Kill the running process
int kexit (int exitValue)
{
	PROC *p;
	int i, wakeupP1 = 0;
	if (running->pid==1 && nproc>2)
	{
		printf("other procs still exist, P1 can't die yet\n");
		return -1;
	}

	// send children (dead or alive) to P1's orphanage
	for (i = 1; i < NPROC; i++)
	{
		p = &proc[i];
		if (p->status != FREE && p->ppid == running->pid)
		{
			p->ppid = 1;
			p->parent = &proc[i];
			wakeupP1++;
		}
	}
	//record exitValue and become a ZOMBIE
	running->exitCode = exitValue;
	running->status = ZOMBIE;
	//wakeup parent and also P1 if necessary
	kwakeup(running->parent);	//parent sleeps on its PROC address
	if (wakeupP1)
		kwakeup (&proc[1]);
	
	tswitch();
}

int kwait (int *status) // wait for zombie child
{
	PROC *p; int i, hasChild = 0;

	while (1)
	{
		for (i = 1; i < NPROC; i++)			// search PROCs for a child
		{
			p = &proc[i];					// exclude p0
			if (p->status != FREE && p->ppid == running->pid)
			{
				hasChild = 1;				// hasChild flag
				if (p->status == ZOMBIE)	// lay the dead child to rest
				{
					*status = p->exitCode;	// collect its exitCode
					p->status = FREE;		// free its PROC to freeList
					enqueue(&freeList, p);
					nproc--;
					return(p->pid);
				}
			}
		}
		if (!hasChild) return -1;		// no child, return ERROR
		ksleep(running); // still has kids alive: sleep on PROC address
	}
}

int do_wait()
{
    int pid, status;
    pid = kwait(&status);
    if (pid == -1)
    {
        printf("\nProcess has no children!");
        return -1;
    }
    else
    {
        printf("\npid=%d, status=%d\n", pid, status);
    }
    return 1;
}


//                     kernel.c file:

//             YOUR kernel.c file as in LAB3
//
//	     MODIFY kfork() to

////////MODIFY KFORK TO HANDLE FILENAME /////////////////

int body()
{
	char c;
	while(1)
	{
		if (rflag)
		{
			printf("proc %d: reschedule\n", running->pid);
			rflag = 0;
			tswitch();
		}
		printList("freelist", freeList);
		printList("readyQueue", readyQueue);
		printf("proc%d running: priority = %d parent = %d enter a char [s | f | w | q | u]: ", running->pid, running->priority, running->ppid);
		c = getc();
		printf("%c\n", c);
		switch(c)
		{
			case 'f' : do_kfork();		break;
			case 's' : do_tswitch();	break;
			case 'w' : do_wait();		break;
			case 'q' : do_exit();		break;
			case 'u' : goUmode();		break;
			default: printf("invalid command\n"); break;
		}
	}
}

PROC *kfork(char *filename)  // forks new child
{
    int i, j, segment;
    
    // Get the new proc
    PROC *p = get_proc(&freeList);
    if (p == 0)
    {
        printf("No Procs Available\n");
        return 0;
    }
    nproc++;
    // initialize the proc...
    p->status = READY; 
    p->priority = 1;
    p->ppid = running->pid;
    p->parent = running;
    
    // initialize stack
    for (j = 1; j<10; j++)
        p->kstack[SSIZE - j] = 0;
    p->kstack[SSIZE - 1] = (int)goUmode;
    p->ksp = &(p->kstack[SSIZE - 9]);
    
    // load umode
    segment = (p->pid + 1) * 0x0800 + 0x1000; //modified here for 16 procs
    if (filename)
    	load(filename, segment);
    for(i = 0; i < 13; i++)
        put_word(0,segment, segment-(i * 2));
    put_word(0x0200, segment, segment-2);
    put_word(segment, segment, segment-4);
    put_word(segment, segment, segment-22);
    put_word(segment, segment, segment-24);
    p->uss = segment;
    p->usp = segment-24;
    
    enqueue(&readyQueue, p);
    printf("Proc[%d] forked child Proc[%d] at segment %x\n", running->pid, p->pid, segment);
    
    return p;
}

int do_kfork()
{
	PROC *p = kfork();
	if (p == 0)
	{
		printf("kfork failed\n");
		return -1;
	}
	printf("PROC %d kfork a child %d\n", running->pid, p->pid);
	return p->pid;
}

int do_tswitch()
{
	tswitch();
}

int do_umode()
{
	goUmode();
}

int do_exit()
{
	int event;
	printf ("Enter exit code: ");
	event = geti();
	printf ("%d\n", event);
	kexit(event);
}

int do_sleep()
{
	int event;
	printf ("Enter sleep code: ");
	event = geti();
	sleep(event);
}

int do_wakeup()
{
	int event;
	printf ("Enter wakeup code: ");
	event = geti();
	kwakeup(event);
}

int reschedule ()
{
	PROC *p, *tempQ = 0;
	while ( (p=dequeue(&readyQueue)))	// reorder readyQueue
	{
		enqueue(&tempQ, p);
	}

	readyQueue = tempQ;
	rflag = 0;		// global reschedule flag
	if (running->priority < readyQueue->priority)
		rflag = 1;
}

int chpriority (int pid, int pri)
{
	PROC *p; int i, ok = 0, reQ = 0;
	if (pid == running->pid)
	{
		running->priority = pri;
		if (pri < readyQueue->priority)
		{
			rflag = 1;
		}
		return 1;
	}
	// if not for running, for both READY and SLEEP procs
	for (i = 1; i<NPROC; i++)
	{
		p = &proc[i];
		if (p->pid == pid && p->status != FREE)
		{
			p->priority = pri;
			ok = 1;
			if (p->status == READY)
				reQ = 1;
		}
	}
	if (!ok)
	{
		printf ("chpriority failed\n");
		return -1;
	}
	if (reQ)
		reschedule(p);
}

int do_chpriority()
{
	int pid, pri;
	printf("input pid ");
	pid = geti();
	printf("input new priority ");
	pri = geti();
	if (pri < 1) pri = 1;
	chpriority(pid, pri);
}

int geti()
{
	char s[32];
	gets(s);
	return atoi(s);
}

void enqueue(PROC **queue, PROC *p)
{
	PROC *current = *queue, *next;
	// Empty queue
	if ((*queue) == NULL)
	{
		*queue = p;
		p->next = 0;
	}

	// new proc goes at head of queue
	else if (p->priority > (*queue)->priority)
	{
		p->next = (*queue);
		(*queue) = p;
	}

	// new process inserted somewhere within queue according to priority
	else
	{
		current = (*queue);
		next = current->next;

		while (next != NULL && p->priority <= next->priority)
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
	// find correct process
	PROC *p = *queue;

	if (*queue != NULL)
		*queue = (*queue)->next;

	return p;
}

void printList(char *name, PROC *queue)
{
    PROC *p;
    
    // check for empty queue
    if (queue == NULL)
    {
    	printf("%s: ", name);
        printf("There are no processes in this queue.\n");
    }
    // if not empty, print the queue!
    else
    {
        p = queue->next;
        printf("%s: ", name);
        printf("[%d]", queue->pid);
        while (p != NULL)
        {
            printf(" -> [%d]", p->pid);
            p = p->next;
        }
        printf(" -> NULL\n");
    }
}


void getStringFromUMode(char *s, char uModeStr[])
{
    char byte;
    int i = 0;

    //read byte from segment
    byte = get_byte(running->uss, s); //check if it's in uss

    while(byte) {
        //putc(byte);
        uModeStr[i++] = byte;
        
        //get another byte
        byte = get_byte(running->uss, s + i);
    }
    uModeStr[i] = 0;
    return;
}

void putStringToUMode(char *s, char uModeStr[])
{
    int i = 0;
    char *p = uModeStr;

    while(*p) {
        put_word(*p, running->uss, s + i);
        i++;
        p++;
    }

    put_word(0, running->uss, s + i); //make sure it's null terminated
}
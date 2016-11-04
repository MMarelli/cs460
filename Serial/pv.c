struct semaphore{
  int value;
  PROC *queue;
};

// Request access
int P(struct semaphore *s)
{
	//lock();
	// write YOUR C code for P()
	if (--s->value < 0)
	{
		running->status = BLOCK;
		enqueue(&s->queue, running);
		unlock();
		while(running->status == BLOCK)
		{

		}
	}
	//unlock();
}

// Close access
int V(struct semaphore *s)
{
  	// write YOUR C code for V()
	PROC *p;
	//lock();
	if(++s->value <= 0)
	{
		p = dequeue(&s->queue);
		p->status = READY;
		p->next = 0;
		enqueue(&readyQueue, p);
	}
	//unlock();
}

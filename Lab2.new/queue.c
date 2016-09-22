int get_nproc()
{
	int count = 0, i;

	for(i = 0; i < NPROC; i++)
	{
		if(proc[i].status == FREE)
		{
			count++;
		}
	}
	return count;
}
void enqueue(PROC **queue, PROC *p)
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

		while (next && p->priority <= next->priority)
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

PROC *get_proc(PROC **list)			/*/ return a first PROC pointer from list*/
{
	if (list != 0)
		return dequeue(list);
	return 0;
}

int put_proc (PROC **list, PROC *p)	/*/ enter p into list*/
{
	PROC *cur, *prev;
	/*/ Check list for other items, otherwise list = p*/
	if (!(*list) || (*list)->pid > p->pid)
	{
		p->next = *list;
		(*list) = p;
	}
	else
	{
		cur = (*list)->next;
		prev = *list;
		while(cur && p->pid > cur->pid)
		{
			prev = cur;
			cur = cur->next;
		}
		p->next = cur;
		prev->next = p;
	}
}

int remove_proc(PROC **list, PROC *p)
{
	PROC *cur = *list, *prev = *list;
	if(!cur)
	{
		return -1;
	}

	if(cur == p) /*If the item is the first*/
	{
		*list = cur->next;
	}
	else
	{
		while(cur = cur->next) /*go through the list until empty*/
		{
			if(cur = p) /*remove it*/
			{
				prev->next = cur->next;
				break;
			}
			prev = cur;
		}
	}

	if(!cur)
	{
		return -1;
	}
	cur->next = 0;
	return 1;

}

void printList(char *name, PROC *queue)
{
    PROC *p;
    
    /*/ check for empty queue*/
    if (queue == 0)
    {
    	printf("%s: ", name);
        printf("There are no processes in this queue.\n");
    }
    /*/ if not empty, print the queue!*/
    else
    {
        p = queue->next;
        printf("%s: ", name);
        printf("[%d]", queue->pid);
        while (p != 0)
        {
            printf(" -> [%d]", p->pid);
            p = p->next;
        }
        printf(" -> NULL\n");
    }
}

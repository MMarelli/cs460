char *MODE[] = {"READ_PIPE", "WRITE_PIPE"};

int show_pipe(PIPE *ptr)
{
    int i, j = ptr->tail;
    
    printf("Pipe %x\n", ptr);
    printf("Head: %u, Tail: %u, Data: %u, Room: %u\n",
           ptr->head, ptr->tail, ptr->data, ptr->room);
    printf("Readers: %u, Writers: %u\n", ptr->nreader, ptr->nwriter);
    printf("------------------ PIPE ------------------\n");
    for (i=ptr->data; i>0; i--) {
        if (i > PSIZE) {
            printf("Invalid pipe address!\n");
            break;
        }
        if (j == PSIZE) { j = 0; }
        printf("%c", ptr->buf[j++]);
    }
    printf("\n----------------------------------------\n");
    
    return(0);
}

int pfd()
{
    int i;
    OFT *ptr;
    
    // print running process' opened file descriptors
    printf("Proc %d file descriptors:\n", running->pid);
    printf("----------------------------\n");
    printf("- FD Mode pipe   refCount  -\n");
    printf("----------------------------\n");
    for(i=0; i<NFD; i++) {
        ptr = running->fd[i];
        printf("- %u  ", i);
        if (ptr != 0) {
            switch(ptr->mode) {
                case(READ_PIPE): printf("%c", 'R'); break;
                case(WRITE_PIPE): printf("%c", 'W'); break;
                default: printf("%c", ' ');
            }
            printf("    %x %d", ptr->pipe_ptr, ptr->refCount);
        }
        printf("\n");
    }
    printf("----------------------------\n");
}

int read_pipe(int pfd, char *buf, int n)
{
    int r = 0;
    PIPE *ptr = running->fd[pfd]->pipe_ptr;
    
    ptr->nreader++;

    if (pfd < 0 || pfd > NFD - 1 || running->fd[pfd] == 0) //File Handle doesn't exist or out of bounds
    {
        printf("Bad File Handle Index(%d)\n", pfd);
        return(-1);
    }
    if (running->fd[pfd]->mode != READ_PIPE)  //File Handle isn't a READ_PIPE
    { 
        printf("Bad File Handle Mode\n", pfd);
        return(-1);
    }

    if (n <= 0) //read no bytes
    {
        return 0;
    }
    
    // read data
    while (n) {
        //printf("In outer loop");
        while(ptr->data) {
            //printf("In loop read\n");
            put_byte(ptr->buf[ptr->tail++], running->uss, buf++);
            if (ptr->tail == PSIZE)
            {
                ptr->tail = 0;
            }
            ptr->room++;
            ptr->data--;
            r++;
            n--;
            if (n==0)
            {
                break;
            }
        }
        if (r) //pipe read some data but data still left to read
        {
            //printf("Wakup Writer\n");
            kwakeup(&(ptr->room));
            return r;
            //printf("Put Reader to sleep\n");
            //ksleep((&ptr->data));
        }
        // pipe has no data
        
        if (ptr->nwriter) //if pipe still has writer
        {
            //printf("Wakup Writer\n");
            kwakeup(&(ptr->room));
            //printf("Put Reader to sleep\n");
            ksleep(&(ptr->data)); //sleep for data
        }
        else 
        {
            printf("Pipe doesn't have a writer - Closing Pipe\n", ptr);
            close_pipe(pfd);
            return 0;
        }
        //printf("Reader Awoke\n");
    }
    
    return(r);
}

int write_pipe(int pfd, char *buf, int n)
{
    char str[1024];
    int r = 0;

    PIPE *ptr = running->fd[pfd]->pipe_ptr;

    if (pfd < 0 || pfd > NFD - 1 || running->fd[pfd] == 0) //File Handle doesn't exist or out of bounds
    {
        printf("Bad File Handle Index(%d)\n", pfd);
        return(-1);
    }
    if (running->fd[pfd]->mode != WRITE_PIPE)  //File Handle isn't a WRITE_PIPE
    { 
        printf("Bad File Handle Mode\n", pfd);
        return(-1);
    }

    if (n <= 0)
        return 0;

    printf("n = %d room = %d", n, ptr->room);

    while (n > 0) {
        if (!ptr->nreader){
            printf("No Pipe Readers\n");
            kexit(-1);
        }
        while (ptr->room > 0) {
            printf("n = %d room = %d", n, ptr->room);
            geti();
            ptr->buf[ptr->head++] = get_byte(running->uss, buf++);
            if (ptr->head == PSIZE)
            {
                ptr->head = 0;
            }
            ptr->room--;
            ptr->data++;
            n--;
            r++;
            if (n<=0)
                break;
        }
        //printf("waking up reader\n");
        kwakeup(&ptr->data); //wakeup data reader

        if (n<=0) //wrote all of its data
            return r;

        // No room, wait
        //printf("Putting Waiter to sleep on room\n");
        ksleep(&ptr->room); //put writer to sleep on room
        //printf("Writer Awoke\n");
    }
    
    return(0);
}

OFT *get_fd()
{
    OFT *ptr = 0;
    int i = 0;
    
    while (i < NFD) {
        ptr = &oft[i++];
        if (ptr->refCount == 0) 
        {
            return(ptr);
        }
    }
    
    printf("Unable to get File Descriptor\n");
    return(0);
}

PIPE *get_pipe()
{
    PIPE *ptr;
    int i = 0;
    
    while (i < NPIPE)
    {
        ptr = &pipe[i++];
        if (ptr->busy == NULL)
        {
            return(ptr);
        }
    }
    
    printf("Unable to ge Pipe\n");
    return(0);
}

int kpipe(int pd[2])
{
    OFT *rptr = 0, *wptr = 0;
    PIPE *p_ptr;
    int i = 0, j = 0;  

    //get an empty pipe to use
    rptr = get_fd();
    rptr->refCount = 1;
    wptr = get_fd();
    p_ptr = get_pipe();

    //find open file descriptors
    while (i < NFD && running->fd[i] != 0) { i++; }
    while (j < NFD && (j == i || running->fd[j] != 0)) { j++; }
    if (rptr == NULL || wptr == NULL ||
        p_ptr == NULL || i == NFD || j == NFD) {
        rptr->refCount = 0;
        return(-1);
    }
    
    //assign pipe to proc
    running->fd[i] = rptr;
    put_word(i, running->uss, pd);
    running->fd[j] = wptr;
    put_word(j, running->uss, pd++);
    
    // set initial pipe values
    p_ptr->head = 0;
    p_ptr->tail = 0;
    p_ptr->data = 0;
    p_ptr->room = PSIZE;
    p_ptr->nwriter = 1;
    p_ptr->nreader = 1;
    p_ptr->busy = 1;
    
    rptr->mode = READ_PIPE;
    rptr->pipe_ptr = p_ptr;
    wptr->mode = WRITE_PIPE;
    wptr->refCount++;
    wptr->pipe_ptr = p_ptr;
    
    return(0);
}

int close_pipe(int fd)
{
    OFT *op; PIPE *pp;
    int i;

    op = running->fd[fd];
    running->fd[fd] = 0;
    pp = op->pipe_ptr;
  
    if (op->mode == READ_PIPE) {
        pp->nreader--;  
        
        if (--op->refCount == 0) 
        {
            if (pp->nwriter <= 0) 
            {
                pp->busy = 0; 
                return;
            }
        }
        kwakeup(&pp->room); 
        return;
    }
    
    if (op->mode == WRITE_PIPE) 
    {
        pp->nwriter--;
        
        if (--op->refCount == 0) 
        {
            if (pp->nreader <= 0) 
            {
                pp->busy = 0;   
                return;
            }
        }
        kwakeup(&pp->room); 
        return;
    }
}
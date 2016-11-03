
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
int kcinth()
{
   int a,b,c,d, r;

//==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 
    a = get_word(running->uss, running->usp + 26);
    b = get_word(running->uss, running->usp + 28);
    c = get_word(running->uss, running->usp + 30);
    d = get_word(running->uss, running->usp + 32);

    printf(a);
    printf('\n');
   switch(a){
       case 0 : r = kgetpid();          break;
       case 1 : r = kps();              break;
       case 2 : r = kchname(b);         break;
       case 3 : r = kkmode();           break;
       case 4 : r = ktswitch();         break;
       case 5 : r = kkwait(b);          break;
       case 6 : r = kkexit(b);          break;
       case 7 : r = fork();             break;
       case 8 : r = kexec(b);           break;
       case 9 : r = ksout(b);           break;
       case 10: r = ksin(b);            break;

       case 30 : r = kpipe(b);          break;
       case 31 : r = read_pipe(b,c,d);  break;
       case 32 : r = write_pipe(b,c,d); break;
       case 33 : r = close_pipe(b);     break;
       case 34 : r = pfd();             break;
       //case 35 : r = kkshow_pipe(b);   break;

       case 36 : r = khop(b);            break;

       case 90: r = getc();             break;
       case 91: r = putc(b);            break;
       case 99: kkexit(b);              break;
       default: printf("invalid syscall # : %d\n", a); 
   }

//==> WRITE CODE to let r be the return value to Umode
    put_word(r, running->uss, running->usp + 16);
    return r;
}

//============= WRITE C CODE FOR syscall functions ======================

int ksin(char *y)
{
  // get a line from serial port 0; write line to Umode
  int len;
  int i = 0;
  char in[64];

  len = sgetline(0, in);

  do
  {
    put_byte(in[i++], running->uss, y++);
  } while (in[i] != '\0');

  return len;
}

int ksout(char *y)
{
  // get line from Umode; write line to serial port 0
  int i = 0;
  char c;
  char out[64];
  printf("In ksout\n");
  do
  {
    c = get_byte(running->uss, y);
    out[i] = c;
    y++;
    i++;
  } while (c);

  sputline(&stty[0], out);
  printf("End of ksout\n");
}

int get_str(char *str, char *buf, int size) {
    int i = 0;
    
    while (size == 0 || i < size) {
        *(buf + i) = get_byte(running->uss, str + i);
        if (*(buf + i) == '\0') {
            break;
        }
        i++;
    }
    
    return(i);
}

int kkmode()
{
    body();
}

int kexec(char* y)
{
    int i = 0, length = 0;
    char filename[64];
    char *cp = filename;
    char *file;
    int start = *cp;

    
    // same segment
    u16 segment = running->uss; 
    
    //get filename from U space with a length limit of 64
    while ((*cp++ = get_byte(running->uss, y++)) && length++ < 64);
    
    printf("filename: %s\n", filename);
    if (!load(filename, segment))
        return -1;
    
    // put the bytes in their right location!
    for( i = 1; i <= 12; i++)
    {
        put_word(0, segment, -2 * i);
    }
    
    running->usp = -24;

    // -1    -2  -3 -4 -5 -6 -7 -8 -9 -10 -11 -12
    // flag uCS uPC ax bx cx dx bp si di  uES  uDS 
    put_word(segment, segment, (-2 * 12)); // saved uDS=segment
    put_word(segment, segment, (-2 * 11)); // saved uES=segment
    put_word(segment, segment, (-2 * 2));  // uCS=segment; uPC=0
    put_word(0x0200, segment,  (-2 * 1));  // Umode flag=0x0200
}

int copyImage(u16 pseg, u16 cseg, u16 size)
{
    u16 i;
    for (i = 0; i < size; i++) {
        put_word(get_word(pseg, 2 * i), cseg, 2 * i);
    }
}

int fork()
{
    int pid, i; u16 segment;
    PROC *p = kfork(0); // kfork() a child, do not load image file
    if (p == 0) return -1; // kfork failed
    segment = (p->pid + 1) * 0x0800 + 0x1000; // child segment
    printf("fork\n");
    copyImage(running->uss, segment, 16 * 1024); // copy 32K words
    p->uss = segment; // child’s own segment
    p->usp = running->usp; // same as parent's usp
                           //*** change uDS, uES, uCS, AX in child's ustack ****
    put_word(segment, segment, p->usp); // uDS=segment
    put_word(segment, segment, p->usp + 2); // uES=segment
    put_word(0, segment, p->usp + 2 * 8); // uax=0
    put_word(segment, segment, p->usp + 2 * 10); // uCS=segment

    //copy pipes/file descriptors
    for (i=0; i<NFD; i++)
    {
        if(running->fd[i])
        {
            p->fd[i] = running->fd[i];
            p->fd[i]->refCount++;
            
            if(p->fd[i]->mode == READ_PIPE)
            {
                p->fd[i]->pipe_ptr->nreader++;
            }

            if(p->fd[i]->mode == WRITE_PIPE)
            {
                p->fd[i]->pipe_ptr->nwriter++;
            }
        }
    }

    enqueue(&readyQueue, p);

    return p->pid;
}

int kkpipe(int *pd) {
    return kpipe(pd);
}

/*int kkwrite(int fd, char *cp, int n) {
    char str[32];
    getStringFromUMode(cp, str);
    write_pipe(fd, str, n);
    return 0;
}

int kkread(int fd, char *cp, int n) {
    char str[32];
    read_pipe(fd, str, n);
    putStringToUMode(cp, str);
    return 0;
}*/

int kkclose(int b) {
    close_pipe(b);
    return 0;
}
/*
int kkshow_pipe(int fd_num) {
    return(show_pipe(running->fd[fd_num]->pipe_ptr));
}
*/
int kgetpid()
{
    //WRITE YOUR C code
  return running->pid;
}

int kps()
{
    //WRITE C code to print PROC information
    int i, count;
    bool printinfo = false;
    bool printevent = false;
    bool printexitcode = false;
    PROC *p;
    
    printf("***********************************************************************\n");
    printf("   name       status       pid       ppid       event       exitcode\n");
    printf("***********************************************************************\n");
    
    for (i = 0; i < NPROC; i++)
    {
        printinfo = true;
        printevent = false;
        printexitcode = false;
        p = &proc[i];
        count = 14 - strlen(p->name);
        printf("%s", p->name);
        while (count-- > 0)
            printf(" ");
        
        // write the status and set the information variables
        switch (p->status)
        {
            case FREE:
                printf("FREE         ");
                printinfo = false;
                printexitcode = true;
                break;
            case READY:
                printf("READY        ");
                break;
            /*case RUNNING:
                printf("RUNNING      ");
                break;*/
            case SLEEP:
                printf("SLEEP        ");
                printevent = true;
                break;
            case ZOMBIE:
                printf("ZOMBIE       ");
                printevent = true;
                break;
        }
        
        // show pid and ppid?
        if (printinfo == true)
            printf("%d         %d         ", p->pid, p->ppid);
        else
            printf("                    ");
            
        // show event num?
        if (printevent == true)
            printf("%d             ", p->event);
        else
            printf("              ");
        
        // show exit code?
        if (printexitcode == true)
            printf("%d\n", p->exitCode);
        else
            printf("\n");
    }
    printf("***********************************************************************\n");
    return true;
}

int kchname(char *name)
{
    //WRITE C CODE to change running's name string;
    char c;
    int i = 0;

    while (i < 32)
    {
        c = get_byte(running->uss, name + i);
        running->name[i] = c;
        if (c == '\0') { break; }
        i++;
    }
}

int kkfork()
{
    int r = -1, i = 0;
    PROC *p = kfork("/bin/u1");

    if(p) {
        r = p->pid;

        for(; i < NFD; i++) {
            p->fd[i] = running->fd[i];

            if(p->fd[i]) {
                p->fd[i]->refCount++;
                p->fd[i]->pipe_ptr->nreader++;
                p->fd[i]->pipe_ptr->nwriter++;
            }

        }

        enqueue(&readyQueue, p);
    }
    return (p->pid);
}

int ktswitch()
{
    return tswitch();
}

int kkwait(int *status)
{
    int pid, c;
    pid = kwait(&c);
    put_word(c, running->uss, status);
    return pid;
}

int kkexit(int value)
{
    return kexit(value);
}

int get_word(u16 segment, u16 offset)
{
  u16 word;
  setds(segment);
  word = *(u16 *)offset;
  setds(0x1000);
  return word;
}

int put_byte(u8 byte, u16 segment, u16 offset)
{
    setds(segment);
    *(u8 *)offset = byte;
    setds(0x1000);
}

int khop (int newSegment)
{
    PROC *p = running;
    int segment = (newSegment + 1) * 0x0800 + 0x1000;
    copyImage(running->uss, segment, 16 * 1024); // copy 32K words
    p->uss = segment; // child’s own segment
    p->usp = running->usp; // same as parent's usp
                           //*** change uDS, uES, uCS, AX in child's ustack ****
    put_word(segment, segment, p->usp); // uDS=segment
    put_word(segment, segment, p->usp + 2); // uES=segment
    put_word(0, segment, p->usp + 2 * 8); // uax=0
    put_word(segment, segment, p->usp + 2 * 10); // uCS=segment
}

// gets a string and returns it
char* getstring(char *str)
{
    // vars
    char temp[64];
    int i;
    u16 segment = (running->pid) * 0x0800 + 0x1000;
    
    // while we've not yet reached the end of the string, cycle...
    for (i = 0; i < 64; i++)
    {
        // get the next char and make sure we've not reached the end of the string.
        temp[i] = get_byte(segment, str + i);
        if (temp[i] == '\0')
            break;
    }
    
    // make sure that we end on a nullchar and return the string
    temp[i] = '\0';
    return temp;
}
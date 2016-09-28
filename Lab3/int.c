
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
/*==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack */
  u16 segment, offset;
  segment = running->uss;
  offset = running->usp;
  a = get_word(segment, offset + 2*13);
  b = get_word(segment, offset + 2*14);
  c = get_word(segment, offset + 2*15);
  d = get_word(segment, offset + 2*16);


   switch(a){
       case 0 : r = kgetpid();        break;
       case 1 : r = kps();            break;
       case 2 : r = kchname(b);       break;
       case 3 : r = kkfork();         break;
       case 4 : r = ktswitch();       break;
       case 5 : r = kkwait(b);        break;
       case 6 : r = kkexit(b);        break;

       case 99: kkexit(b);            break;
       default: printf("invalid syscall # : %d\n", a); 
   }

/*==> WRITE CODE to let r be the return value to Umode*/
   put_word(r, segment, offset + 16);
   return r;
}

/*//============= WRITE C CODE FOR syscall functions ======================*/

int kgetpid()
{
    return running->pid;
}

char * getStatusString(int status)
{
  switch(status)
  {
    case FREE:
      return "FREE\0";
    case READY:
      return "READY\0";
    case RUNNING:
      return "RUNNING\0";
    case SLEEP:
      return "SLEEP\0";
    case ZOMBIE:
      return "ZOMBIE\0";
    default:
      return "";
    }
}
int kps()
{
    /*WRITE C code to print PROC information*/
  int i;
  printf("************************************\n");
  printf("Name:     Status     pid     ppid\n");  
  printf("************************************\n");
  for(i = 0; i < NPROC; i++)
  {
    printf("%s     %s     %d     ", proc[i].name, getStatusString(proc[i].status), i);
    if(proc[i].status == FREE)
    {
      printf("\n");
    }
    else
    {
      printf("%d\n", proc[i].ppid);
    }
  }
  printf("************************************\n");
}

int kchname(char *name)
{
    /*WRITE C CODE to change running's name string;*/
  int i = 0;

  while(i < 32)
  {
    running->name[i] = get_byte(running->uss, name + i); /*name[i]*/
    i++;
  }
  return 0;
}

int kkfork()
{
  PROC *p = 0;
  /*use you kfork() in kernel;*/
  p = kfork("/bin/u1");
  if(p < 0)
  {
    return -1;
  }

  return p->pid; /*child pid or -1 to Umode!!!*/
}

int ktswitch()
{
    return tswitch();
}

int kkwait(int *status)
{
  /*  use YOUR kwait() in LAB3;*/
  return kwait(status); /*values to Umode!!!*/
}

int kkexit(int value)
{
  if(running->pid = 1)
  {
    printf("P1 Cannot Exit\n");
    return -1;
  }
    /*use your kexit() in LAB3
    do NOT let P1 die*/
  return kexit(value);
}

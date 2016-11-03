#include "ucode.c"

main0(char *s)
{
    char *cp = s;
    char *tempP;
    int i = 0;
    int start = 0;
    int argc = 0;
    char temp [100];

    // Can hold up to 50 arguments
    char *argv[50];

    // Skip if we have a blank char
    while (*cp == 42)
    {
        cp++;
    }
    
    while (*cp)
    {
        temp[i] = *cp;
        if (*cp == ' ' || *cp == '\0')
        {
            // null terminate
            temp[i] = 0;
            tempP = &temp[start];
            argv[argc] = tempP;
            // increment argc and start index
            argc++;
            start = i + 1;  
        }
        cp++;
        i++;

    }
    argv[argc] = 0;
    main(argc, argv);
}

main(int argc, char *argv[])
{ 
  char name[64];
  int pid, cmd, i;

  printf("enter main : argc = %d\n", argc);

    
  for (i = 0; i<argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  while(1){
    pid = getpid();
    color = 0x0C;
       
    printf("***********************************************\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : exit();     break;
           case 7 : fork();     break;
           case 8 : uexec();    break;
           case 9 : pipe();     break;
           case 10 : pfd();     break;
           case 11 : read();    break;
           case 12 : write();   break;
           case 13 : close();   break;
           case 14 : hop();     break;

           default: invalid(name); break;
    }
  }
}




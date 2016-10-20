/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00	   /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	   /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L	   /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60 /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

u16 tick;
u16 hours, minutes, seconds;

int enable_irq(u16 irq_nr)
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

  printf("timer init\n");
  tick = 0; 
  hours = minutes = seconds = 0;


  out_byte(TIMER_MODE, SQUARE_WAVE);	// set timer to run continuously
  out_byte(TIMER0, TIMER_COUNT);	// timer count low byte
  out_byte(TIMER0, TIMER_COUNT >> 8);	// timer count high byte 
  enable_irq(TIMER_IRQ); 
}

/*===================================================================*
 *		    timer interrupt handler       		     *
 *===================================================================*/
int thandler()
{
  /*
  tick++; 
  tick %= 60;
  if (tick == 0){                      // at each second
      printf("1 second timer interrupt in ");
      running->inkmode > 1 ? putc('K') : putc('U');
      printf("mode\n");
  }
  out_byte(0x20, 0x20);                // tell 8259 PIC EOI
  */
  u16 base = 0xB800;
  int w, i; 
  int orig_col = column;
  int orig_row = row;
  column = 70;
  row = 24;
  color = 0x0A;

  if(!(++tick%60))
  {
    seconds++;
    if(!(seconds%60))
    { 
      minutes++;
      seconds = 0;
      if(!(minutes%60))
      {
        hours++;
        minutes = 0;
      }
    }
  }

  putc(hours/10 + '0');
  putc(hours%10 + '0');
  putc(':');
  putc(minutes/10 + '0');
  putc(minutes%10 + '0');
  putc(':');
  putc(seconds/10 + '0');
  putc(seconds%10 + '0');

  column = orig_col;
  row - orig_row;

  out_byte(0x20, 0x20);
}

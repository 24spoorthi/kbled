#include<linux/module.h>
#include<linux/configfs.h>
#include<linux/init.h>
#include<linux/tty.h>
#include<linux/kd.h>
#include<linux/vt.h>
#include<linux/console_struct.h>
#include<linux/vt_kern.h>
#include<linux/sched.h>

MODULE_DESCRIPTION("Programme to flash keyboard led(s)");
MODULE_AUTHOR("SPOORTHI");
MODULE_LICENSE("GPL");

struct tty_driver *my_driver;
char kbled_status = 0;
unsigned long ptr;

#define BLINK_DELAY HZ/5
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF

#ifndef CLOCK_TICK_RATE
#define CLOCK_TICK_RATE 1193180
#endif
#define DEFAULT_FREQ 440.0

struct timer_data{
  struct timer_list timer;
  unsigned long data;
}tmd;

static void print_str(char* str){
  struct tty_struct *tty_det;
  tty_det = get_current_tty();
  if(tty_det != NULL){
    ((tty_det->driver)->ops->write)(tty_det,str,strlen(str));
    ((tty_det->driver)->ops->write)(tty_det,"\015\012",2);
  }
}
static void my_timer_func(struct timer_list *t){
  struct timer_data *td = from_timer(td,t,timer);
  printk(KERN_INFO "td->data : %ld\n",td->data);
  int *pstatus = (int*)td->data;
  if(*pstatus == ALL_LEDS_ON){
    printk(KERN_INFO "in if part\n");
    *pstatus = RESTORE_LEDS;
    printk(KERN_INFO "*pstatus-> %d\n",*pstatus);
  }
  else{
    printk(KERN_INFO "in else part\n");
    *pstatus = ALL_LEDS_ON;
  }
  (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty,KDSETLED,*pstatus);
  (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KIOCSOUND,CLOCK_TICK_RATE/DEFAULT_FREQ);
  mod_timer(&td->timer,jiffies + BLINK_DELAY);
}

static int __init kbleds_init(void){
  int i;
  print_str("This module has been inserted.....Heyaa!!");
  printk(KERN_INFO "kdleds loading\n");
  printk(KERN_INFO "kbleds fgconsole is %x\n",fg_console);
  for(i = 0 ; i < MAX_NR_CONSOLES ; i++){
    if(!vc_cons[i].d)
      break;
    printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n",i,MAX_NR_CONSOLES,vc_cons[i].d->vc_num,(unsigned long)vc_cons[i].d->port.tty);
  }
  printk(KERN_INFO "kbleds finished scanning consoles\n");
  my_driver = vc_cons[fg_console].d->port.tty->driver;
  printk(KERN_INFO "kbleds tty driver magic %x\n",my_driver->magic);
  tmd.data = (unsigned long)&kbled_status;
  tmd.timer.expires = jiffies + BLINK_DELAY;
  tmd.timer.flags = 0;
  timer_setup(&tmd.timer,my_timer_func,0);
  add_timer(&tmd.timer);
  return 0;
}
static void __exit kbleds_exit(void){
  print_str("This module has ben removed.....Bye felica");
  printk(KERN_INFO "kbleds unloading...\n");
  del_timer_sync(&tmd.timer);
  (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty,KDSETLED,RESTORE_LEDS);
  (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KIOCSOUND,0x00);
}

module_init(kbleds_init);
module_exit(kbleds_exit);	 

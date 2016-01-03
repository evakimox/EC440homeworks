# include <linux/init.h>
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/slab.h> /* kmalloc */
# include <linux/fs.h>
# include <linux/types.h>
# include <linux/gpio.h>
# include <linux/time.h>
# include <linux/delay.h> //msleep
# include <linux/errno.h>
# include <asm/uaccess.h> //copy_to_user and copy from user


/* build a module blink based on dif letters */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xu, Du");
MODULE_DESCRIPTION("A MorseCode Transmitter for RPI GPIO 17");



/* function prototype */
int init_module(void);
void cleanup_module(void);

static void    relax(void);
static void    dot(void);
static void    dash(void);
static ssize_t device_read(struct file* flip, const char* buff, size_t count, loff_t* fpos);
static ssize_t device_write(struct file* flip, const char* buff, size_t count, loff_t* fpos);
static int     device_open(struct inode *inode, struct file *file);
static int     device_release(struct inode *inode, struct file *file);


#define DEVICE_NAME "MorseToLed"
#define SUCCESS 0
#define CAPACITY 256
#define gpioLED 17
#define TIME 1000



//static int CAPACITY=256;
//static unsigned int gpioLED=17;
static int Major;
static int Device_Open=0;
static char  message[CAPACITY];
char* messageBuffer;
//static int TIME= 1000;
static int SLEEP_TIME = 3*TIME;
static int DASH_WAIT_TIME = TIME*1;
static int DOT_WAIT_TIME = TIME*2;
static int length;
//int cap;
//module_param(cap,int,0000);



struct file_operations fops={
  .read    =    device_read,     /* read from driver, copy to user  */
  .write   =    device_write,    /* copy from user, write to driver */
  .open    =    device_open,
  .release =    device_release
};


//static char*    what="init";
//static unsigned howmany=1;
//module_param(what,charp, S_IRUGO);
//module_param(howmany, uint, S_IRUGO);


static void relax(void){
  msleep(SLEEP_TIME);
}

static void dot(void){
  gpio_set_value(17,0);
  msleep(DOT_WAIT_TIME);
}
static void dash(void){
  gpio_set_value(17,1);
  msleep(DASH_WAIT_TIME);
}


int init_module(void){
  Major= register_chrdev(0,DEVICE_NAME,&fops);
  if(Major<0){
    printk(KERN_ALERT "Registering char device failed with %d\n ",Major);
    return Major;
  }


  //allocate space for message
  messageBuffer=kmalloc(CAPACITY,GFP_KERNEL);
  memset(messageBuffer,0,CAPACITY);
  length=0;
  			
  printk(KERN_INFO "I was assigned to major number %d. To talk to \n", Major);
  printk(KERN_INFO "the driver, create a dev file with\n");
  printk(KERN_INFO  " 'mknod /dev/%s c %d 0' .\n", DEVICE_NAME, Major);
  printk(KERN_INFO  "Remove the device file and module when done. \n");
  if(!gpio_is_valid(gpioLED)){
    printk(KERN_INFO "GPIO_TEST: invalid LED GPIO \n");
    return -ENODEV;
  }
  //get ready gpio17 pin
  gpio_request(17,"sysclass");
  // gpio_direction_input(gpioLED);
  gpio_direction_output(17,1);
  //gpio_set_value(gpioLED,1);
  //gpio_export(gpioLED,true);

  
  return SUCCESS;
}

void cleanup_module(void){
  printk(KERN_INFO "suucessfully unload kernel module \n");
  gpio_free(17);
  unregister_chrdev(Major, DEVICE_NAME);
}

/* read from device, copy data to user space */
static ssize_t device_read(struct file* flip, const char* buff, size_t count, loff_t* fpos){
  printk(KERN_INFO "im in devcire read\n");
  char driver_buffer[1024];
  char* driver_ptr=driver_buffer;
  memset(driver_buffer,0,1024);
  /* already reach end */
  if(*fpos>=length) return 0;
  if(count>CAPACITY) count=CAPACITY;

  copy_to_user( &buff , driver_buffer + *fpos , count);


  printk(KERN_INFO "\n %s \n",driver_buffer);
  *fpos = *fpos + count; //move to appropriate position of device
  return count;

}

//write to device, move data from user space to kernel space */
static ssize_t device_write(struct file* flip, const char* buff, size_t count, loff_t* fpos){
  printk(KERN_INFO "IM IN DEVICE WRITE\n");
 
   
  if(*fpos >= CAPACITY){
    printk(KERN_INFO "buffer full\n");
    return -ENOSPC;
  }
  /*
  if(count>CAPACITY) 
    count=CAPACITY;
  if(count>length - *fpos) 
    count = length - *fpos;
  */
  memset(messageBuffer,0,CAPACITY);
  int retval= copy_from_user(messageBuffer,buff,count);
  printk(KERN_INFO "copy returned %d we read %s\n",retval,messageBuffer);   
  
  /*    
  char container[2*CAPACITY];

  char morseCode[CAPACITY];
  int i=0;
  int j=0;

  
  for(i=*fpos;i<*fpos,count;i++){
    morseCode[j]=messageBuffer[i];
    j++;  
  }
  
  morseCode[j]='\0';
  
  sscanf(morseCode, "%s", container);
  
  strcpy(message,container);
  printk(KERN_INFO "%s\n",message);
  */
 int i=0;
  while(i<count-1){
    printk(KERN_INFO "blinking letter: %c \n", messageBuffer[i]);

    switch(messageBuffer[i]){
      case 'a': case 'A': case'b': case'B': case'c': case'C': case'd': case'D': case'e': case 'E': case 'f': case'F': case 'g': case'G':
         dot();dot();dash();dash();dot();
         break;
      case 'h': case 'H': case 'i': case 'I' : case 'j': case 'J': case 'k': case 'K' : case 'l' : case 'L' : case 'm': case 'M': case'n': case 'N':
         dash();dash();dot();dot();dash();
         break;
      case 'o': case 'O': case 'p': case 'P': case 'q': case 'Q': case 'r' : case 'R': case's': case'S': case 'T': case 't':
	       dash();dot();dash();dot();dot();
	       break;
      case 'u':case 'U':case 'v':case 'V':case 'w':case 'W':
        dot();dash();dot();dash();dash();
	       break;
      case 'x':case 'X':case 'y':case 'Y':case 'z':case 'Z':
        dot();relax();
	break;
      default:
        printk(KERN_INFO " Chracter is not defined \n");
    }
    i++;
  }
  return count;
}


static int device_open(struct inode *inode, struct file *file)
{
  printk(KERN_INFO "im in device open\n");
  static int counter = 0;

  if (Device_Open)
    return -EBUSY;

  Device_Open++;
  //sprintf(msg, "I already told you %d times Hello world!\n", counter++);
  //msg_ptr = msg;
  try_module_get(THIS_MODULE);
  return SUCCESS;
}
  
static int device_release(struct inode *inode, struct file *file)
{
  printk(KERN_INFO "im in device release\n");
  Device_Open--;    /* We're now ready for our next caller */
  gpio_free(17);
  /* 
   * Decrement the usage count, or else once you opened the file, you'll
   * never get get rid of the module. 
   */
  module_put(THIS_MODULE);

  return 0;
}













  
    
  
  


  


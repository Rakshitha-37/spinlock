#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fcntl.h> /*Helps fix O_ACCMODE*/
#include <linux/sched.h> /*Helps fix TASK_UNINTERRUPTIBLE */
#include <linux/fs.h> /*Helps fix the struct intializer */

#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>

#include <linux/proc_fs.h>    // proc file system
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/proc_ns.h>

#include <linux/kobject.h>

#include <linux/interrupt.h>

#include <linux/kthread.h> // therad 
#include <linux/delay.h>   // sleep 
#include <linux/sched.h>  
#include<linux/spinlock.h>

MODULE_LICENSE("GPL");

DEFINE_SPINLOCK(spinlock);

int  my_thread_handle(void *p);
int my_thread_handle2(void *p);

int count =0;
dev_t mydev;
struct task_struct *char_thread;
struct task_struct *char_thread2;

int my_thread_handle(void *p)
{
	while(!kthread_should_stop())
	{
	        spin_lock(&spinlock);
		printk("my thread1 function %d",count++);
		spin_unlock(&spinlock);
		msleep(1000);
	}
	return 0;
}

int my_thread_handle2(void *p)
{
	while(!kthread_should_stop())
	{
	        spin_lock(&spinlock);
		printk("my thread2 function %d",count++);
		spin_unlock(&spinlock);
		msleep(1000);
	}
	return 0;
}

struct cdev *my_cdev;
static struct class *dev_class;
#define CHAR_DEV_NAME "my_Char_driver"

static int char_dev_open(struct inode *inode, struct file *file)
{
	printk("opening my char driver");
	return 0;
}

static int char_dev_release(struct inode *inode, struct file *file)
{
	printk("closing my char driver");
	return 0;
}

static ssize_t char_dev_write(struct file *file, const char *buf, size_t lbuf, loff_t *ppos)
{
	printk("wring data in my char driver");
	return 0;
} 

static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.write = char_dev_write,
	.open = char_dev_open,
	.release = char_dev_release
};

static int __init my_thread_driver_init(void)
{
	
	int ret =0;
	printk("mt driver function init");

	if (alloc_chrdev_region (&mydev, 0, count, CHAR_DEV_NAME) < 0) {
            printk (KERN_ERR "failed to reserve major/minor range\n");
            return -1;
    	}	
	//Macros that extract the major and minor numbers from a device number
	printk("My major no is = %d my minor no is %d ", MAJOR(mydev),MINOR(mydev));	


	//Functions for the management ofcdevstructures, which represent char deviceswithin the kernel
	my_cdev= cdev_alloc();  // allow allocate cdev instance, 
	cdev_init(my_cdev,&char_dev_fops);  

	ret=cdev_add(my_cdev,mydev,count);// register with VFS layer , count = how many minor no. in use .
	
	if( ret < 0 ) {
		printk("Error registering device driver\n");
		return ret;
	}
	printk(KERN_INFO"\nDevice Registered %s\n",CHAR_DEV_NAME); 
	
	if((dev_class = class_create(THIS_MODULE,"my_Char_class")) == NULL){
		printk(KERN_INFO"Cannot create the struct class for device");
		
	}
	if((device_create(dev_class,NULL,mydev,1,"my_device_file"))==NULL) {
		printk(KERN_INFO" cannot create the device\n");
		
	}

	printk(KERN_INFO"character drivre init sucess\n");
	char_thread = kthread_run(my_thread_handle, NULL, "thread1");

	if(char_thread)
	{
		printk("thread created successfully");
	}
	else
	{
		printk("thread created failed");
		 
	}
	char_thread2 = kthread_run(my_thread_handle2, NULL, "thread2");
	if(char_thread2)
	{
		printk("thread2 created successfully");
	}
	else
	{
		printk("thread created failed");
		 
	}
	return 0;
}

static void __exit my_therad_driver_exit(void)
{

	printk("exit hello world\n");

	kthread_stop(char_thread);
	kthread_stop(char_thread2);
}
module_init(my_thread_driver_init);
module_exit(my_therad_driver_exit);

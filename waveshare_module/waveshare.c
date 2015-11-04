
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include "waveshare.h"


MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("Driver for the Waveshare 4.3 e-paper display");
MODULE_AUTHOR ("Anna-Lena Marx");



#define DEBUG

#ifdef DEBUG
//#define PRINT(msg) 	printk(KERN_INFO "waveshare - %s \n", msg)
#endif // DEBUG

#define WAVESHARE "waveshare"


static dev_t waveshare_dev_number = 0; //Devicenumber
static struct cdev *waveshare_obj = NULL; //Driverobject
struct class *waveshare_class = NULL; //Class for sysfs
static struct device *waveshare_dev = NULL;

static atomic_t bytes_to_write = ATOMIC_INIT(0); //how much bytes are available for writing?
static atomic_t bytes_available = ATOMIC_INIT(0); // how much are available for reading?

#define READ_POSSIBLE (atomic_read(&bytes_available) != 0)
#define WRITE_POSSIBLE (atomic_read(&bytes_to_write) != 0)



module_init (waveshare_init);
module_exit (waveshare_exit);


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = waveshare_driver_open,
	.release = waveshare_driver_close,
	.read =  waveshare_driver_read,
	.write = waveshare_driver_write,
	//do i need poll? .poll = ;
};


static int __init waveshare_init (void) {
	
	if (alloc_chrdev_region (&waveshare_dev_number, 0, 1, WAVESHARE) < 0 ) {
		return -EIO; // definition
	}
	
	waveshare_obj = cdev_alloc();
	
	if (waveshare_obj == NULL) {
		goto free_device_number;
	}

	waveshare_obj->owner = THIS_MODULE;
	waveshare_obj->ops = &fops;

	if (cdev_add (waveshare_obj, waveshare_dev_number,1)) {
		goto free_cdev;
	}

	waveshare_class = class_create (THIS_MODULE, WAVESHARE);
	
	if (IS_ERR (waveshare_class)) {
	//	PRINT (sysfs class creation failed, no udev support);
		goto free_cdev;
	}

	// init powermanagement

  	waveshare_dev = device_create (waveshare_class, NULL, waveshare_dev_number, NULL, "%s", WAVESHARE);
	
	return 0;
	
	
free_cdev:
	//PRINT (adding cdev failed);
	kobject_put (&waveshare_obj->kobj);

free_device_number:
	//PRINT (unregister_chrdev_region (waveshare_dev_number, 1);
	return -EIO;	
}


static void __exit waveshare_exit (void) {

	device_destroy (waveshare_class, waveshare_dev_number);
	class_destroy (waveshare_class);
	cdev_del (waveshare_obj);
	unregister_chrdev_region (waveshare_dev_number, 1);
}



static int waveshare_driver_open (struct inode *devicefile, struct file *driverinstance) {
	return 0;
}

static int waveshare_driver_close (struct inode *devicefile, struct file *driverinstance) {
	return 0;
}

ssize_t waveshare_driver_read (struct file *driverinstance, char __user *buffer, size_t max_bytes_to_read, loff_t *offset) {
}

ssize_t waveshare_driver_write (struct file *driverinstance, const char __user *buffer, size_t max_bytes_to_write, loff_t *offset) {
}

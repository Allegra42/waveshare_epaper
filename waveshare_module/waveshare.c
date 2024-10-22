#include <linux/fs.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "waveshare.h"


MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("Driver for the Waveshare 4.3 e-paper display");
MODULE_AUTHOR ("Anna-Lena Marx");



#define DEBUG

#ifdef DEBUG
#define PRINT(msg) 	do { printk(KERN_INFO "waveshare - %s \n", msg); } while (0)
#endif // DEBUG

#define WAVESHARE "waveshare"
#define TTY_NAME "ttyWAV"
#define DEVICENAME "waveshare_epaper"


static unsigned int resetPin = 49; //P9_23
static unsigned int wakeupPin = 115; //P9_27

static dev_t waveshare_dev_number = 0; //Devicenumber
static struct cdev *waveshare_obj = NULL; //Driverobject
struct class *waveshare_class = NULL; //Class for sysfs
static struct device *waveshare_dev = NULL; //Device object
static wait_queue_head_t wq_read;
static wait_queue_head_t wq_write; // read/write waitqueues


//Match uart port to driver
static const struct of_device_id waveshare_uart_of_ids[] = {
	{ .compatible = "ti,omap3-uart" ,},
	{ .compatible = "ti,am3352-uartti" ,},
	{ },
};
MODULE_DEVICE_TABLE(of, waveshare_uart_of_ids);


//Structs for UART
static struct uart_driver waveshare_uart_driver = {
	.owner = THIS_MODULE,
	.driver_name = "waveshare",
	.dev_name = DEVICENAME,
	.nr =  1,
	.cons = NULL,
};


static struct platform_driver waveshare_serial_driver = {
	.probe = waveshare_uart_probe,
	.remove = waveshare_uart_remove,
//	.suspend = waveshare_serial_suspend,
//	.resume = waveshare_serial_resume,
	.id_table = waveshare_uart_of_ids,
	.driver = {
		.name = "waveshare_uart",
		.owner = THIS_MODULE,
		.of_match_table = waveshare_uart_of_ids,
	},
};

struct waveshare_uart_port {
	struct uart_port port;
// 	unsigned int min_baud;
//	unsigned int max_baud;
//	struct clk *clk;
};

static struct uart_ops waveshare_uart_ops = {
	.startup = waveshare_uart_startup,
};

//TODO at functional driver, this are just start values
static atomic_t bytes_to_write = ATOMIC_INIT(10); //how much bytes are available for writing?
static atomic_t bytes_available = ATOMIC_INIT(5); // how much are available for reading?
static atomic_t access_counter = ATOMIC_INIT(-1); // open device just for one writing instance 

#define READ_POSSIBLE (atomic_read(&bytes_available) != 0)
#define WRITE_POSSIBLE (atomic_read(&bytes_to_write) != 0)


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = waveshare_driver_open,
	.release = waveshare_driver_close,
	.read =  waveshare_driver_read,
	.write = waveshare_driver_write,
 	.poll = waveshare_driver_poll, 
};


module_init (waveshare_init);
module_exit (waveshare_exit);


//read with sysfs from kernel space
static ssize_t waveshare_sysfs_read (struct device *dev, struct device_attribute *attr, char *buf) {
	//read some useful data
	char example [] = {"TEST"};
	return sprintf (buf, "%s \n", example); 
}

//write with sysfs in the kernelspace
static ssize_t waveshare_sysfs_write (struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {
	int val = 0;
	PRINT ("write a number");
	
	val = simple_strtoul (buf, NULL, 10);
	printk (KERN_INFO "You wrote %d \n", val);	
	
	return size;
}

static DEVICE_ATTR (wav_sysfs, 0644, waveshare_sysfs_read, waveshare_sysfs_write);



static int __init waveshare_init (void) {

	static bool val = true;

	PRINT ("in waveshare_init");
	
	if (alloc_chrdev_region (&waveshare_dev_number, 0, 1, WAVESHARE) < 0 ) {
		goto free_device_number;
	}
	
	PRINT ("init_chrdev_region success");
	
	waveshare_obj = cdev_alloc();

	if (waveshare_obj == NULL) {
		goto free_device_number;
	}

	waveshare_obj->owner = THIS_MODULE;
	waveshare_obj->ops = &fops;

	if (cdev_add (waveshare_obj, waveshare_dev_number,1)) {
		goto free_cdev;
	}

	PRINT ("waveshare obj before uart_register_driver");
        if (uart_register_driver(&waveshare_uart_driver)) {
		goto free_uart;
	}

	PRINT ("waveshare obj before platform_driver");
	if (platform_driver_register(&waveshare_serial_driver)) {
		goto free_platform;
	}

	waveshare_class = class_create (THIS_MODULE, WAVESHARE);
	
	if (IS_ERR (waveshare_class)) {
		PRINT ("sysfs class creation failed, no udev support");
		goto free_cdev;
	}
	
	// init powermanagement, here would be init for sleep/wakeup

  	waveshare_dev = device_create (waveshare_class, NULL, waveshare_dev_number, NULL, "%s", WAVESHARE);
	if(device_create_file(waveshare_dev, &dev_attr_wav_sysfs)) {
		PRINT ("failed to register file under /dev/waveshare");
	}

	// reset display
	PRINT ("try to reset display");
	gpio_request(resetPin, "sysfs");
	gpio_direction_output(resetPin, val);
	
	gpio_request(wakeupPin, "sysfs");
	gpio_direction_output(wakeupPin, val);

	gpio_set_value(resetPin, !val);
	mdelay(10);
	gpio_set_value(resetPin, val);
	mdelay(500);
	gpio_set_value(resetPin, !val);
	mdelay(3000);
	PRINT("finished reset"); 

	PRINT ("module init seemed to be successful");	

        
	return 0;
        
free_cdev:
	gpio_set_value(resetPin, !val);
	gpio_set_value(wakeupPin, !val);
	gpio_free(resetPin);
	gpio_free(wakeupPin);
	PRINT ("adding cdev failed");
        kobject_put (&waveshare_obj->kobj);

free_platform:
	PRINT ("register_#platform failed");
	platform_driver_unregister(&waveshare_serial_driver);	

free_uart:
	PRINT ("register_uart failed");
	uart_unregister_driver(&waveshare_uart_driver);
        
free_device_number:
	PRINT ("alloc_chrdev_region or cdev_alloc failed");
	unregister_chrdev_region (waveshare_dev_number, 1);
	return -EIO;	
	
}


static void __exit waveshare_exit (void) {

	gpio_set_value(resetPin, false);
	gpio_set_value(wakeupPin, false);
	gpio_free(resetPin);
	gpio_free(wakeupPin);

	uart_unregister_driver(&waveshare_uart_driver);
	platform_driver_unregister(&waveshare_serial_driver);
	device_destroy (waveshare_class, waveshare_dev_number);
	class_destroy (waveshare_class);
	cdev_del (waveshare_obj);
	unregister_chrdev_region (waveshare_dev_number, 1);
	
	PRINT ("module exited");
}


static int waveshare_driver_open (struct inode *devicefile, struct file *driverinstance) {
	// Just one writing instance shall be allowed
	if ( ((driverinstance->f_flags&O_ACCMODE) == O_RDWR) ||
	     ((driverinstance->f_flags&O_ACCMODE) == O_WRONLY) ) {
		
  		// true if result is 0 -> inital -1 -> one instance allowed
		if (atomic_inc_and_test(&access_counter)) {
			return 0;
		}
		// dec counter, cause instance isn't allowed to access driver
		atomic_dec(&access_counter);
		PRINT ("sorry - just one writing instance");
		return -EBUSY;
	}

	return 0;
}


static int waveshare_driver_close (struct inode *devicefile, struct file *driverinstance) {
	// if a writing instance called close, decrement counter so an other instance is able to write
	if ( ((driverinstance->f_flags&O_ACCMODE) == O_RDWR) ||
	     ((driverinstance->f_flags&O_ACCMODE) == O_WRONLY) ) {
		atomic_dec(&access_counter);
	}

	return 0;
}


ssize_t waveshare_driver_read (struct file *driverinstance, char __user *buffer, size_t max_bytes_to_read, loff_t *offset) {

        // for real here no fixed values, how much could be copied should told from device
	size_t to_copy = 0; 
	size_t not_copied = 0; 

	//read from displaybuffer, read here buffercontent which is identical to display output

	char kern_buffer[128]; //correcting to max chars at the display
	kern_buffer[0] = 'H';
	kern_buffer[1] = 'A';
	kern_buffer[2] = 'L';
	kern_buffer[3] = 'L';
	kern_buffer[4] = 'O';
	kern_buffer[5] = '\n';
	
	PRINT ("in the read function");

	// no data available, nonblocking
	if ((!READ_POSSIBLE) && (driverinstance->f_flags&O_NONBLOCK)) {
		PRINT ("failed at reading, no data available, nonblock");
		return -EAGAIN;
	} 
	
	// signal while sleeping
	if (wait_event_interruptible (wq_read, READ_POSSIBLE)) {
		PRINT ("failed at reading, sig while sleep");
		return -ERESTARTSYS;
	}

	to_copy = min ((size_t) atomic_read (&bytes_available), max_bytes_to_read+1);
	not_copied = copy_to_user (buffer, kern_buffer, to_copy);
        
        // for non-statical output
	//atomic_sub ((to_copy - not_copied), &bytes_available);
	//*offset += to_copy - not_copied;
	
	return (to_copy - not_copied);	
}


ssize_t waveshare_driver_write (struct file *driverinstance, const char __user *buffer, size_t max_bytes_to_write, loff_t *offset) {
	size_t to_copy;
	size_t not_copied;
	
	char kern_buffer [56]; // change in real usage to displaysize/displaybuffersize
	
	// writing not possible, nonblocking
	if ((!WRITE_POSSIBLE) && (driverinstance->f_flags&O_NONBLOCK)) {
		PRINT ("failed at writing, not possible, nonblock");
		return - EAGAIN;
	}
	
	if (wait_event_interruptible (wq_write, WRITE_POSSIBLE)) {
		PRINT ("failed at writing, sig while sleep");
		return -ERESTARTSYS;
	}
	
	if (atomic_read(&bytes_to_write) < 1) {
		PRINT ("no bytes to write");
	}

	to_copy = min ((size_t) atomic_read(&bytes_to_write), max_bytes_to_write);
	printk (KERN_INFO "waveshare - write - to copy = %d ", to_copy);
	
	not_copied = copy_from_user (kern_buffer, buffer, to_copy);
	printk (KERN_INFO "waveshare - write - not_copied = %d ", not_copied);

	//write to display, here should be a call to write the buffer as display output
	
	printk (KERN_INFO "waveshare - you wrote %s to driver \n" , kern_buffer);

        //For non-statical input
	//atomic_sub ((to_copy - not_copied), &bytes_to_write);
	//*offset += (to_copy - not_copied);

	return (to_copy - not_copied);
}


unsigned int waveshare_driver_poll (struct file *driverinstance, struct poll_table_struct *event_list) {
	unsigned int mask = 0;
	
	poll_wait (driverinstance, &wq_read, event_list);
	poll_wait (driverinstance, &wq_write, event_list);

	if (READ_POSSIBLE) {
		mask |= POLLIN | POLLRDNORM;
	}
	if (WRITE_POSSIBLE) {
		mask |= POLLOUT | POLLWRNORM;
	}

	return mask;
}


static int waveshare_uart_probe (struct platform_device *pdev) {
	
	struct waveshare_uart_port *wav_port;
	struct uart_port *port;
	struct resource *mem_res;
	unsigned int baud;

	PRINT (" in waveshare_uart_probe");
		
	wav_port = devm_kzalloc (&pdev->dev, sizeof (struct waveshare_uart_port), GFP_KERNEL);
	if (!wav_port) {
		return -EINVAL;
	}

	port = &wav_port->port;
	
	mem_res = platform_get_resource (pdev, IORESOURCE_MEM, 0);
	port->membase = devm_ioremap_resource(&pdev->dev, mem_res);
	if (IS_ERR(port->membase)) {
		return PTR_ERR(port->membase);
	}

	port->mapbase = mem_res->start;
	port->dev = &pdev->dev;

	port->ops = &waveshare_uart_ops;

	baud = 115200;

	if (uart_add_one_port(&waveshare_uart_driver, &wav_port->port)) {
		goto free_uart_add_one_port;
	}
	
	platform_set_drvdata(pdev, wav_port);
	
	return 0;

free_uart_add_one_port:
	PRINT ("failed add uart_add_one_port");
	return -1;	

}


static int waveshare_uart_remove (struct platform_device *pdev) {

	struct waveshare_uart_port *wav_port;

	wav_port = platform_get_drvdata(pdev);

	if (wav_port) {
		uart_remove_one_port(&waveshare_uart_driver, &wav_port->port);
	}
	
	return 0;
}

//Prototypes for UART communication methods
static inline unsigned int waveshare_uart_read (struct waveshare_uart_port *up, int offset) {
	return readb (up->port.membase + offset);
}

static inline void waveshare_uart_write (struct waveshare_uart_port *up, int offset, char value) {
	return writeb (value, up->port.membase + offset);
}

static int waveshare_uart_startup (struct uart_port *port) {

	struct waveshare_uart_port *wave_port = container_of (port, struct waveshare_uart_port, port);
	
	PRINT ("in waveshare_uart_startup");
	
	waveshare_uart_write (wave_port, 0x00, 0xA5);
	waveshare_uart_write (wave_port, 0x00, 0x00);
	waveshare_uart_write (wave_port, 0x00, 0x09);
	waveshare_uart_write (wave_port, 0x00, 0x2E);
	waveshare_uart_write (wave_port, 0x00, 0xCC);
	waveshare_uart_write (wave_port, 0x00, 0x33);
	waveshare_uart_write (wave_port, 0x00, 0xC3);
	waveshare_uart_write (wave_port, 0x00, 0x3C);
        waveshare_uart_write (wave_port, 0x00, 0x82);
		
	return 0;
}

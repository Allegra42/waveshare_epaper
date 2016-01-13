

#ifndef __waveshare_H__
#define __waveshare_H__

static int __init waveshare_init (void);
static void __exit waveshare_exit (void);

static int waveshare_driver_open (struct inode *devicefile, struct file *driverinstance);
static int waveshare_driver_close (struct inode *devicefile, struct file *driverinstance);
ssize_t waveshare_driver_read (struct file *driverinstance, char __user *buffer, size_t max_bytes_to_read, loff_t *offset);
ssize_t waveshare_driver_write (struct file *driverinstance, const char __user *buffer, size_t max_bytes_to_write, loff_t *offset);
unsigned int waveshare_driver_poll (struct file *driverinstance, struct poll_table_struct *event_list);

static int waveshare_uart_probe (struct platform_device *pdev);
static int waveshare_uart_remove (struct platform_device *pdev);

static int waveshare_uart_startup (struct uart_port *port);

#endif

KERNEL_DIR=~/Development/linux-stable

obj-$(CONFIG_WAVESHARE) += waveshare.o
obj-m := waveshare.o

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD)

modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean: 
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean

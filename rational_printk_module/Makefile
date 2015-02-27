obj-m := rational_printk.o
all:
	make -C /usr/src/kernels/$(shell uname -r) SUBDIRS=$(PWD) modules

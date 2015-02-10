obj-m := printk_testing.o
all:
	make -C /usr/src/kernels/$(shell uname -r) SUBDIRS=$(PWD) modules

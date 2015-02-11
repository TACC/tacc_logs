obj-m := rational_libcfs_debug_msg.o
ccflags-y := -I/usr/src/lustre-2.6.94/libcfs/include
ccflags-y += -DCDEBUG_ENABLED

all:
	make -C /usr/src/kernels/$(shell uname -r) -f /usr/src/lustre-2.6.94/build/Makefile SUBDIRS=$(PWD) modules

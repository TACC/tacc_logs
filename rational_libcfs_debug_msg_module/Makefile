obj-m := rational_libcfs_debug_msg.o
ccflags-y := -I/home/rtevans/lustre-release/libcfs/include
ccflags-y += -DCDEBUG_ENABLED

all:
	make -C /usr/src/kernels/$(shell uname -r) SUBDIRS=$(PWD) modules

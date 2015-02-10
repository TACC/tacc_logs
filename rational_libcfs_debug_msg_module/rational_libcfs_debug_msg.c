#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ctype.h>
#include "libcfs/libcfs.h"

int ins_libcfs_debug_msg(struct libcfs_debug_msg_data *msgdata, const char *format, ...) 
{

  va_list args;
  int rc;

  va_start(args,format);
  rc = libcfs_debug_vmsg2(msgdata, format, args, NULL);
  printk(format, args);
  va_end(args);

  jprobe_return();
  return rc;
}

static struct jprobe my_jprobe = {
	.kp.addr = (kprobe_opcode_t *) libcfs_debug_msg,
	.entry = (kprobe_opcode_t *) ins_libcfs_debug_msg
};

int init_module(void)
{
	register_jprobe(&my_jprobe);
	printk("plant libcfs_debug_msg jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

void cleanup_module(void)
{
  unregister_jprobe(&my_jprobe);
  printk("printk jprobe unregistered\n");

}

MODULE_LICENSE("GPL");

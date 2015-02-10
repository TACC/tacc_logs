#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

int hello_init(void) {
  int a = 0;
  char *b = "looks-good";
  
  printk(KERN_INFO "format string for data: %d %s",a,b);

  return 0;

}

void hello_exit(void) {

  printk("awesome finalization");

}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");

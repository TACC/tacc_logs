#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

void test_pV(const char *fmt, ...) 
{
  va_list args;
  struct va_format vaf;
  va_start(args,fmt);
  vaf.fmt = fmt;
  vaf.va = &args;
  printk(KERN_INFO "trying out pV: %pV",&vaf);
}

int hello_init(void) {
  int a = 0;
  char *b = "looks-good";
  
  printk(KERN_INFO "format string for data: %d %s",a,b);
  test_pV("%s %d","string" "-test",111);
  
  return 0;

}

void hello_exit(void) {

  printk("%s awesome finalization\n",KERN_INFO);

}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");

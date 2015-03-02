#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kprobes.h>
#include <linux/ctype.h>
#define LOG_LINE_MAX     1024

static int JOBID = 0;
module_param(JOBID, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(JOBID, "The current jobid");

static int rpk_encode2(char *buf, size_t size, 
		       const char *fmt, va_list args);

static char *rpk_encode_args(char *pos, char *end, int *key, const char *fmt,
			     va_list args);

static asmlinkage int rprintk(const char *fmt, ...) 
{
  
  static char msg[LOG_LINE_MAX];
  va_list args;
  int r;

  va_start(args,fmt);
  r =  rpk_encode2(msg, sizeof(msg), fmt, args);
  printk(KERN_INFO "JobID %d %s\n",JOBID,msg);
  va_end(args);

  jprobe_return();
  return 0;
}

static int rpk_skip_atoi(const char **s)
{
  int i = 0;
  while(isdigit(**s))
    i = i * 10 + *((*s)++) - '0';

  return i;
}

#define rpk_put(pos, end, c)			\
  do {						\
    int _c = (c);				\
    if (pos < end)				\
      *(pos++) = _c;				\
  } while(0)

static char *rpk_num( char *pos, char *end, unsigned long long num, int sign)
{
  char tmp[66];
  int i;
  if (sign) {
    if ((signed long long) num < 0) {
      rpk_put(pos, end, '-');
      num = -(signed long long) num;
    }
  }

  i = 0;
  if (num == 0) {
    tmp[i++] = '0';
  } else {
    while (num != 0)
      tmp[i++] = '0' + do_div(num, 10);
  }

  while (i-- > 0)
    rpk_put(pos, end, tmp[i]);

  return pos;
}

static inline char *rpk_key(char *pos, char *end, int *key)
{
  return rpk_num(pos, end, (*key)++, 1);
}

static inline char *rpk_str(char *pos, char *end, const char *str, int prec)
{
  for (; prec > 0 && *str != 0; prec--, str++) {
    if (*str == '\n') continue;

#ifdef KERN_SOH_ASCII
    if (*str != KERN_SOH_ASCII) 
      rpk_put(pos, end, *str);
    else {
      str++;
      rpk_put(pos, end, '<');
      rpk_put(pos, end, *(str++));
      rpk_put(pos, end, '>');
      return pos;
    }
#else
    rpk_put(pos, end, *str);
#endif
  }
  return pos;
}

/* Encode integer argument.  Format is "<key>:<argument-in-decimal>\0" */
static char *rpk_encode_num(char *pos, char *end, int *key,
			    unsigned long long num, int sign)
{
  pos = rpk_key(pos, end, key);
  rpk_put(pos, end, ':');
  rpk_put(pos, end, ' ');
  pos = rpk_num(pos, end, num, sign);
  rpk_put(pos, end, ' ');
  //rpk_put(pos, end, '\n');
  return pos;
}

/* Encode string argument. Format is "<key>:<upto-prec-chars-from-string>\0" */
static char *rpk_encode_str(char *pos, char *end, int *key, const char *str,
			    int prec)
{
  pos = rpk_key(pos, end, key);
  rpk_put(pos, end, ':');
  rpk_put(pos, end, ' ');
  pos = rpk_str(pos, end, str, prec);
  //rpk_put(pos, end, '\n');
  rpk_put(pos, end, ' ');
  return pos;
}

static char *rpk_encode_args(char *pos, char *end, int *key, const char *fmt,
			     va_list args)
{
  int prec, qual, sign;
  unsigned long long num;
  const char *str;
  va_list va;
  void *ptr;

  for (; *fmt != 0; ++fmt) {
    if (*fmt != '%')
      continue;
    /* Ignore all flags. */
  repeat:
    ++fmt;		/* This also skips the '%'. */
    switch (*fmt) {
    case '-':
      goto repeat;
    case '+':
      goto repeat;
    case ' ':
      goto repeat;
    case '#':
      goto repeat;
    case '0':
      goto repeat;
    }

    /* Field width. */
    if (isdigit(*fmt)) {	/* Ignore embedded field width. */
      (void)rpk_skip_atoi(&fmt);
    } else if (*fmt == '*') {	/* Specified by next argument. */
      ++fmt;
      pos =
	rpk_encode_num(pos, end, key, va_arg(args, int), 1);
    }

    /* Precision. */
    prec = INT_MAX;
    if (*fmt == '.') {
      ++fmt;
      if (isdigit(*fmt)) {
	prec = rpk_skip_atoi(&fmt);
      } else if (*fmt == '*') {	/* Specified by next argument. */
	++fmt;
	prec = va_arg(args, int);
	pos = rpk_encode_num(pos, end, key, prec, 1);
      }
      if (prec < 0)
	prec = 0;
    }

    /* Conversion qualifier. */
    qual = 0;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
	*fmt == 'Z' || *fmt == 'z' || *fmt == 't') {
      qual = *fmt;
      ++fmt;

      if (qual == 'h' && *fmt == 'h') {
	qual = 'H';
	++fmt;
      }

      if (qual == 'l' && *fmt == 'l') {
	qual = 'L';
	++fmt;
      }
    }

    sign = 0;

    switch (*fmt) {
    case 'c':
      pos =
	rpk_encode_num(pos, end, key, va_arg(args, int), 1);
      continue;

    case 's':
      str = va_arg(args, char *);
      if ((unsigned long)str < PAGE_SIZE)
	str = "<NULL>";
      pos = rpk_encode_str(pos, end, key, str, prec);
      continue;

    case 'p':
      if (*(fmt+1) == 'V') {	
	ptr = va_arg(args, void*);
	va_copy(va, *((struct va_format *)ptr)->va);
	pos = rpk_encode_str(pos, end, key, 
			     ((struct va_format *)ptr)->fmt, prec);
	pos = rpk_encode_args(pos, end, key, 
			      ((struct va_format *)ptr)->fmt, va);
	continue;
      }      
      else
	pos = rpk_encode_num(pos, end, key,
			     (unsigned long)va_arg(args,
						   void *), 0);
      continue;

    case 'n':	/* Ignore. */
      (void)va_arg(args, void *);
      continue;

    case '%':
      continue;

      /* Integer number formats. */
    case 'd':
    case 'i':
      sign = 1;
    case 'o':
    case 'u':
    case 'x':
    case 'X':
      if (qual == 'L') {
	num = va_arg(args, long long);
      } else if (qual == 'l') {
	num = va_arg(args, unsigned long);
	if (sign)
	  num = (signed long)num;
      } else if (qual == 'Z' || qual == 'z') {
	num = va_arg(args, size_t);
      } else if (qual == 't') {
	num = va_arg(args, ptrdiff_t);
      } else if (qual == 'h') {
	num = (unsigned short)va_arg(args, int);
	if (sign)
	  num = (signed short)num;
      } else if (qual == 'H') {
	num = (unsigned char)va_arg(args, int);
	if (sign)
	  num = (signed char)num;
      } else {
	num = va_arg(args, unsigned int);
	if (sign)
	  num = (signed int)num;
      }

    pos = rpk_encode_num(pos, end, key, num, sign);
    continue;

    default:	/* This is what the kernel's vsprintf does. */
      if (*fmt == 0)
	goto out;
      continue;
    }
  }
 out:
  return pos;
}

static inline int rpk_get_level(const char *buffer)
{
#ifdef KERN_SOH_ASCII
  if (buffer[0] == KERN_SOH_ASCII && buffer[1]) {
    switch (buffer[1]) {
    case '0' ... '7':
    case 'd':	/* KERN_DEFAULT */
      return buffer[1];
    }
  }
#else
  if (buffer[0] == '<' && buffer[1] && buffer[2] && buffer[2] == '>') {
    switch(buffer[1]) {
    case '0' ... '7':
    case 'd':    
      return buffer[1];
    }
  }
#endif
  return 0;
}
static inline const char *rpk_skip_level(const char *buffer)
{
  int header_len = 0;
#ifdef KERN_SOH_ASCII // New kernel style - 3.x
  header_len = 2;
#else
  header_len = 3;
#endif
  if (rpk_get_level(buffer)) {
    switch (buffer[1]) {
    case '0' ... '7':
    case 'd':	/* KERN_DEFAULT */
      return buffer + header_len;
    }
  }
  return buffer;
}

static int rpk_encode2(char *buf, size_t size, 
		       const char *fmt, va_list args)
{
  int kern_level = 0;
  char rpk_level[3];
  char *pos = buf, *end = buf + size;
  int key = 0;

  if (end < buf) 
    end = (void *)-1;

  /* Get kern level if part of string */
  kern_level = rpk_get_level(fmt);
  if (kern_level) {
    rpk_level[0] = '<', rpk_level[1] = fmt[1], rpk_level[2] = '>';
    fmt = rpk_skip_level(fmt);
  }

  pos = rpk_encode_str(pos, end, &key, fmt, LOG_LINE_MAX); // format string
  if (kern_level) {
    pos = rpk_encode_str(pos, end, &key, rpk_level, 3); // log-level
  }
  pos = rpk_encode_args(pos, end, &key, fmt, args);
  rpk_put(pos, end, '\0');

  return pos - buf;
}


static struct jprobe rpk_jprobe = {
  .entry = rprintk,
  .kp = {
    .symbol_name = "printk",
  },
};

static int __init rpk_init(void)
{
  int ret;

  ret = register_jprobe(&rpk_jprobe);
  if (ret < 0) { 
    printk(KERN_INFO "rational_printk module failed, returned %d\n", ret);
    return -1;
  }
  printk(KERN_INFO "plant rational_printk jprobe at %p, handler addr %p\n",
	 rpk_jprobe.kp.addr, rpk_jprobe.entry);

  return 0;
}

static void __exit rpk_exit(void)
{
  unregister_jprobe(&rpk_jprobe);
  printk(KERN_INFO "rational_printk jprobe at %p unregistered\n", rpk_jprobe.entry);
}

module_init(rpk_init);
module_exit(rpk_exit);
MODULE_LICENSE("GPL");

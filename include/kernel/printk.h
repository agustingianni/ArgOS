#ifndef PRINTK_H_
#define PRINTK_H_

#include <stdarg.h>

int printk(const char *fmt, ...);
int vprintk(const char *fmt, va_list args);

#endif /*PRINTK_H_*/

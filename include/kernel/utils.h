#ifndef UTILS_H_
#define UTILS_H_

#include <kernel/printk.h>
#include <kernel/console.h>
#include <stdarg.h>
#include <lib/string.h>

void panic(const char *fmt, ...);

#endif /*UTILS_H_*/

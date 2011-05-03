#include <kernel/printk.h>
#include <kernel/console.h>
#include <stdarg.h>
#include <lib/string.h>

extern int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);

int
vprintk(const char *fmt, va_list args)
{
	int len;
	char *p;
	static char buffer[1024];

	len = vscnprintf(buffer, sizeof(buffer), fmt, args);

	for (p = buffer; *p; p++)
	{
		putchar(*p);
	}

	return len;
}

int
printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	return r;
}


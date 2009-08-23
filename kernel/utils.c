#include <kernel/utils.h>
#include <kernel/printk.h>
#include <arch/asm.h>

void
panic(const char *fmt, ...)
{
	va_list         ap;
	va_start(ap, fmt);

	vprintk(fmt, ap);
	printk("\n");
	va_end(ap);

	cpu_halt();
}

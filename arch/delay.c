#include <jiffies.h>
#include <arch/timer.h>
#include <arch/cpuid.h>
#include <arch/asm.h>

/*
 * Busy delay that loops until a given number of clock
 * ticks have passed
 */
void
busy_delay_ticks(unsigned long n)
{
	/* FIXME: Integer overflows here */
	unsigned long delay = jiffies + n;
	
	while(time_before(jiffies, delay))
		;
	
	return;
}

/*
 * Now we loop fo N seconds or N*HZ ticks or jiffies 
 */
void
busy_delay_seconds(unsigned long n)
{
	busy_delay_ticks(seconds_to_jiffies(n));
}


/* FIXME: Integer overflows here
 * Micro seconds delay = 1/1.000.000 seconds
 */
void
udelay(unsigned long usecs)
{
    u_int64_t ticks;
    ticks = rdtsc() + (usecs*(cpu_info.frequency/1000));

    while(rdtsc() < ticks);
}

/* FIXME: Integer overflows here
 * Milli seconds delay = 1/1.000
 */
void
mdelay(unsigned long msecs)
{
	udelay(msecs * 1000);
}

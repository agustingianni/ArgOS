#ifndef JIFFIES_H_
#define JIFFIES_H_

#include <kernel/kernel.h>

/*
 * Notes on jiffies:
 *  - Jiffies is the number of clock ticks that have occured
 * 	  since the system booted.
 * 	- There are HZ jiffies each second
 *  - Hence, System uptime is jiffies/HZ seconds.
 */

extern unsigned long volatile jiffies;

#define seconds_to_jiffies(n)		\
	(n * HZ)

#define jiffies_to_seconds(n)		\
	(n/HZ)

#define system_uptime()				\
	(jiffies_to_seconds(jiffies))

/*
 * Handle the jiffies wrap arround, code from Robert Love book 
 * and linux kernel sources.
 */
#define time_after(a,b)				\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(b) - (long)(a) < 0))

#define time_before(a,b)			\
	time_after(b,a)

#define time_after_eq(a,b)			\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(a) - (long)(b) >= 0))

#define time_before_eq(a,b)			\
	time_after_eq(b,a)

#endif /*JIFFIES_H_*/

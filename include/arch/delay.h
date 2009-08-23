#ifndef DELAY_H_
#define DELAY_H_

void busy_delay_ticks(unsigned long n);

void busy_delay_seconds(unsigned long n);

/* FIXME: Integer overflows here
 * Micro seconds delay = 1/1.000.000 seconds
 */
void udelay(unsigned long usecs);

/* FIXME: Integer overflows here
 * Milli seconds delay = 1/1.000
 */
void mdelay(unsigned long msecs);

#endif


#include <arch/timer.h>
#include <arch/irq.h>
#include <jiffies.h>
#include <kernel/scheduler.h>
#include <arch/cpuid.h>
#include <arch/msr.h>
#include "../include/kernel/printk.h"

/*
 * @doc: http://download.intel.com/design/archives/periphrl/docs/23124406.pdf
 *
 * The PIT provides three independent 16-bit counters,
 * each capable of handling clock inputs up to 10 MHz.
 *
 * Six programmable timer modes allow the 82C54 to be used
 * as an event counter, elapsed time indicator,
 * programmable one-shot, and in many other applications.
 */

void
timer_handler(icontext_t *context)
{
	/* TODO: jiffies must be 64 bits long */
	jiffies++;

	/*
	 * There are two process times, one for
	 * user process an another for kernel
	 * the user_mode macro retusn 0 for system and
	 * 1 for user mode interrupts
	 */

	/* update_process_times(user_mode(context)); */

    irq_send_eoi((context->int_no - 32));

    /* Funcion que calcula los Q de los procesos */
    sched_tick();

	return;
}

void
init_timer(void)
{
	/*
	 * The Control Word specifies which Counter is being
	 * programmed.
	 *
	 * Initial counts are written into the Counters
	 *
	 * Programming:
	 *
	 * 1. For each Counter, the Control Word must be
	 * written before the initial count is written.
	 *
	 * 2. The initial count must follow the count format
	 * specified in the Control Word (least significant
	 * byte only, most significant byte only, or least significant
	 * byte and then most significant byte).
	 */

	printk("[Init] Initializing PIT\n");

	int iflag = begin_atomic();
	{
		/*
		 * Initialize channel 0:
		 * - Square wave generator
		 * - We are going to send LSB + MSB of a int16
		 */
	    outb(PIT_CTRL_WR_ADDR,
	    		PIT_COUNTER_0 |
	    		PIT_CMD_RW_LSB_MSB |
	    		PIT_CMD_MODE_3);

	    /* Send LSB */
	    outb(PIT_COUNTER_0_ADDR,
	    		(unsigned char) (LATCH & 0xff));

	    /* SEND MSB */
	    outb(PIT_COUNTER_0_ADDR,
	    		(unsigned char) (LATCH >> 8));
	}
    end_atomic(iflag);

    /* Register the timer handler */
    irq_set_handler(0, &timer_handler);
}

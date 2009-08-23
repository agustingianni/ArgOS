/*
 * softirq.c
 *
 *  Created on: Jun 18, 2008
 *      Author: gr00vy
 */

/*
 * structure representing a single softirq entry
 */
struct softirq_action
{
        void (*action)(struct softirq_action *); /* function to run */
        void *data;                              /* data to pass to function */
};

static struct softirq_action softirq_vec[32];

void
softirq_consume(void)
{

}

void
softirq_register(long id,  void (*action)(struct softirq_action *), void *data)
{

}

void
softirq_schedule(long id)
{

}

/*
 * Kernel Thread that handles pending softirq's.
 */
void
softirq_daemon(void)
{

}

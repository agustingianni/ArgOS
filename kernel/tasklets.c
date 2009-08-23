/*
 * tasklets.c
 *
 *  Created on: Jun 18, 2008
 *      Author: gr00vy
 */

struct tasklet_struct
{
        struct tasklet_struct *next;  /* next tasklet in the list */
        unsigned long state;          /* state of the tasklet */
        atomic_t count;               /* reference counter */
        void (*func)(unsigned long);  /* tasklet handler function */
        unsigned long data;           /* argument to the tasklet function */
};

#define TASKLET_STATE_SCHED 0   /* Scheduled to run */
#define TASKLET_STATE_RUN 1     /* Running */


void
tasklet_schedule(struct tasklet_struct *tasklet)
{

}

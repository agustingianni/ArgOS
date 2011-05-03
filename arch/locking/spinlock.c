#include <stdio.h>
//#include <locking/spinlock.h>
#define SPINLOCK_LOCKED   0
#define SPINLOCK_UNLOCKED 1

typedef struct spinlock
{
	unsigned int spinlock;
} spinlock_t;

void spinlock_init    (spinlock_t *s);
int  spinlock_locked  (spinlock_t *s);
int  spinlock_acquire (spinlock_t *s);
int  spinlock_release (spinlock_t *s);

/* Set the spin lock to 1 (unlocked) */
void
spinlock_init(spinlock_t *s)
{
	s->spinlock = SPINLOCK_UNLOCKED;
}

/*  */
int
spinlock_locked(spinlock_t *s)
{
	/*
	 * the spinlock value is subject to sudden changes so
	 * skip the possible optimizations by casting it to volatile
	 */
	return *(volatile signed char *)(&(s)->spinlock) <= SPINLOCK_LOCKED;
}

/* Trata de obtener el spinlock, retorna  */
int
spinlock_try_acquire(spinlock_t *s)
{
    char oldval;
    asm volatile
    (
            "xchgb %b0,%1"
            : "=q" (oldval), "+m" (s->spinlock)
            :"0" (0) : "memory"
    );

    return oldval > 0;
}

/* Cycle until spin lock becomes 1 (unlocked), then set it to 0 (locked) */
int
spinlock_acquire(spinlock_t *s)
{
	asm volatile
	(
		"1:\t"
		LOCK_PREFIX " ; decb %0\n\t"
		"jns 3f\n"			/* If it is clear -> spinlock was unlocked (1) */
		"2:\t"				/* spinlock was locked */
		"rep;nop\n\t"
		"cmpb $0,%0\n\t"	/* lock->slock == SPINLOCK_LOCKED ? */
		"jle 2b\n\t"
		"jmp 1b\n"
		"3:\n\t"			/* now we have the spinlock, return */
		: "+m" (s->spinlock) : : "memory"
	);
}

/* Set the spin lock to 1 (unlocked) */
int
spinlock_release(spinlock_t *s)
{
	char oldval = SPINLOCK_UNLOCKED;

	asm volatile
	(
		"xchgb %b0, %1"
		: "=q" (oldval), "+m" (s->spinlock)
		: "0" (oldval) : "memory"
	);

	preemption_enable();
}

/* This will disable interrupts locally and provide the spinlock on SMP. */
int
spinlock_acquire_irqsave(spinlock_t *s, unsigned long flags)
{

}

/* This will enable interrupts and restore the cflags */
int
spinlock_release_irqrestore(spinlock_t *s, unsigned long flags)
{

}

#ifdef UNIT_TEST
int
main(int argc, char **argv)
{
	spinlock_t slock;

	spinlock_init(&slock);

	spinlock_acquire(&slock);

	if(spinlock_try_acquire(&slock) == 0)
	{
		printf("Spinlock not acquired\n");
	}
	else
	{
		printf("Spinlock aquired\n");
	}

	spinlock_release(&slock);
}
#endif

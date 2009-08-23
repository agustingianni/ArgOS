#ifndef SPINLOCK_H_
#define SPINLOCK_H_

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

#endif /*SPINLOCK_H_*/

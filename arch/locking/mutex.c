#include <locking/atomic.h>
#include <locking/spinlock.h>
#include <locking/mutex.h>

/*
 * A mutex, mutex lock, or sleep lock, is similar to a spinlock, 
 * except that instead of constantly polling, it places itself 
 * on a queue of threads waiting for the lock, then yields the 
 * remainder of its time quantum. It does not execute again 
 * until the thread holding the lock wakes it (or in some user 
 * space variations, until an asynchronous signal arrives).
 * 
 * Because mutexes are based on blocking, they can only be used
 * in places where blocking is allowed. For this reason, mutexes
 * cannot be used in the context of interrupt handlers. 
 */

#define MUTEX_LOCKED   0
#define MUTEX_UNLOCKED 1

typedef struct mutex
{
	/* 1: unlocked, 0: locked, negative: locked, possible waiters */
	atomic_t			count;
	spinlock_t			wait_lock;
	struct list_head	wait_list;
} mutex_t;

/* Set the mutext to MUTEX_UNLOCKED state */
void
mutex_init(mutex_t *s)
{
	atomic_set(&lock->count, MUTEX_UNLOCKED);
	spinlock_init(&lock->wait_lock);
	INIT_LIST_HEAD(&lock->wait_list);
}

/* Returns 1 if the mutex is locked, 0 otherwise. */
int
mutex_locked(mutex_t *s)
{
	return atomic_read(&s->count) != MUTEX_UNLOCKED;
}

/* Cycle until spin lock becomes 1 (unlocked), then set it to 0 (locked) */
int
mutex_acquire(mutex_t *s)
{
	asm volatile
	(
		LOCK_PREFIX "decl (%%eax)\n"
		"jns 1f\n"
		"call "#fail_fn"	\n"
		"1:			\n"
		:"=a" (dummy)
		: "a" (count)
		: "memory", "ecx", "edx"
	);

}

/* Set the spin lock to 1 (unlocked) */
int
mutex_release(mutex_t *s)
{

}

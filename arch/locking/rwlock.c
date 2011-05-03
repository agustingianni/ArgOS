#include <locking/atomic.h>
#include <locking/rwlock.h>

/*
 * Read-write locks (also called shared-exclusive locks) are somewhat 
 * different from traditional locks in that they are not always exclusive 
 * locks. A read-write lock is useful when shared data can be reasonably 
 * read concurrently by multiple threads except while a thread is modifying
 * the data. Read-write locks can dramatically improve performance if 
 * the majority of operations on the shared data are in the form of 
 * reads (since it allows concurrency), while having negligible impact 
 * in the case of multiple writes.
 * 
 * A read-write lock allows this sharing by enforcing the following constraints:
 * 	Multiple readers can hold the lock at any time.
 *  Only one writer can hold the lock at any given time.
 *  A writer must block until all readers have released the lock before obtaining the lock for writing.
 *  Readers arriving while a writer is waiting to acquire the lock will block until after the writer has obtained and released the lock.
 */

typedef struct read_write_lock
{
	
} rwlock_t;

void
rwlock_init(spinlock_t *s)
{
	
}

int
rwlock_acquire(spinlock_t *s)
{
	
}

int
rwlock_release(spinlock_t *s)
{
	
}

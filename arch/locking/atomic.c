#include <locking/atomic.h>
#include <kernel/printk.h>

void
test_atomic(void)
{
	atomic_t v;
	atomic_set(&v, 5);  /* v = 5 (atomically) */
	atomic_add(3, &v);  /* v = v + 3 (atomically) */
	atomic_dec(&v);     /* v = v - 1 (atomically) */

	printk("This will print 7: %d\n", atomic_read(&v));
}

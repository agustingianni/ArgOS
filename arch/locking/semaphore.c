#include <locking/atomic.h>
#include <locking/semaphore.h>

typedef struct semaphore
{
	atomic_t count;
	atomic_t sleepers;
	wait_queue_head_t wait;
} semaphore_t;

void
semaphore_init(semaphore_t *s, int value)
{
	atomic_set(&s->count, value);
	atomic_set(&s->sleepers, 0);
	init_waitqueue_head(&sem->wait);
}

/* No conviene usarlo a no ser que sea realmente necesario */
void
semaphore_acquire(semaphore_t *s)
{
	
}

/* hay que usar este supuestamente, permite que sea interrumpido */
int
semaphore_acquire_interruptible(semaphore_t *s)
{
	
}

/* Prueba a ver si puede adquirir el semaforo y sino retorna */
int
semaphore_acquire_trylock(semaphore_t *s)
{
	
}

void
semaphore_release(semaphore_t *s)
{
	
}

#include <arch/timer.h>
#include <arch/delay.h>
#include <arch/tss.h>
#include <kernel/scheduler.h>
#include <kernel/printk.h>
#include <kernel/compiler.h>
#include <jiffies.h>
#include <mm/mm.h>
#include <locking/spinlock.h>
#include <bug.h>

/*
 * Process Preemption:
 * Linux lo que hace es cada ves que volvemos del manejador
 * de una Syscall o una Interrupcion checkea el valor
 * de current->need_resched y si esta en TRUE llama a
 * schedule()
 *
 * Durante una interrupcion no podemos schedulear, por que
 * como el manejador de interrupciones no es una task del
 * kernel no tiene contexto ni lugar para guardarlo. Por lo tento
 * no podemos llamar a funciones que puedan "sleepear" ya que
 * no podemos schedulear.
 *
 * En el caso de las syscalls
 *
 * Una ves en schedule, se elije la task que debe entrar a
 * ejecutarse.
 *
 * Kernel Preemption:
 * El kernel puede schedulear entre tareas del kernel
 * siempre y cuando la tarea actual no posea ningun lock.
 * Esto es asi por que si scheduleamos una tarea con un lock
 * y corremos otra que necesita ese lock podemos entrar en un
 * bloqueo mutuo.
 *
 * Como sabemos si la task de kernel tiene algun lock? Le agregamos
 * un contador a la struct que define el thread que nos diga cuantos
 * locks se adquirieron.
 *
 * Por lo tanto ahora agrega la comprobacion de que el preemt_count
 * sea 0 y ademas el need_sched sea TRUE para llamar a schedule()
 *
 * SOFTIRQS:
 * Cuando volvemos de una interrupcion o lo que sea, nos fijamos
 * si hay alguna softirq para ejecutar.
 * Tambien son atendidas por un thread del kernel , en linux 'ksoftirqd'
 *
 * A las softirq las usan solo en subsistema de networking y el driver SCSI
 *
 */

typedef struct priority_array
{
    #define PRIORITY_NUMBER 64
    #define PRIORITY_BITMAP_SIZE (PRIORITY_NUMBER / sizeof(long))
    /* Bitmap que nos da el indice dentro de 'pa_list' */
    long pa_bitmap[PRIORITY_BITMAP_SIZE];

    /* Esta es la lista de colas de prioridades */
    struct list_head pa_list[PRIORITY_NUMBER];

    /* Numero de threads activos en esta parray */
    uint32_t nr_active;

} parray_t;

/* Run queue, there is one PER CPU */
typedef struct run_queue
{
    /* Para controlar el acceso concurente */
    spinlock_t rq_lock;

    /* number of running tasks */
    uint32_t rq_running;

    /* cpu load, used for load balancing */
    uint32_t rq_cpuload;

    /* Current task */
    task_t *rq_current_task;

    /* Pointer to this CPU idle task */
    task_t *rq_idle_task;

    /* these two pointers are used to swap the priority queues */
    parray_t *rq_active;
    parray_t *rq_expired;

    /* The actual priority arrays */
    parray_t rq_arrays[2];
} rqueue_t;

extern tss_t init_tss;

/* We need the page directory to store it in init_mm */
extern pdir_t page_directory;

/* The kernel virtual memory space */
static vm_space_t init_mm =
{
    (pde_t *) &page_directory,
    NULL,
    {0}
};

/* Idle task */
static task_t init_task =
{
    .gid = 0,
    .mm = &init_mm,
    .pid = 0,
    .ppid = 0,
    .state = PROCESS_RUNNING,
    .time_slice = 0,
    .parent_task = &init_task,
    .static_priority = PROCESS_MAX_PRIORITY,
    .dinamic_priority = PROCESS_MAX_PRIORITY,
};

static char init_thread_stack[THREAD_STACK_SIZE];

/* Idle thread */
static thread_t init_thread =
{
    .task = &init_task,
    .flags = 0,
    .preempt_count = 0,
    .status = 0,

    /* La stack en ring3 la seteamos despues */
    .esp = 0,
    .esp0 = (unsigned long ) init_thread_stack + THREAD_STACK_SIZE,
    .eip = 0
};

/* Current is set to the idle thread task */
static thread_t *current = &init_thread;

/* Run queue array, there's one RQ per CPU */
static rqueue_t run_queue;

#ifdef COMPLETE_SCHEDULER
static rqueue_t *
sched_get_rqueue(void)
{
    return NULL;
}

static uint32_t
sched_ffbit(unsigned long *bitmap)
{
    return 0;
}

static void
sched_enqueue_task(thread_t *t, parray_t *array)
{
    list_add_tail(&t->head, array->pa_list + t->priority);
    //__set_bit(t->priority, array->pa_bitmap);
    array->nr_active++;
}

static void
sched_dequeue_task(thread_t *t,  parray_t *array)
{
    array->nr_active--;
    list_del(&t->head);
    //if (list_empty(array->pa_list + t->priority))
    //    __clear_bit(t->priority, array->pa_bitmap);
}

static thread_t *
sched_pick_task(void)
{
    thread_t *t = NULL;
    rqueue_t *rq = sched_get_rqueue();
    uint32_t idx;

    parray_t *active = rq->rq_active;

    /* Is there any waiting tasks? */
    if(!rq->rq_running)
    {
        rq->rq_active = rq->rq_expired;
        rq->rq_expired = active;
    }

    /* Now we need the index inside the priority queues */
    idx = sched_ffbit(active->pa_bitmap);

    /* We get the task */
    t = list_entry(active->pa_list[idx], thread_t, head);

    /* FIXME En realidad aca la task la movemos solo cuando se expira su QUANTUM */
    /* Now we remove the task and we place it on the expired queue */
    list_move_tail(t->head, rq->rq_expired[idx]);

    rq->rq_running--;

    return t;
}

/*
 * La task actual resigna el uso de la CPU para
 * que otra task la pueda user
 */
void
sched_yield(void)
{

}

/*
 * Sets the state of a given task. This function
 * checks for "imposible" state changes.
 *
 * @param task The task whose state will be change
 * @param status The new state
 */
void
task_set_state(thread_t *t, long state)
{
    t->state = state;
}

/*
 * Returns the task's state
 *
 * @param task The task whose state is going to be returned
 * @return Task state
 */
long
task_get_state(thread_t *t)
{
    return t->state;
}

/*
 * Crea una task nueva y la devuelve
 *
 * @param task A pinter to a function
 * @return Task struct with all the information set
 */
task_t *
task_create(void (*task)(void))
{
    return NULL;
}

/*
 * This function calculates the dinamic_priority of the
 * current task. It gives a bonus priority to
 * task's proportional to it's sleep_average.
 *
 * The bonus is a value from 0 to 10
 */
static void
task_recalculate_priority(task_t *task)
{
    #define DEF_TIMESLICE (100 * HZ / 1000)
    #define MAX_SLEEP_AVG (DEF_TIMESLICE * MAX_BONUS)
    #define MAX_BONUS 10
    #define MAX_SLEEP_AVERAGE 0

    #define NS_TO_JIFFIES(x) \
        ((x) / (1000000000 / HZ))
    #define JIFFIES_TO_NS(x) \
        ((x) * (1000000000 / HZ))

    #define CURRENT_BONUS(p) \
        NS_TO_JIFFIES((p)->sleep_average) * MAX_BONUS / MAX_SLEEP_AVERAGE

    task->dinamic_priority = task->static_priority -
        CURRENT_BONUS(task) - MAX_BONUS/2;
}

/*
 * Lo vamos a usar cuando implementemos la capacidad de multiprocesador
 * esta funcion va a migrar task hacia otros procesadores que esten menos
 * utilizados.
 */
void
task_balance_load(void)
{

}
#endif

/******************************************************************************
 * TODO Esto es preeliminar, una ves que tenga todo en claro implementamos el *
 * scheduler completo y borramos estas colas sencillas.                       *
 ******************************************************************************/

/* Aca van a estar los threads que quieren ser ejecutados */
LIST_HEAD(active_queue);

/* Aca estan los threads que expiraron su cuanto */
LIST_HEAD(expired_queue);

struct list_head *active = &active_queue;

/*
 * Devuelve el thread que se esta ejecutando
 * actualmente en el CPU
 *
 * @return The current executing thread on the current CPU
 */
thread_t *
task_current(void)
{
    return current;
}

/*
 * This function is called by the timer interrupt
 * handler. It checks the run queues for processes
 * that need reschudling and also balances the run queues
 * between different processors.
 *
 * The load balancing is issued when a CPU has more task
 * running on it than other CPU in the system.
 * The load value is calculated by multiplying the current
 * number of active task running on the CPU by a scale factor
 * used only to increase the resolution of the value.
 */
void
sched_tick(void)
{
    thread_t *t = task_current();

    if(--t->time_slice == 0)
    {
        printk("sched_tick(): Time Slice expired, need_resched = True\n");

        /* Se le acabo el tiempo, necesita reesquedulearse */
        t->reschedule = 1;

        /* Aca calculariamos la prioridad efectiva */

        /* Aca calculariamos el nuevo timeslice con respecto a la prioridad efectiva */
        t->time_slice = DEF_TIME_SLICE;

        /* Encolamos la tarea actual en la lista de expirados */
        list_move_tail(&t->head, &expired_queue);
    }

    /* TODO: SMP, hacer balance de carga aca */
}

thread_t __attribute__ ((regparm(3))) *
sched_do_switch(thread_t *t1, thread_t *t2)
{

    /* Ahora la stack de ring0 va a ser la stack de ring0 del nuevo thread */
    init_tss.esp0 = t2->esp0;

    /* NOTE: SS0 y el CS0 van a ser siemrpe los mismos asi que no los cambiamos */

    /* Guardar el estado de la FPU */
    if(t1->status || THREAD_STATUS_USED_FPU)
    {

    }

    /* Guardamos los registros de debuggeo */
    if(t1->status || THREAD_STATUS_USED_DBG_REGS)
    {

    }

    /* Guardar los selectores que cambian (ej. GS, FS, etc) */

    /* Cargar registros de proposito general */

    /* Cargar estado de la FPU anterior (si existe) */

    /* Cargar los selectores si cambiaron */

    return t1;
}

/*
 * Switchea entre 2 threads. Guarda los valores del thread que esta corriendo
 * y los reemplaza con los del thread que va a entrar.
 *
 * Es llamada solamente por task_schedule()
 *
 * @param t1 The thread to be replaced
 * @param t2 The thread that will replace 't1'
 */
void
sched_switch(thread_t *t1, thread_t *t2)
{
    /* Si tienen el mismo espacio de VMM no es necesario cambiarlo */
    if(likely(t1->task->mm != t2->task->mm))
    {
        /* Load the new page directory and page tables */
        //set_page_directory((pdir_t *) t1->task->mm->pg_dir);
    }

    /* Guardamos ESP0 en la struct thread por */
    unsigned long esi,edi;
    asm volatile
    (
        "pushfl\n\t"            /* save FLAGS register */
        "pushl %%ebp\n\t"       /* save EBP register */

        "movl %%esp, %0\n\t"    /* t1->esp = esp */
        "movl %5, %%esp\n\t"    /* esp = t2->esp */
        "movl $1f, %1\n\t"      /* t1->eip = eip */

        "pushl %6\n\t"          /* push t2->eip as the new EIP */
        "jmp sched_do_switch\n" /* note that here we use jmp instead of call */

        "1:\t"
        "popl %%ebp\n\t"        /* restore ebp */
        "popfl"                 /* restore CFLAGS */
        : "=m" (t1->esp),       /* 0 */
          "=m" (t1->eip),       /* 1 */
          "=a" (t1),            /* 2 */
          "=S" (esi) ,          /* 3 */
          "=D" (edi)            /* 4 */
        : "m" (t2->esp) ,       /* 5 */
          "m" (t2->eip),        /* 6 */
          "2" (t1),             /* 2 */
          "d" (t2)              /* edx */
    );
}

/*
 * Rutina principal del Scheduler. Selecciona
 * la proxima tarea a ser ejecutada por el procesador.
 *
 * Una de las particularidades de esta funcion es que
 * no es llamada solo cuando se le acaba el quantum al thread
 * sino qeu tambien, cuando algun thread duerme, llama a esta funcion
 * para que se ejecute otro thread.
 * El thread que durmio, no es retirado de la cola de procesos,
 * queda ahi hasta que despierta (cuando le toca de nuevo segun
 * la cola de prioridades).
 *
 * Basicamente, es el Scheduler en si.
 */
void
task_schedule(void)
{
    thread_t *new, *old;

    /* obtenemos la tarea que vamos a remover */
    BUG_ON(!(old  = task_current()));

    if(unlikely(list_empty(active)))
        active = &expired_queue;

    /* obtener la task qeu vamos a hacer que corra */
    BUG_ON(!(new = list_entry(active, thread_t, head)));

    /* Este caso se da por que solo sacamos el thread de la rq si se le termina el Q */
    if(unlikely(new == old))
    {
        printk("task_schedule(): new == old\n");
        return;
    }

    /* do context switch */
    sched_switch(new, old);
}

char kthread_stack[THREAD_STACK_SIZE];
thread_t kthread_descriptor;

void
kthread_fn(void)
{
    printk(" k ");
    task_schedule();
}

void
idle_fn(void)
{
    printk(" i ");
    task_schedule();
}

/*
 * Initializes the scheduler. It's called from kernel_main()
 */
void
init_scheduler(void)
{
    /* Encolamos el idle thread */
    list_add(&init_thread.head, active);

    init_thread.eip = &idle_fn;

    kthread_descriptor.task = &init_task;

    /* Seteamos la funcion que se va a ejecutar */
    kthread_descriptor.eip = &kthread_fn;

    /* Seteamos la stack del nuevo thread */
    kthread_descriptor.esp0 = (unsigned long)
        &kthread_stack[THREAD_STACK_SIZE];

    memset((void *) kthread_stack, 0x00, THREAD_STACK_SIZE);

    /* Agregamos el nuevo thread */
    list_add(&kthread_descriptor.head, active);
}


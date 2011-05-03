#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <types.h>
#include <arch/paging.h>
#include <arch/tss.h>
#include <mm/vm.h>

/*Estas macros definen los posibles estados
  de un proceso, siendo running el estado
  en el que se encuentra un proceso que espera
  ser scheduleado o que actualmente se encuentra
  en ejecucion, y waiting, el estado de un proceso
  que espera la terminacion de una E/S, una senial
  o algun evento en particular*/
#define TASK_RUNNING 0
#define TASK_WAITING 1

struct task;

typedef struct task
{
    /* La estructura va a pertenecer a varias listas, asi que usamos una lista */
    struct list_head head;

    /* Directorio de paginas y demas cosas relacionadas con la memoria virtual */
	vm_space_t *mm;

    /* Identificador unico del proceso */
	uint16_t pid;

	/* Identificador unico del grupo */
	uint16_t gid;

    /* Pid del padre */
    uint16_t ppid;

    /* Estructura que describe el padre del proceso actual */
    struct task *parent_task;

    #define PROCESS_MAX_PRIORITY 0
    #define PROCESS_MIN_PRIORITY 255

    /* the nice value */
    uint32_t static_priority;

    /* scheduler modifies this value of priority */
    uint32_t dinamic_priority;

    /* used to modify the dinamic_priority of a task */
    uint32_t sleep_average;

    #define PROCESS_READY 0
    #define PROCESS_RUNNING 1
    #define PROCESS_BLOCKED 2
    #define PROCESS_READY_SUSPENDED 3
    #define PRCOESS_BLOCKED_SUSPENDED 4

    /* Estado actual del proceso */
    uint32_t state;

    /* Lista con todos los hijos del proceso */
    struct list_head process_childs;

    struct tss_segment *context;

    uint32_t time_slice;
} task_t;

/* Este es el tamano de la stack de los threads */
#define THREAD_STACK_SIZE 4*1024

typedef struct thread_info
{
    /* Thread list member */
    struct list_head head;

    task_t *task; /* main task structure */

    unsigned long flags; /* low level flags */

    #define THREAD_STATUS_USED_FPU 1
    #define THREAD_STATUS_USED_DBG_REGS 2
    unsigned long status; /* thread-synchronous flags */
    unsigned long state;

    unsigned long priority;

    /* Una marca para calcular cuanto tiempo se ejecuto el thread */
    unsigned long time_stamp;

    #define MIN_TIME_SLICE 100
    #define DEF_TIME_SLICE 300
    #define MAX_TIME_SLICE 800
    unsigned long time_slice;
    unsigned long reschedule;

    int preempt_count; /* 0 => preemptable, <0 => BUG */

    unsigned long esp;
    unsigned long esp0;
    unsigned long eip;
} thread_t;

void sched_tick(void);

#endif

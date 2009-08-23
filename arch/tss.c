#include <arch/tss.h>
#include <arch/paging.h>
#include <lib/string.h>
#include <kernel/printk.h>
#include <arch/paging.h>
#include <mm/mm.h>

/*
 * Este TSS va a tener los valores de SS0:ESP0 del proceso
 * actual que esta corriendo en la CPU.
 * Esta estructura se actualiza cuando switcheamos a otro proceso
 */

/* TODO: #define TSS_ARRAY_SIZE NUMBER_OF_CPUS */
#define TSS_ARRAY_SIZE 1

tss_t tss_array[TSS_ARRAY_SIZE];

/* Por ahora usamos esta TSS, despues usamos el array por CPU */
tss_t init_cpu_tss;

void
init_tss()
{
    /* Inicializado a 0 por defecto */
    memset((void *) tss_array, 0x00, sizeof(tss_t) * TSS_ARRAY_SIZE);

    struct segment_selector ss = {SS_RPL_0, SS_TABLE_GDT, GDT_CPU0TSS_SEL};

    /* Ahora el registro TR apunta a la entrada en la GDT que descibe a cpu0_tss */
    load_task_register(ss);
}

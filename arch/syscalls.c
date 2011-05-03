#include <syscalls.h> 
#include <arch/irq.h>
#include <kernel/printk.h>

/* This is the syscall table, each entry is
 * a syscall handler */
void *syscall_table[SYSCALL_NUMBER];

void syscall_dispatcher(icontext_t *r)
{
	/* Esta funcion debe copiar los argumentos 
	 * que paso el USER a la syscall a kernel mode
	 * para que cuando el kernel los use el usuario
	 * no pueda modificarlos
	 */
	
	/* Si existen buffers estos deberan ser verificados
	 * para ver si pueden ser accedidos
	 */
	
	/* Ahora debemos llamar al syscall_handler correspondiente */
}

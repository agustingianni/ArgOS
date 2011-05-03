 #include <arch/idt.h>
 #include <arch/gates.h>

 /*
  * @doc http://my.execpc.com/~geezer/osd/intr/index.htm
  *
  * The IDT may contain interrupt gates (access byte = 8Eh or 0EEh)
  * or trap gates (access byte = 8Fh or 0EFh).
  * Interrupt gates clear the IF bit when an interrupt occurs,
  * disabling further hardware interrupts.
  * Trap gates leave the IF bit unchanged.
  */

 extern struct idt_entry idt[];
 /*
  * The processor will not clear the IF flag on EFLAGS
  * so there may be overlaps when handling trap_gate's
  *
  * Cleared flags: TF, VM, RF and NT
  */
 void
 set_trap_gate(int n, void *handler, int dpl)
 {
 	idt_set_entry
 	(
 		&idt[n],
 		handler,
 		KERNEL_CS,
 		SDT_SYS386TGT,
 		dpl
 	);
 }

 /*
  * When we use interrupt gates, the processor, clears
  * the IF flag on EFLAGS to prevent other interrupts to
  * happen.
  *
  * Cleared flags: IF, TF, VM, RF and NT
  */
 void
 set_interrupt_gate(int n, void *handler, int dpl)
 {
 	idt_set_entry
 	(
 		&idt[n],
 		handler,
 		KERNEL_CS,
 		SDT_SYS386IGT,
 		dpl
 	);
 }

 /*
  * En la IDT podemos setear una task gate que lo que
  * va a tener en ves de la direccion y el selector
  * de una funcion que maneja la interrupcion tiene
  * un selector que es un indice a la GDT. En el lugar
  * indexado hay un descriptor que apunta al TSS que
  * sera ejecutado.
  */
 void
 set_task_gate(int n, void *handler, int dpl)
 {
 	idt_set_entry
 	(
 		&idt[n],
 		handler,
 		KERNEL_CS,
 		SDT_SYSTASKGT,
 		dpl
 	);
 }

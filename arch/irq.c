 #include <arch/irq.h>
 #include <arch/gates.h>
 #include <arch/idt.h>
 #include <arch/asm.h>
 #include <stddef.h>
 #include <assert.h>
 #include <kernel/console.h>
 #include <kernel/printk.h>
 #include <drivers/io.h>

 /*
  * @see
  * http://heim.ifi.uio.no/~stanisls/helppc/8259.html
  * http://atc.ugr.es/docencia/udigital/1204.html
  * http://karloxxx.dimor.net/data/files/UNID/
  * 	Sistemas%20Operativos/UNIDAD%202/interrupciones.pdf
  * http://www.osdev.org/wiki/PIC#Programming_the_PIC_chips
  */

 static handler_t irq_handlers[IRQ_HANDLERS_NUMBER] = {NULL};
 static irqmask_t irq_mask = 0xfffb;

/*
 * Returns the "software" irq mask.
 * The software mask is used to avoid accessing
 * the APIC
 *
 * @return The current irq mask
 */
 irqmask_t
 irq_get_mask(void)
 {
	 return irq_mask;
 }

 /*
  * Sets the "software" irq mask, there is no need to access
  * the APIC. It also returns the old mask to be
  * replaced so we can restore it later
  *
  * @param new_mask The new mask to be set
  * @return The replaced mask
  */
 irqmask_t
 irq_set_mask(irqmask_t new_mask)
 {
	 irqmask_t old_mask = irq_mask;
	 irq_mask = new_mask;

	 /* store the IRQ masks.*/
	 outb(PIC_MASTER_IMR, (new_mask & 0xff));
	 outb(PIC_SLAVE_IMR, (((new_mask)>>8) & 0xff));

	 return old_mask;
 }

 /*
  * TODO: Lo que ahora tenemos que hacer
  * es reflejarlos cambios que hacemos aca
  * en la mascara de IRQ por software.
  * Hay que tratar de hacerlo eficientemente,
  * verificando si la int ya fue enmascarada
  * y demas cosas.
  */

 /*
  * Disable the given interrupt line and ensure no handler
  * on the line is executing before returning
  *
  * @param n The irq line to disable
  */
 void
 irq_set_disabled(int n)
 {
	 outb(PIC_NUMBER(n),
		(inb(PIC_NUMBER(n)) | INTERRUPT_MASK(n)));
 }

 /*
  * Enables the 'n' IRQ by sending the
  * correct INTERRUPT_MASK to the PIC
  *
  * @param n The irq line to enable
  */
 void
 irq_set_enabled(int n)
 {
	 outb(PIC_NUMBER(n),
		(inb(PIC_NUMBER(n)) & INTERRUPT_MASK(n)));
 }

 /*
  * Remap IRQ0 - IRQ15 to IDT entries 32-47
  * This range is specified by intel to be used
  * by maskable interrupts and int NN instruction.
  *
  * Si no hacemos esto las IRQ del hardware
  * van a caer dentro del rango que reservo
  * intel para las exceptions y demas, es decir
  * las vamos a interpretar mal. Es por eso que
  * hay que decirle al PIC que cuando haya una interrupcion
  * de hardware, le "sume" cierto offset.
  */
 void
 irq_remap(void)
 {
	//unsigned char a1,a2;

	/* We save the IRQ mask */
    //a1 = inb(PIC_MASTER_IMR);
    //a2 = inb(PIC_SLAVE_IMR);

    printk("[irq_remap] Remapping PIC to offsets 0x20 and 0x28\n");

    /* ICW1 */
    outb(PIC_MASTER_CMD, 0x11); /* NEED_ICW4 && CASCADE_MODE */
    outb(PIC_SLAVE_CMD, 0x11);	/* NEED_ICW4 && CASCADE_MODE */

    /* ICW2 */
    outb(PIC_MASTER_IMR, 0x20); /* route IRQs 0...7 to INTs 20h...27h */
    outb(PIC_SLAVE_IMR, 0x28);	/* ...IRQs 8...15 to INTs 28h...2Fh */

    /* ICW3 */
    outb(PIC_MASTER_IMR, 0x04); /* 4d = 0100b = IRQ Line 2 will have a cascaded PIC */
    outb(PIC_SLAVE_IMR, 0x02);	/* cascaded into IRQ line 2*/

    /* ICW4 */
    outb(PIC_MASTER_IMR, 0x01);	/* MODE_8086 & !AUTO_EIO*/
    outb(PIC_SLAVE_IMR, 0x01);

    /* Restore the IRQ saved masks.*/
    //outb(PIC_MASTER_IMR, a1);
    //outb(PIC_SLAVE_IMR, a2);

    outb(PIC_MASTER_IMR, 0);
    outb(PIC_SLAVE_IMR, 0);
 }

 /*
  * Install all the IRQ service routines
  */
 void
 irq_install(void)
 {
	/* Sets all the ISR's to the null_handler */
	irq_set_default_handlers();

	/* PIC remap */
	irq_remap();

	/*
	 * h_irq* son rutinas que estan en asm_idt.s, lo que hacen
	 * es guardar el contexto de ejecucion y switchear las pilas
	 * si es necesario. Luego llaman al dispatcher de irq's.
	 */
	set_trap_gate(32, h_irq0, SD_DPL_0);
	set_trap_gate(33, h_irq1, SD_DPL_0);
	set_trap_gate(34, h_irq2, SD_DPL_0);
	set_trap_gate(35, h_irq3, SD_DPL_0);
	set_trap_gate(36, h_irq4, SD_DPL_0);
	set_trap_gate(37, h_irq5, SD_DPL_0);
	set_trap_gate(38, h_irq6, SD_DPL_0);
	set_trap_gate(39, h_irq7, SD_DPL_0);
	set_trap_gate(40, h_irq8, SD_DPL_0);
	set_trap_gate(41, h_irq9, SD_DPL_0);
	set_trap_gate(42, h_irq10, SD_DPL_0);
	set_trap_gate(43, h_irq11, SD_DPL_0);
	set_trap_gate(44, h_irq12, SD_DPL_0);
	set_trap_gate(45, h_irq13, SD_DPL_0);
	set_trap_gate(46, h_irq14, SD_DPL_0);
	set_trap_gate(47, h_irq15, SD_DPL_0);

    /*
     * TODO: Mover de aca. No tiene nada que ver con las IRQ's
     */
	/* System call handler */
    set_trap_gate(0x80, h_syscall, SD_DPL_3);
}

 /*
  * Generic IRQ hanlding routine.
  * Calls the registered ISR routine
  *
  * @param r The Interrupt Context of the current task
  */
 void
 irq_dispatcher(icontext_t *r)
 {
	/*
	 * Since irqs 0..15 were mapped to interrupts 32..47 we
	 * need to fix the index into the irq_handler array
	 */
	int irqno = r->int_no - 32;
	assert(irqno >= 0 && irqno <= IRQ_HANDLERS_NUMBER);

	/* Call the registered ISR */
	irq_handlers[irqno](r);

	/*
	 * If the interrupt is on SLAVE_PIC
	 * wee need to send EOI to the first pic too
	 * since its cascaded into IRQ2 line.
	 */
	irq_send_eoi(irqno);
 }

 /*
  * The null handler is the default ISR
  * handler. It just halts the machine
  *
  * @param r The Interrupt Context of the current task
  */
 static void
 irq_null_handler(icontext_t *r)
 {
 	printk("INT (%d) system halted\n",
 		r->int_no);

 	irq_dump_context(r);
 	cpu_halt();
 }

 /*
  * TODO: Aca vamos a tener que agregar un dev_id por que
  * cuando soportemos multiples dispositivos por interrupcion
  * los handlers van a tener que ir encadenados y algo los tiene
  * que identificar
  *
  * Installs an IRQ handler
  *
  * @param n The interrupt line
  * @param handler Handler of the interrupt line 'n'
  */
 void
 irq_set_handler(int n, handler_t handler)
 {
	 int iflag = begin_atomic();
	 {
		 irq_handlers[n] = handler;
	 }
	 end_atomic(iflag);
 }

 /*
  * TODO: Agregar el dev_id tambien aca
  *
  * Returns the IRQ handler at position N
  *
  * @param n Interrupt line
  * @return The handler of the interrupt line 'n'
  */
 handler_t
 irq_get_handler(int n)
 {
	if(n < 0 || n >= sizeof(irq_handlers))
	{
		return NULL;
	}

 	return irq_handlers[n];
 }

 /*
  * TODO: Agregar el dev_id aca tambien.
  *
  * Uninstalls an IRQ handler and sets
  * the default null handler as the handler
  *
  * @param n Interrupt line
  */
 void
 irq_clear_handler(int n)
 {
 	irq_set_handler(n, &irq_null_handler);
 }

 /*
  * TODO: dev_id tambien aca
  *
  * Initializes the IRQ handlers array to the
  * null handler
  */
 void
 irq_set_default_handlers(void)
 {
 	int i;

 	for(i = 0; i < IRQ_HANDLERS_NUMBER; i++)
 	{
		irq_clear_handler(i);
 	}
 }

 /*
  * Prints the current interrupt context
  * Mainly used in debugging routines.
  *
  * @param context The current interrupt context to be printed
  */
 void
 irq_dump_context(icontext_t *context)
 {
    printk
    (
    	"  eax = 0x%.8x ebx = 0x%.8x ecx = 0x%.8x edx = 0x%.8x\n"
    	"  esi = 0x%.8x edi = 0x%.8x ebp = 0x%.8x eip = 0x%.8x\n"
    	"  EFLAGS = 0x%.8x\n"
    	"  cs = 0x%.4x index = %.2d ti = %.2d rpl = %.2d\n"
    	"  ds = 0x%.4x index = %.2d ti = %.2d rpl = %.2d\n"
    	"  es = 0x%.4x index = %.2d ti = %.2d rpl = %.2d\n"
    	"  fs = 0x%.4x index = %.2d ti = %.2d rpl = %.2d\n"
    	"  gs = 0x%.4x index = %.2d ti = %.2d rpl = %.2d\n\n"
       	"  INT = %d ERR_CODE = %.2d\n"
       	"  index = %d TI = %d IDT = %d EXT = %d\n",
    	context->eax, context->ebx, context->ecx, context->edx,
    	context->esi, context->edi, context->ebp,
    	context->eip, context->eflags,
    	context->cs, context->cs >> 3, (context->cs >> 2) & 1, context->cs & 3,
    	context->ds, context->ds >> 3, (context->ds >> 2) & 1, context->ds & 3,
    	context->es, context->es >> 3, (context->es >> 2) & 1, context->es & 3,
    	context->fs, context->fs >> 3, (context->fs >> 2) & 1, context->fs & 3,
    	context->gs, context->gs >> 3, (context->gs >> 2) & 1, context->gs & 3,
    	context->int_no, context->err_code,
    	context->err_code, context->err_code >> 3,
    	(context->err_code >> 2) & 1,(context->err_code >> 1) & 1, context->err_code & 1
    );
 }


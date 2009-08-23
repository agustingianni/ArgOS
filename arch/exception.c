 #include <arch/gates.h>
 #include <arch/exception.h>
 #include <arch/irq.h>
 #include <arch/idt.h>
 #include <stddef.h>
 #include <assert.h>
 #include <kernel/console.h>
 #include <kernel/printk.h>

 handler_t exception_handlers[EXCEPTION_HANDLERS_NUMBER] = {NULL};

 /* Fills the IDT with the Exception Handlers */
 void 
 exceptions_install(void)
 {
	/* Initialize all the handlers to the null_handler */
	init_exception_handlers();
	 
 	/*
 	 * trap_gate 			= it does not disable interrupts
 	 * interrupt_gate 		= it does disable interrupts
 	 * task_gate 			= task switch occours
     *
     * The only difference between these two is that an Interrupt Gate 
     * will disable further processor handling of hardware interrupts, 
     * making it especially suitable to service hardware interrupts, 
     * while a Trap will leave hardware interrupts enabled and is thus 
     * mainly used for handling software interrupts and exceptions.
 	 */
	set_trap_gate(0, h_divide_exception, SD_DPL_0);
    set_interrupt_gate(1, h_debug_exception, SD_DPL_0);
	set_interrupt_gate(2, h_nmi_exception, SD_DPL_0);

    /* These can be called by user */
    set_interrupt_gate(3, h_int3_exception, SD_DPL_3);
    set_trap_gate(4, h_into_exception, SD_DPL_3);
	
    set_trap_gate(5, h_bounds_exception, SD_DPL_0);
    set_trap_gate(6, h_opcode_exception, SD_DPL_0);
    set_trap_gate(7, h_coprocessor_exception, SD_DPL_0);

    set_interrupt_gate(8, h_dfault_exception, SD_DPL_0);

    set_trap_gate(9, h_mathoverflow_exception, SD_DPL_0);
    set_trap_gate(10, h_badtss_exception, SD_DPL_0);
    set_trap_gate(11, h_segnotpresent_exception, SD_DPL_0);
    set_trap_gate(12, h_stk_fault_exception, SD_DPL_0);
    set_trap_gate(13, h_gp_fault_exception, SD_DPL_0);
    
    set_interrupt_gate(14, h_pg_fault_exception, SD_DPL_0);
    
    set_trap_gate(15, h_reserved_exception, SD_DPL_0);
    set_trap_gate(16, h_fp_exception, SD_DPL_0);
    set_trap_gate(17, h_align_exception, SD_DPL_0);
    set_trap_gate(18, h_machine_chk_exception, SD_DPL_0);
    
    set_trap_gate(19, h_reserved_19_exception, SD_DPL_0);
    set_trap_gate(20, h_reserved_20_exception, SD_DPL_0);
    set_trap_gate(21, h_reserved_21_exception, SD_DPL_0);
    set_trap_gate(22, h_reserved_22_exception, SD_DPL_0);
    set_trap_gate(23, h_reserved_23_exception, SD_DPL_0);
    set_trap_gate(24, h_reserved_24_exception, SD_DPL_0);
    set_trap_gate(25, h_reserved_25_exception, SD_DPL_0);
    set_trap_gate(26, h_reserved_26_exception, SD_DPL_0);
    set_trap_gate(27, h_reserved_27_exception, SD_DPL_0);
    set_trap_gate(28, h_reserved_28_exception, SD_DPL_0);
    set_trap_gate(29, h_reserved_29_exception, SD_DPL_0);
    set_trap_gate(30, h_reserved_30_exception, SD_DPL_0);
    set_trap_gate(31, h_reserved_31_exception, SD_DPL_0);
 }
 
 /* This function is called by all the Exception
  * handlers defined in asm_idt.s */
 void
 exception_dispatcher(icontext_t *r)
 {
	assert(r->int_no >= 0 && r->int_no < EXCEPTION_HANDLERS_NUMBER);
	exception_handlers[r->int_no](r);
	
	return;
 }
 
 /*
  * The null handler is the default EXCEPTION 
  * handler. It just halts the machine
  */
 static void 
 null_handler(icontext_t *r)
 {
 	printk("Exception (%s) system halted\n", 
 		exception_name(r->int_no));
 	
 	irq_dump_context(r);
 	cpu_halt();
 }
 
 /*
  * Returns a pointer to the handler
  * of the exception "n"
  */
 handler_t
 get_exception_handler(int n)
 {
 	if(n< 0 || n >= 32)
 	{
		return NULL;
 	}
 	
 	return exception_handlers[n];
 }
 
 /*
  * Sets "handler" as the exception handler of the
  * exeption number "n"
  * 
  * TODO: Use locks to access the exception_handlers
  * array 
  */
 void
 set_exception_handler(int n, handler_t handler)
 {
 	if(n< 0 || n >= sizeof(exception_handlers))
 	{
		return;
 	}
 	
 	exception_handlers[n] = handler;
 }

 /* Initialize all the execption handlers to the 
  * "null_handler"
  */
 void
 init_exception_handlers(void)
 {
 	int i;
 	
 	/* clear all the exception handlers */
 	for(i = 0; i < EXCEPTION_HANDLERS_NUMBER; i++)
 	{
 		set_exception_handler(i, &null_handler);
 	}
 }

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

 #include <arch/idt.h>
 #include <arch/irq.h>

 #define EXCEPTION_HANDLERS_NUMBER 32
 
 void exceptions_install		(void);
 handler_t get_exception_handler(int n);
 void set_exception_handler   	(int n, handler_t handler);
 void clear_exception_handler 	(int n);
 void init_exception_handlers 	(void);
  
 static char *exception_names[EXCEPTION_HANDLERS_NUMBER] =
 {
    "Divide Error Exception",
    "Debug Exception",
    "NMI (Non Maskable Interrupt) Exception",
    "Breakpoint Exception",
    "Overflow Exception",
    "Out of Bounds Exception",
    "Invalid Opcode Exception",
    "Device not available Exception",

    "Double Fault Exception",
    "Coprocessor Segment Overrun Exception",
    "Invalid TSS Exception",
    "Segment Not Present Exception",
    "Stack Fault Exception",
    "General Protection Fault Exception",
    "Page Fault Exception",
    "Unknown Interrupt Exception",

    "Coprocessor Fault Exception",
    "Alignment Check Exception",
    "Machine Check Exception",
    "SIMD Floating-Point Exception",
    "int20-Reserved",
    "int21-Reserved",
    "int22-Reserved",
    "int23-Reserved",

    "int24-Reserved",
    "int25-Reserved",
    "int26-Reserved",
    "int27-Reserved",
    "int28-Reserved",
    "int29-Reserved",
    "int30-Reserved",
 	"int31-Reserved"
 };

 #define exception_name(n) exception_names[n]

 extern struct idt_entry idt[IDT_ENTRIES];
 
 extern void h_divide_exception();
 extern void h_debug_exception();
 extern void h_nmi_exception();
 extern void h_int3_exception();
 extern void h_into_exception();
 extern void h_bounds_exception();
 extern void h_opcode_exception();
 extern void h_coprocessor_exception();
 extern void h_dfault_exception();
 extern void h_mathoverflow_exception();
 extern void h_badtss_exception();
 extern void h_segnotpresent_exception();
 extern void h_stk_fault_exception();
 extern void h_gp_fault_exception();
 extern void h_pg_fault_exception();
 extern void h_reserved_exception();
 extern void h_fp_exception();
 extern void h_align_exception();
 extern void h_machine_chk_exception();

 extern void h_reserved_19_exception();
 extern void h_reserved_20_exception();
 extern void h_reserved_21_exception();
 extern void h_reserved_22_exception();
 extern void h_reserved_23_exception();
 extern void h_reserved_24_exception();
 extern void h_reserved_25_exception();
 extern void h_reserved_26_exception();
 extern void h_reserved_27_exception();
 extern void h_reserved_28_exception();
 extern void h_reserved_29_exception();
 extern void h_reserved_30_exception();
 extern void h_reserved_31_exception();
 
#endif /*EXCEPTION_H_*/

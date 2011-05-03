#ifndef IRQ_H_
#define IRQ_H_

 #include <arch/idt.h>
 #include <arch/asm.h>
 #include <drivers/io.h>
 #include <arch/registers.h>

 #define EFLAGS_IF (1 << 9)

 /* The number of handlers */
 #define IRQ_HANDLERS_NUMBER 16

 /* Given an interrupt number it returs the mask
  * to be sent to the PIC */
 #define INTERRUPT_MASK(n) ~(1 << n)

 /*
  * Get the master and slave parts of an IRQ mask.
  */
 #define GET_MASTER_MASK (mask) ((mask) & 0xff)
 #define GET_SLAVE_MASK  (mask) (((mask)>>8) & 0xff)

 /* Given an interrupt number it returns the corresponding PIC
  * address mask register*/
 #define PIC_NUMBER(n) (n < 8) ? PIC_MASTER_IMR : PIC_SLAVE_IMR

 /* i8259A PIC registers */
 #define PIC_MASTER_CMD		0x20			/* IRQ's from 0 to 7  */
 #define PIC_MASTER_IMR		0x21			/* INT MASK REGISTER */
 #define PIC_MASTER_ISR		PIC_MASTER_CMD
 #define PIC_MASTER_POLL	PIC_MASTER_ISR
 #define PIC_MASTER_OCW3	PIC_MASTER_ISR
 #define PIC_SLAVE_CMD		0xa0			/* IRQ's from 8 to 15 */
 #define PIC_SLAVE_IMR		0xa1			/* INT MASK REGISTER */

 /*
  * This should be the THREAD_CONTEXT structure
  * its used to save the context of a thread when
  * it is interrupted by a IRQ or an Exception
  */
 typedef struct
 {
	  unsigned int gs, fs, es, ds;
	  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	  unsigned int int_no, err_code;
	  unsigned int eip, cs, eflags, useresp, ss;
 } __attribute__((packed)) icontext_t;

 typedef unsigned short irqmask_t;
 typedef void (*handler_t)(icontext_t *context);


 extern struct idt_entry idt[IDT_ENTRIES];

 /*
  * The EIO (End of interrupt) signal is sent to the pic
  * so the PIC can reset the In Service Register
  */
 static inline void
 irq_send_eoi(int intno)
 {
	 /* because PIC2 is cascaded we need to acknowledge it too */
	 if(intno >= 8)
	 {
		 outb(PIC_SLAVE_CMD, 0x20);
	 }

	 outb(PIC_MASTER_CMD, 0x20);
 }

 /* Disable local interrupt delivery */
 static inline void irq_disable(void)
 {
     __asm__ __volatile__ ("cli");
 }

 /* Enable local interrupt delivery */
 static inline void irq_enable(void)
 {
     __asm__ __volatile__ ("sti");
 }

 /*
  * Returns nonzero if local interrupt
  * delivery is disabled; otherwise returns zero
  */
 static inline int irq_enabled(void)
 {
	 eflags_t eflags = read_eflags();

	 return eflags.if_;
	 //return ((eflags & EFLAGS_IF) != 0);
 }

 /* Begin interrupt-atomic critical section */
 static inline int begin_atomic(void)
 {
     if (irq_enabled())
     {
    	 irq_disable();
    	 return 1;
     }

     return 0;
 }

 /* End interrupt-atomic critical section */
 static inline void end_atomic(int iflag)
 {
     if (iflag)
     {
    	 /* Interrupts were originally enabled, so turn them back on */
    	 irq_enable();
     }
 }

 void irq_install		(void);
 void irq_set_handler		(int n, handler_t handler);
 handler_t irq_get_handler	(int n);
 void irq_clear_handler		(int n);
 void irq_set_default_handlers		(void);
 void irq_dump_context		(icontext_t *context);

 extern void h_irq0();
 extern void h_irq1();
 extern void h_irq2();
 extern void h_irq3();
 extern void h_irq4();
 extern void h_irq5();
 extern void h_irq6();
 extern void h_irq7();
 extern void h_irq8();
 extern void h_irq9();
 extern void h_irq10();
 extern void h_irq11();
 extern void h_irq12();
 extern void h_irq13();
 extern void h_irq14();
 extern void h_irq15();

 extern void h_syscall();

#endif /*IRQ_H_*/

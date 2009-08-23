 #ifndef IDT_H_
 #define IDT_H_
 
 #include <types.h>
 #include <mm/mm.h>
 
 #define IDT_ENTRIES 256
 
 static inline void
 lidt(struct region_descriptor *region)
 {
	 __asm volatile("lidt %0" : : "m" (*region));
 }
 
  /*
  * An IDT entry can be a task_gate, trap_gate
  * or an interrupt_gate
  */
 struct idt_entry
 {
    unsigned ie_lobase:16;
    unsigned ie_sel:16;
    unsigned ie_zero:8;
    unsigned ie_type:5;
    unsigned ie_dpl:2;
    unsigned ie_present:1;
    unsigned ie_hibase:16;
 } __attribute__((packed));

 /* Fills a idt entry */
 void idt_set_entry
 (
 	struct idt_entry *ie,
	void *base,
	int type,
	int dpl,
	int sel
 );
 
 void init_idt(void);
 
 extern void idt_flush(struct region_descriptor *);

#endif /*IDT_H_*/

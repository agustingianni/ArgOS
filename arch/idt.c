 #include <arch/idt.h>
 #include <arch/irq.h>
 #include <arch/exception.h>
 #include <mm/mm.h>
 #include <stddef.h>
 #include <arch/gates.h>
 #include <lib/string.h>

 struct idt_entry idt[IDT_ENTRIES];
 static struct region_descriptor idtp;

 /*
  * Add an IDT entry to the IDT table
  */
 void
 idt_set_entry
 (
 	struct idt_entry *ie,
	void *base,		/* pointer to function */
	int sel,		/* selector */
	int type,
	int dpl
 )
 {
	ie->ie_sel 		= sel;
	ie->ie_lobase 	= ((int) base & 0xFFFF);
	ie->ie_hibase 	= ((int) base >> 16) & 0xFFFF;
	ie->ie_zero		= 0;
	ie->ie_type     = type;
	ie->ie_present  = 1;
	ie->ie_dpl		= dpl;
 }
 
 /*
  * Initialize all the IDT entries to zero and load
  * the idt descriptor to IDTR
  */
 void
 init_idt(void)
 {
 	int iflags = begin_atomic();
 	{
 		/* zero out the array */
 		memset((void *) idt, 0x00, sizeof(idt[0])*IDT_ENTRIES);
 		
 		/* set the region describing the new IDT */
	 	set_region
		(
			&idtp,
			&idt,
			(sizeof(idt[0]) * IDT_ENTRIES) - 1
		);

	 	/* now load the new IDT */
		lidt(&idtp);
		
	   	/* install the exception handlers */
	   	exceptions_install();
	   	
	   	/* install the IRQ handlers */
	    irq_install();
	}
 	end_atomic(iflags);
 	
 	return;
 }

 #if 0
 /* debugging routines */
 #define TEST_BIT(v,b) (!!(v & (1 << b)))

 void
 dump_descriptor(char *name, unsigned base, unsigned i)
 {
 	printk
 	(
 		"%s#%03d [0x%02x]: flags=%02x <%s,DPL%d,%s>"
 		" sel=%02x <%s#%02d,RPL%d> off=%08x\n",
 		name,
 		i, 
 		i, 
 		flags, 
 		flags & 0x80 ? "P" : "NP",
 		(flags & 0x60) >> 5,
 		(flags & 0x1f) == 0x0e ? "intr" :
 		(flags & 0x1f) == 0x0f ? "trap" :
 		(flags & 0x1f) == 0x05 ? "task" :
 		(flags & 0x1f) == 0x0c ? "call" : "????",
 		sel, 
 		sel & 4 ? "LDT" : "GDT", 
 		sel >> 3, 
 		sel & 3,
 		(unsigned)((desc.off2 << 16) | desc.off1)
 	);
 }

 void
 dump_gdt_ldt (char * name, unsigned base, unsigned num)
 {
 	unsigned dbase, dlimit, a, b, t, i, f;
 	gdt_ldt_entry_t desc;
 	tss_t tss;

 	printf("\n %s at %08x, %d entries:\n\n", name, base, num);

 	for (i = 0; i < num; i++)
 	{
 		xkm(rkm, kmem, base + 8*i, &desc, sizeof(desc), "rkm gdtldt");
 		a = desc.a, b = desc.b, f = (b & 0xf0ff00) >> 8, t = f & 0x1f;
 		
 		if (!a & !b)
 			continue;
 		
 		if (t == 0x0c)
 		{ /* call gate */
 			dump_sys_desc(name, base, i);
 			continue;
 		}

 		dbase = (b & 0xff000000) | ((b & 0xff) << 16) | (a >> 16);
 		dlimit = (a & 0xffff) | (b & 0x0f0000);

 		printf
 		(
 			"%s#%03d: base=%08x limit=%05x%s"
 			"\tflags=%04x <P=%d DPL=%d %s>\n",
 			name, i, dbase, dlimit, TEST_BIT(b,23) ? "fff" : "",
 			f, TEST_BIT(b,15), (b & 0x6000) >> 13,
 			t == 0x0b ? "Busy TSS" :
 			t == 0x09 ? "Available TSS" :
 			t == 0x02 ? "LDT" :
 			(f & 0x401c) == 0x4018 ? "32-bit Code" :
 			(f & 0x401c) == 0x18 ? "16-bit Code" :
 			(t & 0x1c) == 0x1c ? "Conforming code" :
 			(t & 0x1e) == 0x10 ? "RO Data" :
 			(t & 0x1e) == 0x12 ? "RW Data" :
 			(t & 0x1c) == 0x14 ? "Exp-down data" :
 			"Unknown"
 		);

 		if (t == 0x02)
 			dump_gdt_ldt(" LDT", dbase, dlimit + 1 >> 3);

 		if ((t == 0x09) | (t == 0x0b))
 		{
 			xkm(rkm, kmem, dbase, &tss, sizeof(tss), "rkm tss");
 			
 			printf("\n TSS at %08x, %d bytes:\n\n",
 				dbase, dlimit + 1);
 	
 			printf(" CS:%04x <%s#%02d,RPL%d>"
 				" EIP:%08x EFLAGS:%08x\n\n",
 				tss.cs, tss.cs & 4 ? "LDT" : "GDT",
 				tss.cs >> 3, tss.cs & 3,
 				tss.eip, tss.eflags);
 		}
 	}

 	printf("\n");
 }

 void
 dump_descriptor_tables(struct region_descriptor rd)
 {
 	int i;

 	printk("Descriptor at %08x, %d entries:\n\n",
 			rd.rd_base, rd.rd_limit);
 	
 	for (i = 0; i < rd.rd_limit; i++)
 	{
 		dump_sys_desc("IDT", base, i);
 	}
 }
 #endif
 

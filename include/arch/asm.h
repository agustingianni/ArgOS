#ifndef ASM_H_
#define ASM_H_

 #include <arch/registers.h>
 #include <types.h>

 /* Put the CPU to sleep until an IRQ occurs */ 
 static inline void cpu_idle(void)
 {
	asm("hlt\n\t");
 }
 
 /* Put the puppy to sleep */
 static inline void cpu_halt(void)
 {
	asm("cli ; hlt\n\t");
 }

 static inline void io_wait(void)
 {
    asm volatile("jmp 1f;1:jmp 1f;1:");
 }
 
 static inline void pause(void)
 {
 	asm volatile("pause");
 }

 static inline void write_cr0(unsigned int value)
 {
	asm volatile("mov %0, %%cr0\n": : "r" (value));
 }
 
 static inline void write_cr1(unsigned int value)
 {
	asm volatile("mov %0, %%cr1\n": : "r" (value));
 }

 static inline void write_cr2(unsigned int value)
 {
	asm volatile("mov %0, %%cr2\n": : "r" (value));
 }

 static inline void write_cr3(unsigned int value)
 {
	asm volatile("mov %0, %%cr3\n": : "r" (value));
 }

 static inline void write_cr4(unsigned int value)
 {
	asm volatile("mov %0, %%cr4\n": : "r" (value));
 }
 
 static inline unsigned int read_cr0(void)
 {
	unsigned int ret = 0;
	asm volatile("movl %%cr0, %0\n": "=r" (ret));
	return ret;
 }
 
 static inline unsigned int read_cr1(void)
 {
	unsigned int ret = 0;
	asm volatile("movl %%cr1, %0\n": "=r" (ret));
	return ret;
 }

 static inline unsigned int read_cr2(void)
 {
	unsigned int ret = 0;	 
	asm volatile("movl %%cr2, %0\n": "=r" (ret));
	return ret;
 }
 
 static inline unsigned int read_cr3(void)
 {
	unsigned int ret = 0;
	asm volatile("movl %%cr3, %0\n": "=r" (ret));
	return ret;
 }

 static inline unsigned int read_cr4(void)
 {
	 unsigned int ret = -1;	 
	 asm volatile("movl %%cr4, %0\n": "=r" (ret));
	 return ret;
 }

 /* Returns the current EFLAGS */
 static inline eflags_t read_eflags(void)
 {
	eflags_t eflags;
	
 	asm volatile
 	(
 		"pushfl\n\t"
 		"popl %0\n\t"
 		 : "=m" (eflags)
 	);
 	
 	return eflags;
 }
  
 /* Sets the current EFLAGS */
 static inline void write_eflags(eflags_t eflags)
 {
 	asm volatile 
 	(
 		"pushl %0\n\t"
 		"popfl\n\t"
 		:
 		: "m"(eflags)
 		: "cc", "memory"
 		/* 
 		 * cc = condition codes will be changed
 		 * memory = memory will be changed
 		 */
 	);
 }
 
 /*
  * INVLPG invalidates the translation lookahead 
  * buffer (TLB) entry associated with the 
  * supplied memory address.
  */
 static inline void 
 invlpg(unsigned int addr)
 { 
         asm volatile("invlpg (%0)" : : "r" (addr) : "memory");
 }  

 static __inline void
 lldt(unsigned short sel)
 {
 	asm volatile("lldt %0" : : "r" (sel));
 }

 static __inline void
 ltr(unsigned short sel)
 {
 	asm volatile("ltr %0" : : "r" (sel));
 }

 static inline void
 tlbflush(void)
 {
	unsigned int val;
 	val = read_cr3();
 	write_cr3(val);
 }

 static inline void
 tlbflushg(void)
 {
 	/*
 	 * Big hammer: flush all TLB entries, including ones from PTE's
 	 * with the G bit set.  This should only be necessary if TLB
 	 * shootdown falls far behind.
 	 *
 	 * Intel Architecture Software Developer's Manual, Volume 3,
 	 *	System Programming, section 9.10, "Invalidating the
 	 * Translation Lookaside Buffers (TLBS)":
 	 * "The following operations invalidate all TLB entries, irrespective
 	 * of the setting of the G flag:
 	 * ...
 	 * "(P6 family processors only): Writing to control register CR4 to
 	 * modify the PSE, PGE, or PAE flag."
 	 *
 	 * (the alternatives not quoted above are not an option here.)
 	 *
 	 * If PGE is not in use, we reload CR3 for the benefit of
 	 * pre-P6-family processors.
 	 */

 #if defined(I686_CPU)
 	if (cpu_feature & CPUID_PGE) {
 		u_int cr4 = rcr4();
 		lcr4(cr4 & ~CR4_PGE);
 		lcr4(cr4);
 	} else
 #endif
 		tlbflush();
 }

 static inline unsigned int
 rdr6(void)
 {
	unsigned int val;

 	asm volatile("movl %%dr6,%0" : "=r" (val));
 	return val;
 }

 static inline void
 ldr6(unsigned int val)
 {

 	asm volatile("movl %0,%%dr6" : : "r" (val));
 }

 static inline u_int64_t
 rdmsr(unsigned int msr)
 {
 	u_int64_t rv;

 	asm volatile("rdmsr" : "=A" (rv) : "c" (msr));
 	return (rv);
 }

 static inline void
 wrmsr(unsigned int msr, u_int64_t newval)
 {
 	asm volatile("wrmsr" : : "A" (newval), "c" (msr));
 }

 static inline void
 wbinvd(void)
 {
 	asm volatile("wbinvd");
 }

 static inline u_int64_t
 rdtsc(void)
 {
 	u_int64_t rv;

 	asm volatile("rdtsc" : "=A" (rv));
 	return (rv);
 }

 static inline u_int64_t
 rdpmc(unsigned int pmc)
 {
 	u_int64_t rv;

 	asm volatile("rdpmc" : "=A" (rv) : "c" (pmc));
 	return (rv);
 }
 
 #define raise_exception(n) do {asm("int $"#n"\n\t");}while(0)
 
#endif /*ASM_H_*/

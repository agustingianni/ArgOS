#ifndef TLB_H_
#define TLB_H_

#define tlb_flush()								\
	do {										\
		unsigned int tmpreg;					\
												\
		__asm__ __volatile__(					\
			"movl %%cr3, %0;              \n"	\
			"movl %0, %%cr3;  # flush TLB \n"	\
			: "=r" (tmpreg)						\
			:: "memory");						\
	} while (0)

/*
 * Global pages have to be flushed a bit differently. Not a real
 * performance problem because this does not happen often.
 */
#define tlb_flush_global()									\
	do {													\
		unsigned int tmpreg, cr4, cr4_orig;					\
															\
		__asm__ __volatile__(								\
			"movl %%cr4, %2;  # turn off PGE     \n"		\
			"movl %2, %1;                        \n"		\
			"andl %3, %1;                        \n"		\
			"movl %1, %%cr4;                     \n"		\
			"movl %%cr3, %0;                     \n"		\
			"movl %0, %%cr3;  # flush TLB        \n"		\
			"movl %2, %%cr4;  # turn PGE back on \n"		\
			: "=&r" (tmpreg), "=&r" (cr4), "=&r" (cr4_orig)	\
			: "i" (~X86_CR4_PGE)							\
			: "memory");									\
	} while (0)

#define tlb_flush_single(addr) 									\
	__asm__ __volatile__("invlpg (%0)" ::"r" (addr) : "memory")

#define tlb_flush_all()				\
	do {							\
		if (cpu_has_pge)			\
			__flush_tlb_global();	\
		else						\
			__flush_tlb();			\
	} while (0)

/*
 * TLB flushing:
 *
 *  - flush_tlb() flushes the current mm struct TLBs, Performing a process switch
 *  - flush_tlb_all() flushes all processes TLBs, Changing the kernel page table entries
 *  - flush_tlb_mm(mm) flushes the specified mm context TLB's, Forking a new process
 *  - flush_tlb_page(vma, vmaddr) flushes one page, Processing a Page Fault
 *  - flush_tlb_range(vma, start, end) flushes a range of pages, Releasing a linear address interval of a process
 *  - flush_tlb_kernel_range(start, end) flushes a range of kernel pages, Changing a range of kernel page table entries
 *  - flush_tlb_pgtables(mm, start, end) flushes a range of page tables, Releasing some page tables of a process
 *  - flush_tlb_others(cpumask, mm, va) flushes a TLBs on other cpus
 *
 * ..but the i386 has somewhat limited tlb flushing capabilities,
 * and page-granular flushes are available only on i486 and up.
 */

#define TLB_FLUSH_ALL	0xffffffff
#if 0
static inline void flush_tlb_mm(struct mm_struct *mm)
{
	if (mm == current->active_mm)
		__flush_tlb();
}

static inline void flush_tlb_page(struct vm_area_struct *vma,
	unsigned long addr)
{
	if (vma->vm_mm == current->active_mm)
		__flush_tlb_one(addr);
}

static inline void flush_tlb_range(struct vm_area_struct *vma,
	unsigned long start, unsigned long end)
{
	if (vma->vm_mm == current->active_mm)
		__flush_tlb();
}
#endif
#endif /*TLB_H_*/

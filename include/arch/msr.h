#ifndef MSR_H_
#define MSR_H_

/*
 * MSR = machine-specific registers 
 */

/*
 * Loads the contents of a 64-bit model specific register (MSR)
 * specified in the ECX register into registers EDX:EAX
 */

#define rdmsr(msr,val1,val2) \
	__asm__ __volatile__("rdmsr" \
			  : "=a" (val1), "=d" (val2) \
			  : "c" (msr))

/*
 * Writes the contents of registers EDX:EAX into the 64-bit 
 * model specific register (MSR) specified in the ECX register.
 * The high-order 32 bits are copied from EDX and the low-order 
 * 32 bits are copied from EAX.
 */
#define wrmsr(msr,val1,val2) \
	__asm__ __volatile__("wrmsr" \
			  : /* no outputs */ \
			  : "c" (msr), "a" (val1), "d" (val2))

#define rdmsrl(msr,val) do { \
	unsigned long l__,h__; \
	rdmsr (msr, l__, h__);  \
	val = l__;  \
	val |= ((u64)h__<<32);  \
} while(0)

static inline void wrmsrl(unsigned long msr, unsigned long long val)
{
	unsigned long lo, hi;
	lo = (unsigned long) val;
	hi = val >> 32;
	wrmsr (msr, lo, hi)
;}

/* wrmsr with exception handling */
#define wrmsr_safe(msr,a,b) ({ int ret__;						\
	asm volatile("2: wrmsr ; xorl %0,%0\n"						\
		     "1:\n\t"								\
		     ".section .fixup,\"ax\"\n\t"					\
		     "3:  movl %4,%0 ; jmp 1b\n\t"					\
		     ".previous\n\t"							\
 		     ".section __ex_table,\"a\"\n"					\
		     "   .align 4\n\t"							\
		     "   .long 	2b,3b\n\t"						\
		     ".previous"							\
		     : "=a" (ret__)							\
		     : "c" (msr), "0" (a), "d" (b), "i" (-EFAULT));\
	ret__; })

/* rdmsr with exception handling */
#define rdmsr_safe(msr,a,b) ({ int ret__;						\
	asm volatile("2: rdmsr ; xorl %0,%0\n"						\
		     "1:\n\t"								\
		     ".section .fixup,\"ax\"\n\t"					\
		     "3:  movl %4,%0 ; jmp 1b\n\t"					\
		     ".previous\n\t"							\
 		     ".section __ex_table,\"a\"\n"					\
		     "   .align 4\n\t"							\
		     "   .long 	2b,3b\n\t"						\
		     ".previous"							\
		     : "=r" (ret__), "=a" (*(a)), "=d" (*(b))				\
		     : "c" (msr), "i" (-EFAULT));\
	ret__; })

/*
 * The instruction returns a 64-bit value in registers EDX:EAX
 * that represents the count of ticks from processor reset.
 */
#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define rdtscl(low) \
     __asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")

#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))

#define write_tsc(val1,val2) wrmsr(0x10, val1, val2)

/*
 * This is a native instruction which reads 
 * the P6 performance monitor counters.
 */
#define rdpmc(counter,low,high) \
     __asm__ __volatile__("rdpmc" \
			  : "=a" (low), "=d" (high) \
			  : "c" (counter))

#endif /*MSR_H_*/

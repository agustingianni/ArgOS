#ifndef APIC_H_
#define APIC_H_

#include <arch/cpuid.h>

#define LAPIC_BASE	0xfee00000	/* base address of LAPIC 				*/

#define LAPIC_ID		0x20	/* ID Register 							*/
#define LAPIC_LVT		0x30	/* Version register						*/
#define LAPIC_TPR		0x80	/* Tark Priority register 				*/
#define LAPIC_APR		0x90	/* Arbitration Priority register 		*/
#define LAPIC_PPR		0xA0	/* Processor priority register			*/
#define LAPIC_EOIR		0x80	/* EOI Register 						*/
#define LAPIC_LDR		0xD0	/* Logical Destination Register 		*/
#define LAPIC_DFR		0xE0	/* Destination Format Register 			*/
#define LAPIC_SIVR		0xF0	/* Spurious Interrupt Vector Register 	*/
#define LAPIC_ESR		0x280	/* Error Status Register 				*/
#define LAPIC_ICR0		0x300	/* Interrupt Command Register 			*/
#define LAPIC_ICR1		0x310	/* Interrupt Command Register 			*/

#define LAPIC_LVT_TIMER	0x320	/* LVT Timer Register 					*/
#define LAPIC_LVT_TS	0x330	/* LVT Thermal Sensor Register 			*/
#define LAPIC_LVT_PMC	0x340	/* LVT Performance Mon counter			*/
#define LAPIC_LVT_LINT0	0x350	/* LVT LINT0 Register 					*/
#define LAPIC_LVT_LINT1	0x360	/* LVT LINT1 Register 					*/
#define LAPIC_LVT_ERR	0x370	/* LVT Error Register 					*/
#define LAPIC_LVT_ICR	0x380	/* Initial Count Register (for Timer) 	*/
#define LAPIC_LVT_CCR	0x390	/* Current Count Register (for Timer) 	*/
#define LAPIC_LVT_DCR	0x3e0	/* Divide Conf Register (for Timer) 	*/

/*
 * TODO: Check if CPUID is present 
 */
#if 0
static inline int apic_present(void)
{
	unsigned int cpu[4] = {1,0,0,0};
	cpuid(&cpu[0], &cpu[1], &cpu[2], &cpu[3]);
	
	/*     EDX */
	return (cpu[3] & (1 << 9));
}
#endif

static inline void apic_write
(
	unsigned long reg,
	unsigned long v
)
{
	*((volatile unsigned long *)(LAPIC_BASE+reg)) = v;
}

static inline unsigned long apic_read(unsigned long reg)
{
	return *((volatile unsigned long *)(LAPIC_BASE+reg));
}

static inline int apic_get_id(void)
{
	return apic_read(LAPIC_ID) & 0xff000000;
}

static inline int apic_get_version(void)
{
	return apic_read(LAPIC_LVT) & 0xff;
}

static inline int apic_get_maxlvt(void)
{
	return (apic_read(LAPIC_LVT) >> 16) & 0xff;
}

#endif /*APIC_H_*/

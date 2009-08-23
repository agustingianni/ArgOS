/*
 * File:   bitops.h
 * Author: gr00vy
 *
 * Created on May 19, 2008, 7:25 PM
 */

#ifndef _BITOPS_H
#define	_BITOPS_H

/* TODO: Fijarse donde hay que usar lock */

/*
 * Selects the bit in a bit string (specified with the first operand, called
 * the bit base) at the bit-position designated by the bit offset
 * (specified by the second operand) and stores the value of the bit
 * in the CF flag.
 */
static inline int
bit_test(unsigned int bitno, volatile unsigned long *addr)
{
	/*
	 * bt testea el bit y pone el resultado en el flag de carry (C)
	 * Ahora pasandole el mismo argumento a la funcion
	 * sbb lo que hacemos es restar dos numeros iguales
	 * lo que nos va a dar 0. luego a eso le restamos el bit
	 * de carry, si es 0 el carry flag el resultado va a ser 0
	 * y si era distinto de 0 va a ser -1.
	 */
    
	unsigned int oldbit;
	
    	asm volatile
	(
		"btl %2, %1\n\t"
		"sbbl %0, %0"
		:"=r" (oldbit)
		:"m" (*(volatile long *) addr), "Ir" (bitno)
		/* I especifica que el numero que va a ser
		 * cargado en el registro 'r' esta entre 0 .. 31 */
	);
		
	return oldbit;
}

/*
 * Selects the bit in a bit string (specified with the first operand, called the bit base) at
 * the bit-position designated by the bit offset operand (second operand), stores the
 * value of the bit in the CF flag, and complements the selected bit in the bit string. The
 * bit base operand can be a register or a memory location; the bit offset operand can
 * be a register or an immediate value.
 */
static inline unsigned int
bit_test_complement(unsigned int bitno, volatile unsigned long *addr)
{
	unsigned int oldbit;

	/* Aca usamos el mismo truco que arriba */
	asm volatile
	(
		"btcl %2, %1\n\t"
		"sbbl %0, %0"
		: "=r" (oldbit), "+m" (*(volatile unsigned long *) addr)
		: "Ir" (bitno)
		/* +m Especifica que el operando en este caso 'addr' es leido y escrito
		 * por la instruccion */
	);

	return oldbit;
}

static inline void
bit_complement(unsigned int bitno, volatile unsigned long *addr)
{
	asm volatile
	(
		"btcl %1, %0\n\t"
		: "=m" (*(volatile unsigned long *) addr)
		: "Ir" (bitno)
	);
}

/*
 * Selects the bit in a bit string (specified with the first operand, called the bit base) at
 * the bit-position designated by the bit offset operand (second operand), stores the
 * value of the bit in the CF flag, and clears the selected bit in the bit string to 0. The bit
 * base operand can be a register or a memory location; the bit offset operand can be a
 * register or an immediate value
 */
static inline unsigned int
bit_test_reset(unsigned int bitno, volatile unsigned long *addr)
{
	unsigned int oldbit;

	asm volatile
	(
		"btrl %2, %1\n\t"
		"sbbl %0, %0"
		: "=r" (oldbit), "+m" (*(volatile unsigned long *) addr)
		: "Ir" (bitno)
	);

	return oldbit;
}

static inline void
bit_reset(unsigned int bitno, volatile unsigned long *addr)
{
	asm volatile
	(
		"btrl %1, %0\n\t"
		: "=m" (*(volatile unsigned long *) addr)
		: "Ir" (bitno)
	);
}

/*
 * Selects the bit in a bit string (specified with the first operand, called the bit base) at
 * the bit-position designated by the bit offset operand (second operand), stores the
 * value of the bit in the CF flag, and sets the selected bit in the bit string to 1. The bit
 * base operand can be a register or a memory location; the bit offset operand can be a
 * register or an immediate value
 */
static inline unsigned int
bit_test_set(unsigned int bitno, volatile unsigned long *addr)
{
	unsigned int oldbit;

	asm volatile
	(
		"btsl %2, %1\n\t"
		"sbbl %0, %0"
		: "=r" (oldbit), "+m" (*(volatile unsigned long *) addr)
		: "Ir" (bitno)
	);

	return oldbit;
}

static inline void 
bit_set(unsigned int bitno, volatile unsigned long * addr)
{
        asm volatile
	(
		"btsl %1,%0"
                :"+m" (*(volatile unsigned long *) addr)
                :"Ir" (bitno)
	);
}


#endif	/* _BITOPS_H */


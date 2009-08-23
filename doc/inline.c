#include <stdio.h>
#include <unistd.h>

static void pepe2(void)
{
	puts("pepe2");
}

static void pepe1(void)
{
	puts("pepe1");
}

static double cosine(double x)
{
	double result;
	asm
	( 	"fcos"
		: "=t" (result)
		: "0" (x)
	); 

	return result;
}

#include <string.h>


int
main(int argc, char **argv)
{
	int value = 0;
	int another = 31;

	char intel[12+1];
	
	printf("hola loco");
	
	memset((void *) intel, 0x00, sizeof(intel));

	/*
	 * Get the CPUID
	 * void cpuid(void);
	 * @see http://download.intel.com/design/Xeon/applnots/24161831.pdf
	 */
	int query_no = 0;
	int outvalue = 0;
	cpuid
	(
	        &query_no,
	        (unsigned int *) &intel[0], 
	        (unsigned int *) &intel[8], 
	        (unsigned int *) &intel[4]
	);

	printf("Returned string = %s ; eax = %d\n", intel, outvalue);

	query_no = 1;
	cpuid
	(
		&query_no, 			/* EAX */
		(unsigned int *) &intel[0], 	/* EBX */
		(unsigned int *) &intel[4],	/* ECX */
		(unsigned int *) &intel[8]	/* EDX */
	);

	printf
	(
		"Extended family  %d\n" \
		"Extended model   %d\n" \
		"Processor type   %d\n" \
		"Family Code      %d\n" \
		"Model Number     %d\n" \
		"SteppingID       %d\n",
		CPUID_EXTENDED_FAMILY(query_no),
		CPUID_EXTENDED_MODEL(query_no),
		CPUID_PROCESSOR_TYPE(query_no),
		CPUID_FAMILY_CODE(query_no),
		CPUID_MODEL_NUMBER(query_no),
		CPUID_STEPPING_ID(query_no)
	);

	return 0;

	/* Clear the direction flag
	 * void cld(void);
	 */
	asm
	(
		"cld\n\t"
	);

	/* Clear the interrupt flag
	 * void cli(void);
	 */
	asm
	(
		"cli\n\t"
	);

	/* void set_bit(unsigned long *bitstring, unsigned int n);*/
	asm
	(
		"btsl %1, %0\n\t"
		: "=m" (value)
		: "Ir"  (another)
		: "cc"
	);

	/* void byte_swap(unsigned long *byte);*/
	asm
	(
		"bswap %0\n\t"
		: "=r" (value)
		: "0" (value)
	);

	/* static inline void clear_bit(int nr, volatile unsigned long * addr) */
	asm
	(
		"btrl %1,%0"
		:"=m" (value)
		:"r" (0)
	);

	return 0;
}

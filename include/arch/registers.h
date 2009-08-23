#ifndef REGISTERS_H_
#define REGISTERS_H_

/*
 * NOTE: Aca vamos a poner todas las macros
 * y definiciones de funcioens que accedan a 
 * registros de uso especifico de la arquitecura
 * en este caso sera IA-32 pero para mas adelante
 * deberiamos pensar en aislarlo de alguna manera para
 * poner definiciones de otras arquitecturas
 */

 /*
  * Register: cr0 (control register 0) Machine Status Word
  * Default Status after reboot: 0x60000010
  */

 #define CR0_PE	0x00000001	/* Protected mode Enable */
 #define CR0_MP	0x00000002	/* "Math" Present (NPX or NPX emulator) */
 #define CR0_EM	0x00000004	/* EMulate non-NPX coproc. (trap ESC only) */
 #define CR0_TS	0x00000008	/* Task Switched (if MP, trap ESC and WAIT) */
 #define CR0_ET	0x00000010	/* Extension Type (387 (if set) vs 287) */
 #define CR0_PG	0x80000000	/* PaGing enable */
 #define CR0_NE	0x00000020	/* Numeric Error enable (EX16 vs IRQ13) */
 #define CR0_WP	0x00010000	/* Write Protect (honor PG_RW in all modes) */
 #define CR0_AM	0x00040000	/* Alignment Mask (set to enable AC flag) */
 #define CR0_NW	0x20000000	/* Not Write-through */
 #define CR0_CD	0x40000000	/* Cache Disable */

/*
 * Control register 0 (CR0)
 * 
 * PG:
 * disables or enables the processor's paging
 * capability.
 * ----------------------------------------------------
 * NE:
 * Available from i486
 * Numeric Exception
 * 
 * The state of this bit tells the processor
 * the action to take in the event of a floating-point
 * exception
 * 
 * 0 = the processor asserts its FERR# output
 * 1 = the processor generates an exception
 * ----------------------------------------------------
 * ET:
 * tells the 386 processor whether the
 * attached x87 numeric coprocessor is a 287 or a 387
 * 
 * For i486 >= is always set to 1 because they do not
 * support 287 numeric processor
 * ----------------------------------------------------
 * WP:
 * When set, denies the OS permission to write to pages
 * marked read-only.
 * ----------------------------------------------------
 * AM:
 * When set, the AC (Alignment Check) bit in the
 * EFLAGS register controls whether or not an AC exception
 * is generated when an attempt is made to perform
 * a non-aligned memory access.
 */

 /*
  * Control register 1 (CR1)
  * 
  * Not implemented in any processor
  * it is set to 0 after reboot
  */

 /*
   * Control register 2 (CR2)
   * 
   * Used if the processor is in protected mode
   * and paging has been enabled.
   * 
   * The processor uses CR2 to store the 32-bit
   * linear address that caused a page fault exception.
   */

 /*
   * Control register 3 (CR3)
   * 
   * CR3 has no effect unless the processor 
   * is in protectedmode with paging enabled
   * 
   * CR3 contains the 4KB-aligned physical base 
   * address of the Page Directory in memory.
   * 
   *  31-11                   10-5  4    3    2-0
   * [Page Dir Base address][     ][PCD][PWT][   ]
   */

  #define CR3_PAGE_DIR_BASE 	(0xfffff000)
  #define CR3_PCD 			(1 << 4)	/* Page cache disable */
  #define CR3_PWT 			(1 << 3)	/* Page write through */
  
  /*
   * Control register 4 (CR4)
   * 
   * Introduced with early Pentium processor and later i486
   * 
   * bits 31-7 reserved
   */
  
  #define CR4_VME 0x00000001	/* virtual 8086 mode extension enable */
  #define CR4_PVI 0x00000002	/* protected mode virtual interrupt enable */
  #define CR4_TSD 0x00000004	/* restrict RDTSC instruction to cpl 0 only */
  #define CR4_DE  0x00000008	/* debugging extension */
  #define CR4_PSE 0x00000010	/* large (4MB) page size enable */
  #define CR4_PAE 0x00000020	/* physical address extension enable */
  #define CR4_MCE 0x00000040	/* machine check enable */
  #define CR4_PGE 0x00000080	/* page global enable */
  #define CR4_PCE 0x00000100	/* enable RDPMC instruction for all cpls */
  #define CR4_OSFXSR		0x00000200	/* enable fxsave/fxrestor and SSE */
  #define CR4_OSXMMEXCPT	0x00000400	/* enable unmasked SSE exceptions */
  

  /*
   * EFLAGS
   * bit:   0  1 2  3 4  5 6  7  8  9  A  B  C/D  E  F 10 11 12 13  14  15 16
   * flag:  CF 1 PF 0 AF 0 ZF SF TF IF DF OF IOPL NT 0 RF VM AC VIF VIP ID 0
   */
  
  typedef struct 
  {
	  unsigned cf:1;
	  unsigned R1:1;
	  unsigned pf:1;
	  unsigned R3:1;
	  unsigned af:1;
	  unsigned R5:1;
	  unsigned zf:1;
	  unsigned sf:1;
	  unsigned tf:1;
	  unsigned if_:1;
	  unsigned df:1;
	  unsigned of:1;
	  unsigned iopl:2;
	  unsigned nt:1;
	  unsigned R15:1;
	  unsigned rf:1;
	  unsigned vm:1;
	  unsigned ac:1;
	  unsigned vif:1;
	  unsigned vip:1;
	  unsigned id:1;
	  unsigned R31_22:10;
  } __attribute__ ((packed)) eflags_t;

  #define EFLAGS_CF (1 << 0) 	/* Carry flag */
  // bit 1 its just a fixed 1 value
  #define EFLAGS_PF (1 << 2) 	/* Parity flag */
  // bit 3 its just a fixed 0 value
  #define EFLAGS_AF (1 << 4) 	/* Auxiliary carry flag */
  // bit 5 its just a fixed 0 value
  #define EFLAGS_ZF (1 << 6)	/* Zero flag*/
  #define EFLAGS_SF (1 << 7)	/* Sign flag */
  #define EFLAGS_TF (1 << 8)	/* Trap flag  1 = Single step mode */
  #define EFLAGS_IF (1 << 9) 	/* Interrupt flag 1 = Int enabled */
  #define EFLAGS_DF (1 << 10)	/* Directionflag 0 = inc 1 = dec*/
  #define EFLAGS_OF (1 << 11)	/* Overflow flag */
  #define EFLAGS_IOPL (3 << 12)	/* IO Privilege level */
  /* 
   * Any interrupt that selects a task gate in the
   * interrupt descriptor table (IDT) causes a task switch to the
   * task that services the interrupt. In this case, the processor
   * sets the NT bit.
   */
  #define EFLAGS_NT (1 << 14)	/* Nested task */
  // bit 15 its just a fixed 0 value
  #define EFLAGS_RF (1 << 16)	/* Resume flag */
  #define EFLAGS_VM (1 << 17)	/* Virtual 8086 Mode*/
  /* Controls whether or not an AC exception 
   *
   * is generated when an attempt is made 
   * to perform a non-aligned memory access.
   */
  #define EFLAGS_AC  (1 << 18)	/* Aligment check */
  #define EFLAGS_VIF (1 << 19)	/* Aligment check */
  #define EFLAGS_VIP (1 << 20)	/* Aligment check */
  #define EFLAGS_ID  (1 << 21)	/* Whether or not the cpuid instruction is enabled */  
 #endif /*REGISTERS_H_*/

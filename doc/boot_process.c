Todo empieza aca:
linux/arch/i386/kernel/head.S

	/** Reseteamos el flag de direccion (GCC lo necesita asi)*/
	cld

	/**
	   Carga una GDT provisoria, es importante notar
	   que aca esta "corrigiendo" la direccion de
	   boot_gdt_descr por que esa es una direccion virtual con base
	   en 0xc0000000 (3GB), y como lgdt necesita la direccion fisica
	   necesitamos restarle un offset
	*/
	lgdt boot_gdt_descr - __PAGE_OFFSET

	/**
	   Cargar todos los selectores de datos con el descriptor de datos
	   de ring0
	*/
	movl $(__BOOT_DS),%eax
	movl %eax,%ds
	movl %eax,%es
	movl %eax,%fs
	movl %eax,%gs

	/** Seteamos a 0x00 toda la seccion .bss */
	call clean_bss

	/** Inicializamos la tabla de paginas */
	call init_page_table

	/** Habilitamos la paginacion */
	movl $swapper_pg_dir-__PAGE_OFFSET,%eax
	movl %eax,%cr3		/* set the page table pointer.. */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		/* ..and set paging (PG) bit */
	ljmp $__BOOT_CS,$1f	/* Clear prefetch and normalize %eip */

	1:
	/** Set up the stack pointer */
	lss stack_start,%esp

	/**
	    Initialize eflags.  Some BIOS's leave bits like NT set.  This would
	    confuse the debugger if this code is traced.
	 */
	pushl $0
	popfl

	/**
	    Arma la IDT (la final), con las 256 entradas apuntando a una call gate
	    que llama a la function ignore_int.

	    Algunas exceptions apuntan a manejadores "tempranos"
	    Ellas son:

	    set_early_handler handler=early_divide_err,trapno=0
	    set_early_handler handler=early_illegal_opcode,trapno=6
	    set_early_handler handler=early_protection_fault,trapno=13
	    set_early_handler handler=early_page_fault,trapno=14

	    Lo unico que hacen estas funciones es imprimir un error,
	    y haltear la maquina.

	    Esta funcion NO carga la IDT.
	 */
	call setup_idt

	/** Inicializamos a -1 (CPUID no habilitada) el flag */
	movl $-1,X86_CPUID		#  -1 for no CPUID initially

	/** Checkear si estamos en i386 o i486*/
	movb $3,X86		# at least 386
	pushfl			# push EFLAGS
	popl %eax		# get EFLAGS
	movl %eax,%ecx		# save original EFLAGS
	xorl $0x240000,%eax	# flip AC and ID bits in EFLAGS
	pushl %eax		# copy to EFLAGS
	popfl			# set EFLAGS
	pushfl			# get new EFLAGS
	popl %eax		# put it in eax
	xorl %ecx,%eax		# change in flags
	pushl %ecx		# restore original EFLAGS
	popfl
	testl $0x40000,%eax	# check if AC bit changed

	/** Si el flag AC no cambio es por que estamos en i386 */
	je is386

	/** Por lo menos es i486 */
	movb $4,X86		# at least 486
	testl $0x200000,%eax	# check if ID bit changed

	/** Si el flag AD cambio es por que estamos en i486 */
	je is486

	/** Ahora sabemos que tenemos la instruction CPUID asi que la usamos */
	xorl %eax,%eax			# call CPUID with 0 -> return vendor ID
	cpuid
	movl %eax,X86_CPUID		# save CPUID level
	movl %ebx,X86_VENDOR_ID		# lo 4 chars
	movl %edx,X86_VENDOR_ID+4	# next 4 chars
	movl %ecx,X86_VENDOR_ID+8	# last 4 chars

	orl %eax,%eax			# do we have processor info as well?
	je is486

	movl $1,%eax		# Use the CPUID instruction to get CPU type
	cpuid
	movb %al,%cl		# save reg for future use
	andb $0x0f,%ah		# mask processor family
	movb %ah,X86
	andb $0xf0,%al		# mask model
	shrb $4,%al
	movb %al,X86_MODEL
	andb $0x0f,%cl		# mask mask revision
	movb %cl,X86_MASK
	movl %edx,X86_CAPABILITY

	/**
	    Estamos en un i486 asi que podemos setear los flags

		AM: ALIGMENT MASK (EL procesador genera excepciones de alineacion)
		WP: WRITE PROTECT (Denegar escritura a paginas de solo lectura)
		NW: NUMERIC ERROR
		MP: ?
	 */
	is486:
	movl $0x50022,%ecx	# set AM, WP, NE and MP
	jmp 2f

	is386:
	movl $2,%ecx		# set MP

	2:
	movl %cr0,%eax
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl %ecx,%eax
	movl %eax,%cr0

	/** Verifica si tenemos o no coprocesador matematico */
	call check_x87
	{
		check_x87:
		movb $0,X86_HARD_MATH
		clts
		fninit
		fstsw %ax
		cmpb $0,%al
		je 1f

		/** No tenemos coprocesador */
		movl %cr0,%eax

		/**
		    Cuando esta habilitado el bit EM (Emulate) el
		    procesador va a arrojar una exception cada ves que se
		    utiliza una instruccion de FPU
		 */
		xorl $4,%eax		/* set EM */
		movl %eax,%cr0
		ret

		/** Tenemos coprocesador */
		ALIGN
		1:	
		movb $1,X86_HARD_MATH

		/**
		    fsetpm setea el FPU en modo protegido, si tenemos el 387 es
		    ignorado
		 */
		.byte 0xDB,0xE4		/* fsetpm for 287, ignored by 387 */
		ret
	}

	/**
	    TODO: Ver bien para que se usan las PDA's

	    Setea una PDA (private data area) para el CPU
	    esta PDA es una entrada en la GDT y el
	    descriptor se carga en el selector fs
	 */
	call setup_pda

	/** Cargamos la GDT (no es todavia la final) */
	lgdt early_gdt_descr

	/** Cargamos el IDT (LA idt final) */
	lidt idt_descr

	/** Limpiamos la "prefetch queue" */
	ljmp $(__KERNEL_CS),$1f

	1:
	/** Recargamos todos los selectores de segmento */
	movl $(__KERNEL_DS),%eax	# reload all the segment registers
	movl %eax,%ss			# after changing gdt.

	movl $(__USER_DS),%eax		# DS/ES contains default USER segment
	movl %eax,%ds
	movl %eax,%es

	xorl %eax,%eax			# Clear GS and LDT
	movl %eax,%gs
	lldt %ax

	movl $(__KERNEL_PDA),%eax
	mov  %eax,%fs

	cld			# gcc2 wants the direction flag cleared at all times
	pushl $0		# fake return address for unwinder
	jmp start_kernel












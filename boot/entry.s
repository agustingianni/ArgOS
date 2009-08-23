; The person who say it cannot be done should not interrupt person doing it.  
; -Chinese Proverb 

global _start
global start
extern kernel_main
extern KERN_BSS_END
extern KERN_DATA_END
extern KERN_TEXT_BEGIN

MODULEALIGN  equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO      equ  1<<1                   ; provide memory map
FLAGS        equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC        equ  0x1BADB002             ; 'magic number' lets bootloader find the header
CHECKSUM     equ -(MAGIC + FLAGS)        ; checksum required

STACKSIZE    equ 0x4000                  ; 16k.
FRAME_BUFFER equ 0xB8000

section .text
	align 4

	MultiBootHeader:
		dd MAGIC
		dd FLAGS
		dd CHECKSUM
	
		; These fields are for non ELF kernels
		; We wont be using them but ...
		dd MultiBootHeader	; these are PHYSICAL addresses
   		dd KERN_TEXT_BEGIN	; start of kernel .text section
		dd KERN_DATA_END	; end of kernel .data section
		dd KERN_BSS_END		; end of kernel .bss section
		dd start			; kernel entry point
	
	start:
	_start:
		lgdt [trickgdt]
		mov cx, 0x10
		mov ds, cx
		mov es, cx
		mov fs, cx
		mov gs, cx
		mov ss, cx

		; jump to the higher half kernel
		jmp 0x08:higherhalf

		higherhalf:

		; create an stack because C uses the stack 
		; to allocate local variables and also the 
		; call instruction needs a stack
		mov esp, stack+STACKSIZE 	; set up the stack

		; reset EFLAGS
		push 0
		popf

		; GCC Needs the Direction Flag set to 0
		cld

		; Disable interrupts since we cannot handle them now
		cli

		; we pass some parameters to kernel_main()
		; add ebx, 0xc0000000			; phys_to_virtual :)
		push ebx 					; struct multiboot_information
		push eax 					; MULTIBOOT_INFO_MAGIC
		
		; void
		; kernel_main
		; (
		;   uint32_t magic,
		;   struct multiboot_information *mb_info
		; )
		call kernel_main			; call kernel main C routine
		jmp $

; lgdt need the physical address of the GDT so we link the
; .setup section with identical virtual and physical addresses. 
section .setup
	trickgdt:
		dw gdt_end - gdt - 1 ; size of the GDT
		dd gdt ; linear address of GDT

	gdt:
		dd 0, 0							; null gate
		; code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
		db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40
	
		; data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
		db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40

	gdt_end:

section .bss
	align 32
	stack:
	  	resb STACKSIZE

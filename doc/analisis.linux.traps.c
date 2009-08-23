/* Setear el handler */
set_trap_gate(0, &divide_error);

static void __init set_trap_gate(unsigned int n, void *addr)
{
	/**
	  * DESCTYPE_TRAP es el tipo de gate, addr es 
	  * el handler, __KERNEL_CS es el selector donde
	  * va a estar el codigo
	  */
	_set_gate(n, DESCTYPE_TRAP, addr, __KERNEL_CS);
}

#define DESCTYPE_TRAP	0x8f	/* present, system, DPL-0, trap gate */

#define GDT_ENTRY_KERNEL_BASE	12
#define GDT_ENTRY_KERNEL_CS		(GDT_ENTRY_KERNEL_BASE + 0)
#define __KERNEL_CS (GDT_ENTRY_KERNEL_CS * 8)

static inline void _set_gate(int gate, unsigned int type, void *addr, unsigned short seg)
{
	__u32 a, b;
	pack_gate(&a, &b, (unsigned long)addr, seg, type, 0);
	write_idt_entry(idt_table, gate, a, b);
}

/** El bloque fundamental de cada entrada en la IDT */
struct desc_struct {
	unsigned long a,b;
};

/** Esta es la IDT, tiene 256 entradas y va a ser cargada por lidt */
struct desc_struct idt_table[256] __attribute__((__section__(".data.idt"))) = { {0, 0}, };

/** Formatea de forma correcta una entrada para instalar en la IDT */
static inline void pack_gate(__u32 *a, __u32 *b,
	unsigned long base, unsigned short seg, unsigned char type, unsigned char flags)
{
	*a = (seg << 16) | (base & 0xffff);
	*b = (base & 0xffff0000) | ((type & 0xff) << 8) | (flags & 0xff);
}

/** en este caso dt = idt_table */
#define write_idt_entry(dt, entry, a, b) write_dt_entry(dt, entry, a, b)
static inline void write_dt_entry(void *dt, int entry, __u32 entry_a, __u32 entry_b)
{
	/** Calcular el indice dentro de la idt escalando entry por 8 */
	__u32 *lp = (__u32 *)((char *)dt + entry*8);

	/** seteamos los valores de la entrada en la IDT */
	*lp = entry_a;
	*(lp+1) = entry_b;
}

/** Ahora vamos a analizar el handler de Divide by Zero que seteamos
    con set_trap_gate */

ENTRY(divide_error)
	RING0_INT_FRAME
	pushl $0			# no error code
	CFI_ADJUST_CFA_OFFSET 4
	pushl $do_divide_error
	CFI_ADJUST_CFA_OFFSET 4
	jmp error_code
	CFI_ENDPROC
END(divide_error)

/** here is the dissasembly of this function */
.text:08000C18                 public divide_error
.text:08000C18 divide_error:
.text:08000C18                 push    0
.text:08000C1A                 push    offset do_divide_error
.text:08000C1F                 jmp     error_code


.kprobes.text:08001090 ; START OF FUNCTION CHUNK FOR iret_exc
.kprobes.text:08001090
.kprobes.text:08001090 error_code:                             ; CODE XREF: .text:08000B63j
.kprobes.text:08001090                                         ; .text:08000B6Fj ...
.kprobes.text:08001090                 push    es
.kprobes.text:08001091                 push    ds
.kprobes.text:08001092                 push    eax
.kprobes.text:08001093                 push    ebp
.kprobes.text:08001094                 push    edi
.kprobes.text:08001095                 push    esi
.kprobes.text:08001096                 push    edx
.kprobes.text:08001097                 push    ecx
.kprobes.text:08001098                 push    ebx
.kprobes.text:08001099                 cld
.kprobes.text:0800109A                 push    gs
.kprobes.text:0800109C                 mov     ecx, 0D8h
.kprobes.text:080010A1                 mov     gs, cx
.kprobes.text:080010A3                 assume gs:nothing
.kprobes.text:080010A3                 mov     eax, ss
.kprobes.text:080010A5                 cmp     ax, 0D0h
.kprobes.text:080010A9                 jnz     short loc_80010E2

.kprobes.text:080010AB                 mov     eax, 68h
.kprobes.text:080010B0                 mov     ds, ax
.kprobes.text:080010B2                 assume ds:nothing
.kprobes.text:080010B2                 mov     es, ax
.kprobes.text:080010B4                 assume es:nothing
.kprobes.text:080010B4                 mov     ebx, large gs:4
.kprobes.text:080010BB                 mov     ebx, offset per_cpu__cpu_gdt_descr
.kprobes.text:080010C0                 mov     ebx, [ebx+2]
.kprobes.text:080010C3                 mov     al, [ebx+0D4h]
.kprobes.text:080010C9                 mov     ah, [ebx+0D7h]
.kprobes.text:080010CF                 shl     eax, 10h
.kprobes.text:080010D2                 mov     ax, [ebx+0D2h]
.kprobes.text:080010D9                 add     eax, esp
.kprobes.text:080010DB                 push    68h
.kprobes.text:080010DD                 push    eax
.kprobes.text:080010DE                 lss     esp, fword ptr [esp+38h+var_38]
.kprobes.text:080010E2
.kprobes.text:080010E2 loc_80010E2:                            ; CODE XREF: iret_exc+54j
.kprobes.text:080010E2                 pop     ecx
.kprobes.text:080010E3                 mov     edi, [esp+34h+var_10]
.kprobes.text:080010E7                 mov     edx, [esp+34h+var_C]
.kprobes.text:080010EB                 mov     [esp+34h+var_C], 0FFFFFFFFh
.kprobes.text:080010F3                 mov     [esp+34h+var_10], ecx
.kprobes.text:080010F7                 mov     ecx, 7Bh
.kprobes.text:080010FC                 mov     ds, cx
.kprobes.text:080010FE                 assume ds:nothing
.kprobes.text:080010FE                 mov     es, cx
.kprobes.text:08001100                 assume es:nothing
.kprobes.text:08001100                 mov     eax, esp
.kprobes.text:08001102                 call    edi
.kprobes.text:08001104                 jmp     ret_from_exception
.kprobes.text:08001104 ; END OF FUNCTION CHUNK FOR iret_exc


#define RING0_INT_FRAME \
	CFI_STARTPROC simple;\
	CFI_SIGNAL_FRAME;\
	CFI_DEF_CFA esp, 3*4;\
	/*CFI_OFFSET cs, -2*4;*/\
	CFI_OFFSET eip, -3*4

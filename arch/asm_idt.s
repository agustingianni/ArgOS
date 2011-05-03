extern exception_dispatcher
extern irq_dispatcher
extern syscall_dispatcher

; isr_routine(name, number, stub_addr)
; Aca pusheamos 0 por que sino en el manejador
; general de exeption/irq tendriamos que
; cambiar struct regs y sacarle el campo
; de codigo de error.
%macro CREATE_ISR 3
global %1
%1:
	push dword 0			; pusheamos un codigo de error "falso"
    push dword %2			; push the int number
    jmp %3
%endmacro

; isr_routine(name, number, stub_addr)
%macro CREATE_ISR_WITH_ERRCODE 3
global %1
%1:
    push dword %2
    jmp %3
%endmacro

%macro push_data_selectors 0
    push ds
    push es
    push fs
    push gs
%endmacro

%macro pop_data_selectors 0
    pop gs
    pop fs
    pop es
    pop ds
%endmacro

%macro load_data_selectors 1
	mov ax, %1
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
%endmacro

; common_stub(stub_name, handler)
;
%macro common_stub 2
common_stub_%1:
	; Si venimos desde RING3 a RING0 aca va a realizarse un cambio
	; de TSS. Se va a cargar SS0:ESP0, con la pila de modo
	; kernel que posee cada proceso.

	; Guardamos todos los registros en la pila actual
	; RING0
	; EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI
    pusha

    ; Guardamos los selectores tambien en la
    ; pila de RING0
	push_data_selectors

	; Ahora como estamos en RING0
	; cambiamos los selectores de datos
	; y los seteamos con la entrada del segmento
	; de datos del Kernel
	load_data_selectors 0x10

	; Aca lo que hacemos es obtener un puntero al
	; "contexto" anterior de interrupcion
    mov eax, esp
    push eax		; icontext_t *

    call %2			; call dispatcher
    pop eax

	extern task_schedule
	call task_schedule

    ; if(need_reschedule) then call task_dispatcher()
    ; task_dispatcher se va a encargar de switchear
    ; automaticamente a la pila del proceso nuevo.
    ; Por lo tanto, el contexto del proceso cargado
    ; por task_dispatcher() sera restaurado por
    ; pop_data_selectors() y popa
    ;
    ; Tambien tenemos que checkear si hay senales
    ; pendientes a enviar, si el proceso estaba siendo
    ; debuggeado usando single stepping hay que restaurar el flag
    ; en CFLAGS y tambien algo sobre el modo V8086

    ; Restauramos los selectores de datos
    ; que habiamos guardado
	pop_data_selectors	; (1)

	; Ahora restauramos los registros
    popa				; (1)

    ; Teniamos pusheado IRQNUM y un codigo de error.
    ; en realidad tendria que popearlos en CREATE_ISR
    ; pero bueh ya lo puse aca y no quiero brekear nada.
    add esp, 8

    ; Volvemos de una IRQ
    ; aca si hubo un cambio de RING3 a RING0 vamos a restaurar
    ; los valores de SS:ESP que habiamos sobreescrito con
    ; los valores de SS0:ESP0
    iret
%endmacro

; Here we define the common stubs of code
; used by the exception and irq handlers
;
common_stub isr, exception_dispatcher
common_stub irq, irq_dispatcher
common_stub syscall, syscall_dispatcher

; Define all the exception handlers, those between 19-31 are reserved
; by INTEL arch
;
CREATE_ISR h_divide_exception, 0, common_stub_isr
CREATE_ISR h_debug_exception, 1, common_stub_isr
CREATE_ISR h_nmi_exception, 2, common_stub_isr
CREATE_ISR h_int3_exception, 3, common_stub_isr
CREATE_ISR h_into_exception, 4, common_stub_isr
CREATE_ISR h_bounds_exception, 5, common_stub_isr
CREATE_ISR h_opcode_exception, 6, common_stub_isr
CREATE_ISR h_coprocessor_exception, 7, common_stub_isr
CREATE_ISR_WITH_ERRCODE h_dfault_exception, 8, common_stub_isr

; ACA PASA ALGO
CREATE_ISR h_mathoverflow_exception, 9, common_stub_isr

CREATE_ISR_WITH_ERRCODE h_badtss_exception, 10, common_stub_isr
CREATE_ISR_WITH_ERRCODE h_segnotpresent_exception, 11, common_stub_isr
CREATE_ISR_WITH_ERRCODE h_stk_fault_exception, 12, common_stub_isr
CREATE_ISR_WITH_ERRCODE h_gp_fault_exception, 13, common_stub_isr
CREATE_ISR_WITH_ERRCODE h_pg_fault_exception, 14, common_stub_isr
CREATE_ISR h_reserved_exception, 15, common_stub_isr
CREATE_ISR h_fp_exception, 16, common_stub_isr
CREATE_ISR h_align_exception, 17, common_stub_isr
CREATE_ISR h_machine_chk_exception, 18, common_stub_isr

; Reserved exceptions
CREATE_ISR h_reserved_19_exception, 19, common_stub_isr
CREATE_ISR h_reserved_20_exception, 20, common_stub_isr
CREATE_ISR h_reserved_21_exception, 21, common_stub_isr
CREATE_ISR h_reserved_22_exception, 22, common_stub_isr
CREATE_ISR h_reserved_23_exception, 23, common_stub_isr
CREATE_ISR h_reserved_24_exception, 24, common_stub_isr
CREATE_ISR h_reserved_25_exception, 25, common_stub_isr
CREATE_ISR h_reserved_26_exception, 26, common_stub_isr
CREATE_ISR h_reserved_27_exception, 27, common_stub_isr
CREATE_ISR h_reserved_28_exception, 28, common_stub_isr
CREATE_ISR h_reserved_29_exception, 29, common_stub_isr
CREATE_ISR h_reserved_30_exception, 30, common_stub_isr
CREATE_ISR h_reserved_31_exception, 31, common_stub_isr

; Define all the IRQ handlers
;
CREATE_ISR h_irq0, 0 + 32, common_stub_irq
CREATE_ISR h_irq1, 1 + 32, common_stub_irq
CREATE_ISR h_irq2, 2 + 32, common_stub_irq
CREATE_ISR h_irq3, 3 + 32, common_stub_irq
CREATE_ISR h_irq4, 4 + 32, common_stub_irq
CREATE_ISR h_irq5, 5 + 32, common_stub_irq
CREATE_ISR h_irq6, 6 + 32, common_stub_irq
CREATE_ISR h_irq7, 7 + 32, common_stub_irq
CREATE_ISR h_irq8, 8 + 32, common_stub_irq

; ACA PASA ALGO
CREATE_ISR h_irq9, 9 + 32, common_stub_irq

CREATE_ISR h_irq10, 10 + 32, common_stub_irq
CREATE_ISR h_irq11, 11 + 32, common_stub_irq
CREATE_ISR h_irq12, 12 + 32, common_stub_irq
CREATE_ISR h_irq13, 13 + 32, common_stub_irq
CREATE_ISR h_irq14, 14 + 32, common_stub_irq
CREATE_ISR h_irq15, 15 + 32, common_stub_irq

; Our syscall Handler
CREATE_ISR h_syscall, 0x80, common_stub_syscall

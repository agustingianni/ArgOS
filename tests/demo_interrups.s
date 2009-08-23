;ESTO ES UNA CROTADA AAAAAHAHHHHH!!!
BITS32
section .text
    org 7C00h
    jmp init
gaston:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    cli

    pusha
    push gs
    push fs
    push es
    push ds

    mov eax, 0xb8000

    mov byte [eax],0x47
    inc eax
    mov byte bl, [color]
    mov byte [eax], bl
    inc bl
    mov byte [color], bl
    inc eax
    mov byte [eax],0x41
    inc eax
    mov byte [eax],0x1f
    inc eax
    mov byte [eax],0x53
    inc eax
    mov byte [eax],0x1f
    inc eax
    mov byte [eax],0x54
    inc eax
    mov byte [eax],0x1f
    inc eax
    mov byte [eax],0x4F
    inc eax
    mov byte [eax],0x1f
    inc eax
    mov byte [eax],0x4E
    inc eax
    mov byte [eax],0x1f
    ;mov ecx, 0x20000000
;loop1:
    ;push ecx
    ;call __t_printk
    ;pop ecx

    ;cmp eax, 0xB8FA0
    ;jb loop1

    pop ds
    pop es
    pop fs
    pop gs
    popa

    sti

    iret
;loop2: jmp loop2


__t_printk:;void __t_printk(dword char, byte x, byte y)
    push ebp
    mov ebp, esp

    inc eax
    mov ebx, dword [ebp + 8]
    mov byte [eax], bh
    inc eax
    mov byte [eax],0x1f

    mov esp, ebp
    pop ebp
    ret

;INICIAMOS REGISTRO DE INTERRUPCIONES
init:
    ;lidt [idt_pointer]
    mov eax, dword [int0]
    mov dword [cs:9*4], eax
loop2: jmp loop2

[section .data]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; the IDT with it's descriptors
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
start_of_idt:
;interrupt 0
int0:
dw 0x7c07
dw 0x0000
;dw 0xE00
;dw 0x0000
;interrupt 1
dw 0x7c07
dw 0x00
dw 0xE00
dw 0x0000
;interrupt 2, intel reserved, we set the 'present' bit to 0 on this one
dw 0x7c07
dw 0x00
dw 0xE00
dw 0x0000

;interrupts 3-14 now, since we are making the descriptors
;identical, we are going to loop to get them all(12 total)
%rep 0xC
  dw 0x7c07
  dw 0x00
  dw 0xE00
  dw 0x0000
%endrep

;interrupt 15, intel reserved, we set the 'present' bit to 0 on this one
dw 0x7c07
dw 0x00
dw 0x0E00
dw 0x0000

;interrupt 16
dw 0x7c07
dw 0x00
dw 0xE00
dw 0x0000
end_of_idt:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; now for the IDT pointer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
idt_pointer:
  dw end_of_idt - start_of_idt - 1
  dd start_of_idt
color:
  db 0x1f
;FIN REGISTRO DE INTERRUPCIONES

times 510-($-$$) db 0  ; fill sector w/ 0's
dw 0xAA55        ; req'd by some BIOSes
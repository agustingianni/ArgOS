BITS32
section .text
	ENTRY:
    org 7C00h

    mov eax, 0x000B8000

    mov byte [eax],0x47
    inc eax
    mov byte [eax],0x1f
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
    mov ecx, 0x20000000
loop1:
    push ecx
    call __t_printk
    pop ecx

    cmp eax, 0xB8FA0
    jb loop1
loop2: jmp loop2


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
    
SIZE EQU	$-ENTRY

; validating the size of the boot setor code
%if SIZE+11+2 > 512
  %error "code is too large for boot sector"
%endif

times(512-SIZE-11-2) db 0 ; fill remaining bytes with zeroes

filename  DB	"CMDSHELLBIN"	;11 byte file name - CMDSHELL.BIN
DW     0xAA55		;2  byte boot signature
global gdt_flush

; void gdt_flush(struct region_descriptor *gp);
gdt_flush:
	mov eax, [esp+4]
    lgdt [eax]
    jmp .1
    nop
    
    .1:
    ; 0x10 is the offset in the GDT to our data segment
    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov ss, ax
    
    ; We do not have a per CPU segment to load at fs
    mov eax, 0
    mov fs, ax
    
    ; 0x08 is the offset to our code segment
    ; Here we issue a far jump to reload the CS register
    jmp 0x08:flush2
	flush2:
    ret
	
	; This is the way NetBSD loads the CS register
	;pop eax
	;push long 0x08
	;push eax
	;ret	

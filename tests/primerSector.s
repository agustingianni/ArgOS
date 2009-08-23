BITS 16

	org 0x7c00
	
	lea si, [cadena]
	call puts
	
myloop:
	jmp myloop
	
read_from_disk:
	push bp
	mov bp, sp
	
	
	
	mov sp, bp
	pop bp
	
puts:                       ; Dump ds:si to screen.
    lodsb                   ; load byte at ds:si into al
    or al,al                ; test if character is 0 (end)
    jz done
    mov ah,0eh              ; put character
    mov bx,0007             ; attribute
    int 0x10                ; call BIOS
    jmp puts
done:
    ret
    
cadena db 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA',13,10,0

times 510-($-$$) db 0
dw 0xAA55
;28 Bits LBA disk access
BITS 16
	org 0x7c00
start:    
    ;handler registration
    ;call hook_int3_isr
    
    ;lea si, [msg]
    ;call puts
    ;jmp deuna
    
tmp:
	
    cli
    ;hd io configuration
    ;reading sector 1 (first)
    mov eax, deuna
	mov dword [cs:118*4], eax;IRQ 0x76
	sti
    
    xor al, al
    mov dx, 0x1f1
    
    out dx, al
    
    inc dx
 	mov al, 0x01
    out dx, al ;Sector count
    
    inc dx
    xor al, al
    out dx, al ;Low 8bits of the block address
    
    inc dx
    out dx, al ;Next 8bits of the block address
    
    inc dx
    out dx, al ;Next 8bits of the block address
    
    inc dx
    mov al, 0xe0
    out dx, al ;Indicator, some magic bits
    		   ;and highest 4 bits of the block address
    		   
    mov dx, 0x3f6
	mov al, 0x00
	out dx, al
    					
 	;send read (0x20) command
 	mov dx, 0x1f7
 	mov al, 0x20
   	out dx, al
   	
;====<INTERRUPTION>

iloop: jmp iloop
;====<\INTERRUPTION>
   	
ready:;Espera asta que el disco termina de acceder el sector
	in al, dx
	and al, 0x8
	cmp al, al
	jnz ready

deuna:	
	mov cx, 256
	mov bx, 0
	mov dx, 0x1f0
	mov ax, 0xb800
	mov ds, ax
	mov al, 0x41
rfd:
	in ax, dx
	push ax
	call pasar
	mov [ds:bx], ah
	inc bx
	inc bx
	mov [ds:bx], al
	inc bx
	inc bx
	
	pop ax
	mov al, ah
	
	call pasar
	mov [ds:bx], ah
	inc bx
	inc bx
	mov [ds:bx], al
	inc bx
	inc bx
	
	loop rfd
	iret;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
sleep:
	jmp sleep
;=======================================	
pasar:	
	push bx
	push ax
	and al, 0xf0
	shr al, 4
	cmp al, 0x9
	jbe numero
	add al, 0x57
	
	jmp fin
numero:
	add al, 0x30
fin:
	mov ah, al
	pop bx
	mov al, bl
	pop bx
	
	and al, 0x0f
	cmp al, 0x9
	jbe numero2
	add al, 0x57
	jmp fin2
numero2:
	add al, 0x30
fin2:
	ret
;====================================	
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
        
hook_int3_isr:
	cli
	mov bx, 0x0d
	shl bx, 2   ;multiply by 4
	xor ax, ax
	mov gs, ax   ;start of memory
	mov [gs:bx], dword keyhandler
	mov [gs:bx+2], ds ; segment
	sti
	ret
   	
;IRQ handler
ide_handler:
	cli
	push si
	lea si, [msg]
    call puts
    
    in al, 0x61   ; keybrd control
    mov ah, al
    or al, 0x80   ; disable bit 7
    out 0x61, al   ; send it back
    xchg ah, al   ; get original
    out 0x61, al   ; send that back
    
    pop si
    sti
    iret
    
keyhandler:
   in al, 0x60   ; get key data
   mov bl, al   ; save it
   mov byte [port60], al

   in al, 0x61   ; keybrd control
   mov ah, al
   or al, 0x80   ; disable bit 7
   out 0x61, al   ; send it back
   xchg ah, al   ; get original
   out 0x61, al   ; send that back

   mov al, 0x20   ; End of Interrupt
   out 0x20, al   ;

   and bl, 0x80   ; key released
   jnz done2   ; don't repeat

   mov ax, [port60]
   mov  word [reg16], ax
   lea si, [msg]
   call puts

done2:
   iret
	
port60   dw 0
reg16    dw 0
	
	msg db 'IRQ',13,10,0
	times 510-($-$$) db 0
	dw 0xAA55
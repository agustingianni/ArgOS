; bootstrap.s 
; Version 0.1
; Primera implementacion del bootstrap
; algunas cosas de: http://www.nondot.org/sabre/os/files/Booting/PolyOS.html
; otras cosas de: http://freesourcecodes.tripod.com/bootsector/index.htm

[BITS 16]       ; the bios starts out in 16-bit real mode
[ORG 0]

	jmp start         ; Goto segment 07C0

    bootdrv         db 0
    bootmsg         db 'Booting argos (c) 2008 palenke team',13,10,0
    loadmsg         db 'Loading kernel ...',13,10,0
    jumpmsg         db 'Jumping to kernel ...',13,10,0
    rebootmsg       db 'Press any key to reboot',13,10,0

    ; these are used in the processor identification
    processormsg    db 'Checking for 386+ processor: ',0
    need386         db 'Sorry... 386+ required!',13,10,0
    found386        db 'Excellent!',13,10,0

    ; these are used when entering protected mode
    a20msg          db 'Setting A20 address line',13,10,0
    pmodemsg        db 'Setting CR0 -> Entering PMode',13,10,0
	rrmodemsg       db 'Running on RealMode',13,10,0
	rpmodemsg       db 'Running on ProtectedMode',13,10,0
	; ------------------------------------------
	; Functions used in the boot-loading process
	; ------------------------------------------
    detect_cpu:
    	lea si, [processormsg]  ; tell the user what we're doing
        call puts

        ; test if 8088/8086 is present (flag bits 12-15 will be set)
        pushf                   ; save the flags original value
        
        xor ah,ah               ; ah = 0
        push ax                 ; copy ax into the flags
        popf                    ; with bits 12-15 clear
        
        pushf                   ; Read flags back into ax
        pop ax       
        and ah,0f0h             ; check if bits 12-15 are set
        cmp ah,0f0h
        je no386                ; no 386 detected (8088/8086 present)

        ; check for a 286 (bits 12-15 are clear)
        mov ah,0f0h             ; set bits 12-15
        push ax                 ; copy ax onto the flags
        popf
        
        pushf                   ; copy the flags into ax
        pop ax
        and ah,0f0h             ; check if bits 12-15 are clear
        jz no386                ; no 386 detected (80286 present)
        popf                    ; pop the original flags back
        
        lea si, [found386]
        call puts
        ret                     ; no 8088/8086 or 286, so ateast 386
 		
 		no386:
        lea si, [need386]       ; tell the user the problem
        call puts
        jmp reboot              ; and reboot when key pressed
                 
	;------------------------------------------------------------------
    puts:                           ; Dump ds:si to screen.
        lodsb                   ; load byte at ds:si into al
        or al,al                ; test if character is 0 (end)
        jz done
        mov ah,0eh              ; put character
        mov bx,0007             ; attribute
        int 0x10                ; call BIOS
        jmp puts
		
		done:
        ret
	;------------------------------------------------------------------
    readkey:
        mov ah, 0               ; wait for key
        int 016h
        ret
        
	;------------------------------------------------------------------        
    reboot:
        lea si, [rebootmsg]     ; be polite, and say we're rebooting
	    call puts
	    call readkey            ; and even wait for a key :)
	
	    db 0EAh                 ; machine language to jump to FFFF:0000 (reboot)
	    dw 0000h
	    dw 0FFFFh

	check_mode:
		mov eax, cr0
		and al, 1
		jz real_mode
		lea si, [rpmodemsg]
		call puts
		ret
		
		real_mode:
		lea si, [rrmodemsg]
		call puts
		ret

	;-------------------------------
	; IDT Hooker
	msgss db 'Fake int',13,10,0
	bkup resd 2
	
	fake_isr:
		lea si, [msgss]
		call puts
		iret
	
	hook_int3_isr:
	    cli
	    mov bx, 0x09
	    shl bx, 2   ;multiply by 4
	    xor ax, ax
	    mov gs, ax   ;start of memory
	    mov [gs:bx], dword fake_isr
	    mov [gs:bx+2], ds ; segment
	    sti
		ret

    start:		
        mov ax,0x7c0    ; BIOS puts us at 0:07C00h, so set DS accordinly
        mov ds,ax       ; Therefore, we don't have to add 07C00h to all our data

		call hook_int3_isr
		
		int 9
		int 9
		int 9

        mov [bootdrv], dl ; quickly save what drive we booted from
		
		cli             ; clear interrupts while we setup a stack
        mov ax,0x9000   ; this seems to be the typical place for a stack
        mov ss,ax
        mov sp,0xffff   ; let's use the whole segment.  Why not?  We can :)
        sti             ; put our interrupts back on
		
		lea si, [bootmsg]  ; display our startup message
        call puts
        
		; detectar si estamos en 386+
		call detect_cpu
		
		lea si, [message]
		call puts
		
		; prints the current running mode
		call check_mode
		
		end:
		call reboot
	
	message db 'Hola!',13,0
	
	times 510-($-$$) db 0   ; Fill the file with 0's
	dw 0AA55h               ; End the file with AA55
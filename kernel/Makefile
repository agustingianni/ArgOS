LDSCRIPT	= $(TOPDIR)/kernel/main.ld  

# FIXME:
# Si agregamos algun source mas que tenga que ser linkeado
# con el kernel tenemos que agregarlo aca tambien, creo que
# se pueden usar expresiones regulares, por ahora lo dejo asi.
OBJS		= \
			$(TOPDIR)/boot/entry.o \
			$(TOPDIR)/boot/multiboot.o \
			$(TOPDIR)/kernel/main.o \
			$(TOPDIR)/kernel/console.o \
			$(TOPDIR)/kernel/printk.o \
			$(TOPDIR)/kernel/scheduler.o \
			$(TOPDIR)/kernel/utils.o \
			$(TOPDIR)/kernel/bootmm.o \
			$(TOPDIR)/kernel/slab.o \
			$(TOPDIR)/arch/asm_idt.o \
			$(TOPDIR)/arch/idt.o \
			$(TOPDIR)/arch/cpuid.o \
			$(TOPDIR)/arch/exception.o \
			$(TOPDIR)/arch/paging.o \
			$(TOPDIR)/arch/syscalls.o \
			$(TOPDIR)/arch/irq.o \
			$(TOPDIR)/arch/gates.o \
			$(TOPDIR)/arch/apic.o \
			$(TOPDIR)/arch/timer.o \
			$(TOPDIR)/arch/jiffies.o \
			$(TOPDIR)/arch/delay.o \
			$(TOPDIR)/arch/tss.o \
			$(TOPDIR)/arch/locking/atomic.o \
			$(TOPDIR)/drivers/keyboard/keyboard.o \
			$(TOPDIR)/drivers/ide/ide.o \
			$(TOPDIR)/drivers/rtc.o \
			$(TOPDIR)/mm/asm_gdt.o \
			$(TOPDIR)/mm/gdt.o \
			$(TOPDIR)/mm/vm.o \
			$(TOPDIR)/lib/string.o \
			$(TOPDIR)/lib/ctype.o \
			$(TOPDIR)/lib/vsprintf.o\
			$(TOPDIR)/lib/bitmap.o \
			$(TOPDIR)/vm/buddy_allocator.o \
			$(TOPDIR)/block/gbl.o
			

# Target por default crea la imagen del kernel
all: $(KERNEL_IMAGE)

# Crea la imagen del kernel linkeando todos los .o
$(KERNEL_IMAGE): $(OBJS) $(LDSCRIPT)
	# TODO:
	# Esta pareciera ser la unica que anda, no se por que no
	# me genera bien la imagen el ld
	#
	@echo -e "[kernel] Creating the kernel image"
	$(CC) $(CFLAGS) -Xlinker -T -Xlinker $(LDSCRIPT) -nostartfiles -nodefaultlibs -nostdlib -o $@ $(OBJS)
	
	## $(LD) -T$(LDSCRIPT) -o $@ $(OBJS) $(LDFLAGS)
	## $(LD) -T$(LDSCRIPT) -Map $(MAP) -o $@ $(OBJS) $(LDFLAGS)

# Limpia los .o
clean:
	rm -f $(OBJS)

# Incluye las definiciones por defecto de como compilar .s y .c
include $(TOPDIR)/Rules.make
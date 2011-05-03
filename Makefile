.EXPORT_ALL_VARIABLES:
export MAKE

KERNELNAME	= argos

# Version
MAINVERSION	= 0
SUBVERSION	= 0
SUBLEVEL	= 1
VERSION		= $(MAINVERSION).$(SUBVERSION).$(SUBLEVEL)

# Directories 
TOPDIR		= $(shell /bin/pwd)
BIN			= $(TOPDIR)/bin
DOC			= $(TOPDIR)/doc
INCLUDE		= $(TOPDIR)/include
LIB			= $(TOPDIR)/lib
KERNEL		= $(TOPDIR)/kernel

# Poner los scripts en un lugar predeterminado
SCRIPTS		= $(TOPDIR)/kernel

MAP		= $(TOPDIR)/bin/kernel.map

KERNEL_IMAGE = $(BIN)/kernel.img

ifeq ($(TOPDIR)/.config,$(wildcard $(TOPDIR)/.config))
include $(TOPDIR)/.config
endif

LDFLAGS		= -nostdlib
CFLAGS		= -ggdb -Wall -pipe -I$(INCLUDE)\
			-nostdinc -fno-builtin\
			-nostartfiles -nodefaultlibs\
			-nostdlib -fstrength-reduce
			#-O2 -finline-functions \
			#-fomit-frame-pointer \
			#-mpreferred-stack-boundary=2 \
		  
		  
# Programs 
MAKE		= make -s
AS			= nasm -f elf
CC			= gcc
LD			= ld
OBJDUMP		= objdump -D -l -x -t
NASM		= nasm -f elf
GRUB		= /usr/sbin/grub

all: kernel
	@echo "[kernel] Compiling the kernel."

# Le redirije el control a kernel/Makefile
kernel: depend
	@$(MAKE) -C $(KERNEL)

# Calcula las dependencias de los .c y las guarda en .depend
depend:
	@rm -f .depend
	@echo "[depend] Crating dependencies file."
	@$(foreach i, $(shell find ${TOPDIR} -name \*.c -print), \
		echo -n $(dir $i) >> .depend; $(CC) -I$(INCLUDE) -M $i >> .depend;)
	@echo "[depend] Search of dependencies done."


MOUNT_DIR	= /media/cdrom
DISK_IMAGE	= $(TOPDIR)/bin/harddisk.img
MOUNT_FLAGS	= -t ext2 -o loop,offset=32256

# Copia la imagen del kernel a la imagen del disco de QEMU
image:
	@echo "[image] Installing kernel image to the hard disk image."
	@echo "        Image:       $(DISK_IMAGE)"
	@echo "        Mount point: $(MOUNT_DIR)"
	@echo "        Kernel image: $(KERNEL_IMAGE)"
	
	sudo mount $(MOUNT_FLAGS) $(DISK_IMAGE) $(MOUNT_DIR)
	sudo cp $(KERNEL_IMAGE) $(MOUNT_DIR)/boot/kernel.img
	sudo umount $(MOUNT_DIR)

clean:
	@$(MAKE) -C $(KERNEL) clean
	
	@echo "[clean] Cleanning .depend."
	@rm -f .depend $(KERNEL_IMAGE)
	
	@echo "[clean] Cleanning backup files."
	@find . -name *~ -print | xargs rm -f

distclean: clean
	@echo "[distclean] Clean done."

# Crea un tar.bz2 con los sources y demas cositas
dist: distclean
	cd .. && tar cf - `basename $(TOPDIR)`/ | bzip2 -c -9 > $(KERNELNAME)-$(VERSION).tar.bz2
	sync
	@echo "[dist] Backup done and saved to: $(KERNELNAME)-$(VERSION).tar.bz2.\n"

.PHONY: depend kernel all clean image

# Incluye las reglas por defecto de como compilar .s y .c
include Rules.make

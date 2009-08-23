#include <boot/multiboot.h>
#include <kernel/console.h>
#include <kernel/printk.h>
#include <lib/string.h>
#include <debug.h>
#include <arch/paging.h>
#include <mm/space.h>

/* Provided by the linker (kernel/main.ld) */
extern paddr_t KERN_TEXT_BEGIN;
extern paddr_t KERN_TOP;

/* 
 * Machine state when multiboot boot's the kernel
 *
 * eax = magic value 
 * ebx = has a pointer to the multiboot header
 * A20 is enabled
 * The computer is running on protected mode
 * pic programmed with the normal bios/DOS values
 * 
 */
 
 int
 init_boot_information
 (
 	uint32_t magic,
 	mbootinfo_t *mbi,
 	sysinfo_t *sys_info
 )
 {
	int i = 0;
	
	memset((void *) sys_info, 0x00, sizeof(sysinfo_t));
	 
 	/* Integrity check */
 	if(magic != MULTIBOOT_INFO_MAGIC)
	{
		printk("[INFO] Invalid magic number (%x)\n", magic);
		return -1;
	}
	
	if (!CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_MEMORY))
	{
		printk
		(
			"[FATAL] There is no memory information "
			"on multiboot structure\n"
		);
		
		return -1;
	}

	sys_info->sys_kern_start = (vaddr_t) &KERN_TEXT_BEGIN;
	sys_info->sys_kern_end   = (vaddr_t) &KERN_TOP;
	
	sys_info->sys_lomem 	 = mbi->mi_mem_lower*1024;
	sys_info->sys_himem 	 = mbi->mi_mem_upper*1024;
        
	sys_info->sys_page_size  = PAGE_SIZE;
	
	/** HiMem empieza desde el mega a contar asi que le tenemos que sumar */
	sys_info->sys_memory =  sys_info->sys_lomem + 
		sys_info->sys_himem + 1024*1024;
		
	paddr_t seg_start = 0, seg_end = 0;
	
	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_MMAP))
   	{
                struct multiboot_mmap *mmap;
    	
		#ifdef DEBUG
                printk("[mboot] Memory maps:\n");
		#endif
       
		for(mmap = (struct multiboot_mmap *) mbi->mi_mmap_addr;
			(unsigned long) mmap < mbi->mi_mmap_addr + mbi->mi_mmap_length;
                        mmap = (struct multiboot_mmap *) ((unsigned long) mmap
			+ mmap->mm_size + sizeof (mmap->mm_size)))
		{
			/* Tenemos un limite maximo estatico de # de segmentos */
			if (i == VM_MAX_SEGMENTS)
			{
				printk("[NOTICE] Too many segments increase VM_MAX_SEGMENTS\n");
				break;
			}
			
			/* Si el tipo no es AVAILABLE RAM skipeamos */
			if(mmap->mm_type != AVL_RAM)
			{
				continue;
			}
			
			/* Por ahora usamos la parte baja del puntero que nos da GRUB */
			seg_start = mmap->mm_base_addr_low;
			seg_end	  = mmap->mm_base_addr_low + mmap->mm_length_low;
		
			if(seg_start < 0x100000 && seg_end > 0xa0000)
			{
				printk("[NOTICE] segment overlaps with ``Compatibility Holes''\n");
				
				sys_info->sys_addr_space.segments[i].seg_start = seg_start;
				sys_info->sys_addr_space.segments[i].seg_end   = 0xa0000;
				sys_info->sys_addr_space.segments[i].seg_type  = mmap->mm_type;
				sys_info->sys_addr_space.segments_number++;
				i++;
		
				sys_info->sys_addr_space.segments[i].seg_start = 0x100000;
				sys_info->sys_addr_space.segments[i].seg_end   = seg_end;
				sys_info->sys_addr_space.segments[i].seg_type  = mmap->mm_type;
				sys_info->sys_addr_space.segments_number++;
				i++;
			}
			else
			{
				sys_info->sys_addr_space.segments[i].seg_start = seg_start;
				sys_info->sys_addr_space.segments[i].seg_end   = seg_end;
				sys_info->sys_addr_space.segments[i].seg_type  = mmap->mm_type;
				sys_info->sys_addr_space.segments_number++;
				i++;
			}
                }
        }

	/* Inicializamos en el maximo PFN */
	sys_info->sys_minpfn = PFN_UP(0xffffffff);
	
	/* Inicializamos en el mninimo PFN */
	sys_info->sys_maxpfn = PFN_DOWN(0);
	
	/* Buscamos el Minimum page frame number y el Maximun Page Frame number */
	for(i = 0; i < sys_info->sys_addr_space.segments_number; i++)
	{
		sys_info->sys_minpfn = min(sys_info->sys_minpfn,
			PFN_UP(sys_info->sys_addr_space.segments[i].seg_start));
		
		sys_info->sys_maxpfn = max(sys_info->sys_maxpfn, 
			PFN_DOWN(sys_info->sys_addr_space.segments[i].seg_end));
	}
	
	sys_info->sys_pages = sys_info->sys_maxpfn+1;
	
	/* 
	 * FIXME: Aca hay un error, el maxpfn esta mal calculado
	 * por que lo estoy tomando con respecto a seg_end y si seg_end
	 * termina al medio de una pagina, el max_pfn tendira que ser
	 * el numero de pagina al que corresponde seg_end menos 1. No lo arreglo
	 * por que cuando hago los cambios no se por que carajo crashea. Si
	 * te das cuenta por que pasa Gaston, arreglalo.
	 * Sino, mas adelante loveo.
	 */
	
	#define PAGES_TO_MB(x) ((x) >> (20-PAGE_SHIFT))
	printk("[INFO] %ldMB LOWMEM available.\n", PAGES_TO_MB(sys_info->sys_maxpfn));

	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_VBE))
   	{
 		printk("[mboot] VBE Information present\n");
		DUMP(mbi->mi_vbe_control_info, "%p\n");
		DUMP(mbi->mi_vbe_mode_info, "%p\n");
		DUMP(mbi->mi_vbe_interface_seg, "%d\n");
		DUMP(mbi->mi_vbe_interface_off, "%d\n");
		DUMP(mbi->mi_vbe_interface_len, "%d\n");
   	}
	
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_DRIVES))
   	{
		#ifdef DEBUG
		int i, count;
		struct multiboot_drive *mbd = 
			(struct multiboot_drive *) mbi->mi_drives_addr;

		count = mbi->mi_drives_length / sizeof(struct multiboot_drive);

		printk("[mboot] Drives information:\n");

		for(i = 0; i < count; i++, mbd++)
		{
			printk
			(
				"  | Bios drive number: %d\n"
				"  | Drive Mode:        %s\n"
				"  | C = %d | H = %d | S = %d | BLOCKS = %d | Size = %d\n",
				mbd->md_number,
				(mbd->md_mode ? "LBA" : "CHS"),
				mbd->md_cylinders,
				mbd->md_heads,
				mbd->md_sectors,
				mbd->md_cylinders * mbd->md_heads * mbd->md_sectors,
				mbd->md_cylinders * mbd->md_heads * mbd->md_sectors * 512
			);
		}
		#endif /* DEBUG */
   	}
	
	#ifdef DEBUG
	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_BOOT_DEVICE))
	{
		printk ("[mboot] Device drive: 0x%x\n", 
			mbi->mi_boot_device_drive);
	} 
	
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_CMDLINE))
   	{
    	printk ("[mboot] Command line: %s\n", 
    		mbi->mi_cmdline);
   	}
	
	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_MODS))
    {
	   	struct multiboot_module *mod;
       	int i;
 
       	printk
       	(
       		"[mboot] Number of loaded modules: %d\n",
			(int) mbi->mi_mods_count
		);
       
 		mod = (void *) mbi->mi_mods_addr;
		
		for (i = 0; i < mbi->mi_mods_count; i++, mod++)
		{
			printk
        	(
        		"\t[%u] mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
            	(unsigned) i,
            	(unsigned) mod->mm_mod_start,
                (unsigned) mod->mm_mod_end,
                (char *) mod->mm_string
        	);
       	}
    }
 
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_AOUT_SYMS) 
   		&& CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_ELF_SYMS))
    {
    	printk("[Error] Cannot have both, ELF header and aout header\n");
       	return -1;
    }
 
   	/* Is the symbol table of a.out valid? */
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_AOUT_SYMS))
    {
    	struct multiboot_aout_symtab *aout_sym = &(mbi->u.aout_sym);
 
      	printk
      	(
      		"aout_symbol_table: tabsize = 0x%0x, "
            "strsize = 0x%x, addr = 0x%x\n",
            aout_sym->ma_tabsize,
            aout_sym->ma_strsize,
            aout_sym->ma_addr
        );
    }
 
   	/* Is the section header table of ELF valid? */
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_ELF_SYMS))
    {
    	struct multiboot_elf_sh_table *elf_sec = &(mbi->u.elf_sec);
 
       	printk
       	(
       		"[mboot] elf_sec: num = %u, size = 0x%x,"
            " addr = 0x%x, shndx = 0x%x\n",
            elf_sec->me_num,
            elf_sec->me_size,
            elf_sec->me_addr,
            elf_sec->me_shndx
        );
    }
 
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_CONFIG_TABLE))
   	{
 		
   	}

   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_LOADER_NAME))
   	{
  		printk("[mboot] Loaded by: %s\n", mbi->mi_loader_name);
   	}
  	
   	if (CHECK_BIT (mbi->mi_flags, MULTIBOOT_INFO_HAS_APM_TABLE))
   	{
 		printk("[mboot] APM Information present\n");
 		
		struct multiboot_apm *apm_info = (struct multiboot_apm *) mbi->mi_apm_table;
		
		printk( "    version     : %x\n", apm_info->ma_version );
		printk( "    cseg        : %x\n", apm_info->ma_cseg );
		printk( "    offset      : %x\n", apm_info->ma_offset );
		printk( "    cseg_16     : %x\n", apm_info->ma_cseg_16 );
		printk( "    dseg        : %x\n", apm_info->ma_dseg );
		printk( "    flags       : %x\n", apm_info->ma_flags );
		printk( "    cseg_len    : %x\n", apm_info->ma_cseg_len );
		printk( "    cseg_16_len : %x\n", apm_info->ma_cseg_16_len );
		printk( "    dseg_len    : %x\n", apm_info->ma_dseg_len );
   	}
	#endif	/* DEBUG */
   	
 	return 0;	
 }

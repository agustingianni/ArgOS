#include <asm/types.h>
#include <arch/paging.h>
#include <arch/asm.h>
#include <arch/tlb.h>
#include <arch/exception.h>
#include <arch/cpuid.h>
#include <kernel/kernel.h>
#include <kernel/printk.h>
#include <kernel/utils.h>
#include <mm/space.h>
#include <boot/multiboot.h>

extern sysinfo_t sys_info;

/*
 * La lista global de paginas
 * usada por el alocador de paginas
 */
vaddr_t *page_list;

/*
 * ptable0 maps from 0 to 4 MB (0x00000000-0x003fffff)
 * ptable1 maps from 4 to 8 MB (0x00400000-0x007fffff)
 */
ptable_t ptable0;
ptable_t ptable1;

/*
 * Agregamos dos tablas de paginas temporales
 * al directorio de paginas del kernel.
 *
 * Las vamos a ultilizar para levantar posteriormente
 * la paginacion completa.
 */
pdir_t page_directory;

void
init_temp_paging(void)
{
	vaddr_t  address     = 0;
	uint32_t pde_idx;
	uint32_t pte_idx;

	/*
	 * Zero out all the important structures used by
	 * the MMU
	 */
	memset((void *) &page_directory, 0x00, PAGE_DIR_SIZE);
	memset((void *) &ptable0, 0x00, PAGE_TABLE_SIZE);
	memset((void *) &ptable1, 0x00, PAGE_TABLE_SIZE);

	/* By default assign restrictive permissions */
	for(pde_idx = 0; pde_idx < PAGE_DIR_ENTRIES; pde_idx++)
	{
		set_page_directory_entry
		(
			&page_directory.entry[pde_idx],
			0,
			PAGE_NOT_ACCESSED,
			PAGE_NOCACHE,
			PAGE_NOWTHR,
			PAGE_KERNEL_PRIV,
			PAGE_READ_WRITE,
			PAGE_ABSENT,
			PAGE_SIZE_4KB,
			PAGE_LOCAL,			/* because PSE = off */
			PAGE_CLEAN			/* because PSE = off */
		);
	}

	/*
	 * Inicializamos la tabla de paginas identidad.
	 */
	for (pte_idx = 0; pte_idx < PAGE_TABLE_ENTRIES; pte_idx++)
	{
		set_page_table_entry
		(
			&ptable0.entry[pte_idx],
			address,	/* Address of the page */
			0,
			0,
			PAGE_CLEAN,
			PAGE_ACCESSED,
			PAGE_NOCACHE,
			PAGE_NOWTHR,
			PAGE_KERNEL_PRIV,
			PAGE_READ_WRITE,
			PAGE_PRESENT
		);

		address += PAGE_SIZE;
	}

	/*
	 * Inicializamos la tabla de paginas identidad.
	 */
	for (pte_idx = 0; pte_idx < PAGE_TABLE_ENTRIES; pte_idx++)
	{
		set_page_table_entry
		(
			&ptable1.entry[pte_idx],
			address,	/* Address of the page */
			0,
			0,
			PAGE_CLEAN,
			PAGE_ACCESSED,
			PAGE_NOCACHE,
			PAGE_NOWTHR,
			PAGE_KERNEL_PRIV,
			PAGE_READ_WRITE,
			PAGE_PRESENT
		);

		address += PAGE_SIZE;
	}

	/*
	 * Seteamos la entrada 0 que es la que va a
	 * contener el mapeo identidad, es decir,
	 * mapea 0x00000000-0x00400000 -> 0x00000000-0x00400000
	 */
	set_page_directory_entry
	(
		&page_directory.entry[0],
		virt_to_phys(&ptable0.entry[0]),
		PAGE_ACCESSED,
		PAGE_NOCACHE,
		PAGE_NOWTHR,
		PAGE_KERNEL_PRIV,
		PAGE_READ_WRITE,
		PAGE_PRESENT,
		PAGE_SIZE_4KB,
		PAGE_LOCAL,			/* because PSE = off */
		PAGE_CLEAN			/* because PSE = off */
	);

	set_page_directory_entry
	(
		&page_directory.entry[1],
		virt_to_phys(&ptable1.entry[0]),
		PAGE_ACCESSED,
		PAGE_NOCACHE,
		PAGE_NOWTHR,
		PAGE_KERNEL_PRIV,
		PAGE_READ_WRITE,
		PAGE_PRESENT,
		PAGE_SIZE_4KB,
		PAGE_LOCAL,			/* because PSE = off */
		PAGE_CLEAN			/* because PSE = off */
	);

	set_page_directory_entry
	(
		&page_directory.entry[768],
		virt_to_phys(&ptable0.entry[0]),
		PAGE_ACCESSED,
		PAGE_NOCACHE,
		PAGE_NOWTHR,
		PAGE_KERNEL_PRIV,
		PAGE_READ_WRITE,
		PAGE_PRESENT,
		PAGE_SIZE_4KB,
		PAGE_LOCAL,			/* because PSE = off */
		PAGE_CLEAN			/* because PSE = off */
	);

	set_page_directory_entry
	(
		&page_directory.entry[769],
		virt_to_phys(&ptable1.entry[0]),
		PAGE_ACCESSED,
		PAGE_NOCACHE,
		PAGE_NOWTHR,
		PAGE_KERNEL_PRIV,
		PAGE_READ_WRITE,
		PAGE_PRESENT,
		PAGE_SIZE_4KB,
		PAGE_LOCAL,			/* because PSE = off */
		PAGE_CLEAN			/* because PSE = off */
	);


	/* Set the current page directory */
	set_page_directory((pdir_t *) virt_to_phys(&page_directory.entry[0]));

	/* now we register the page fault handler function */
	set_exception_handler(14, &page_fault_handler);

	/* Enable CPU support for paging */
	enable_paging();

	return;
}

/*
 * Aca inicializamos en serio la paginacion.
 * Hay dos formas de inicializarla, una es cuando PSE = 0
 * que seria la forma normal, con paginas de 4kb y la otra
 * es cuando PSE = 1 que tenemos paginas de 4 mb y no creamos
 * tablas de paginas como en la anterior. En esta ultima
 * el directorio PGD pasa a ser la unica tabla de paginas
 * necesaria.
 */
void
init_paging(void)
{
	uint32_t page_number = 0;
	uint32_t pde_idx, pte_idx;

	/* kernel's page directory */
	pte_t	*pte         = NULL;
	vaddr_t address;

	uint32_t have_pse = cpu_has_pse();
	uint32_t have_pge = cpu_has_pge();
	uint32_t have_nx  = 0;

	#ifdef SUPER_PAGES
	have_pse = 0;
	have_pge = 0;
	#endif /* SUPER_PAGES */

	if(have_pge)
	{
		enable_pge();
	}

	if(have_pse)
	{
		enable_pse();

		/* The TLB must be flushed after enabling large pages */
		//tlb_flush_all();
	}

	if(have_nx)
	{
            /* El procesador tiene capacidad NX */
            printk("Enable NX not implemented\n");
	}

	/** for each page directory entry starting from PAGE_OFFSET */
	for
	(
		pde_idx = PAGE_OFFSET >> 22;
		pde_idx < PAGE_DIR_ENTRIES &&
			page_number < sys_info.sys_maxpfn;
		pde_idx++
	)
	{
		/*
                 * If PSE is on we do not need to create page tables
                 * because they are not used, each entry in the directory
                 * is a big 4mb page.
                 */
		pte = (have_pse) ? ((pte_t *) PFN_VIRT(page_number)) : pgtable_create();

		/* if PSE is on, we dont need to set the PTE entries so we can skip that part */
		if(have_pse)
		{
			page_number += PAGE_TABLE_ENTRIES;
			goto set_pde;
		}

		/* for each pte entry */
		for
		(
			pte_idx = 0,
			address = PFN_VIRT(page_number);
			pte_idx < PAGE_TABLE_ENTRIES &&
				page_number < sys_info.sys_maxpfn;
			pte_idx++,
			page_number++,
			address += PAGE_SIZE
		)
		{
			set_page_table_entry
			(
				pte + pte_idx,
				virt_to_phys((vaddr_t *) address),		/* Address of the page */
				(have_pge) ? PAGE_GLOBAL : PAGE_LOCAL,	/* GLOBAL */
				0,										/* PAT */
				PAGE_DIRTY,
				PAGE_ACCESSED,
				PAGE_NOCACHE,
				PAGE_NOWTHR,
				PAGE_KERNEL_PRIV,
				PAGE_READ_WRITE,
				PAGE_PRESENT
			);
		}

		set_pde:
		set_page_directory_entry
		(
			&page_directory.entry[pde_idx],
			virt_to_phys(pte),		/* page table entry */
			PAGE_ACCESSED,
			PAGE_NOCACHE,
			PAGE_NOWTHR,
			PAGE_KERNEL_PRIV,
			PAGE_READ_WRITE,
			PAGE_PRESENT,
			(have_pse) ? PAGE_SIZE_4MB : PAGE_SIZE_4KB,
			(have_pge) ? PAGE_GLOBAL : PAGE_LOCAL,
			(have_pse) ? PAGE_DIRTY : PAGE_CLEAN
		);

                #ifdef DEBUG
		printk("pde = %p - pde_idx = %d - pte addr = %p address %p\n",
				&page_directory.entry[pde_idx],
				pde_idx,
				virt_to_phys(pte),
				page_directory.entry[pde_idx].pd_address);
                #endif
	}

        /*
         * Ahora tenemos toda la memoria disponible en la maquina mapeada
         * en el directorio de paginas del kernel empezando desde la direccion
         * virtual PAGE_OFFSET (0xc0000000 por defecto).
         * Esto quiere decir que el kernel puede acceder a 1GB de memoria
         * fisica como maximo por ahora. Lo que se puede hacer es
         * cambiar el PAGE_OFFSET y ponerlo que empieze en 0xb0000000 asi podriamos
         * mapear 2GB de memoria fisica a costo de reducir 1GB el espacio
         * de memoria virtual de los procesos.
         *
         * Otra de las opciones es implementar como Ingo Molnar que el kernel
         * no comparta las entradas de su directorio de paginas con los procesos
         * Esto es un gran inconveniente, por que supone que cada ves que se realize
         * un context switch, desde user a kernel y viceversa, debemos cargar
         * el registro CR3 con el directorio de paginas del proceso en el caso de
         * un CS hacia userland y el directorio de paginas del kernel en el
         * caso de un CS hacia kernelland.
         *
         * TODO: Se podria buscar en que casos en necesario hacer el cambio de
         * CR3 y reducir al minimo el overhead.
         */

	/* flusheamos la TLB por que hicimos cambios en los mapeos del kernel */
	/* TODO: Fixear este codigo, es necesario. */

	//tlb_flush_all();

	return;
}

/*
 * TODO: Mover este codigo a uvm_init.c o algo similar ya que esta parte no
 * pertenece al codigo de paginacion. Esta parte es PLATFORM INDEPENDENT
 */
void
init_vm(sysinfo_t *sys_info)
{
	int i;
	uint32_t numpages = 0;
	paddr_t curraddr;

	/* la lista de frames libres esta justo detras del kernel */
	page_list = (vaddr_t *) PAGE_ROUND_UP(sys_info->sys_kern_end);

	/*
	 * Parseamos todas los segmentos que nos paso el GRUB
	 * y vamos a ir agregandolos a una estructura que
	 * nos permita usarlos para alocar memoria.
	 *
	 * Algunos segmentos no van a poder ser usados, tener en cuenta
	 * eso.
	 *
	 * Kernel Global Page Directory
	 */

	space_t *space = &sys_info->sys_addr_space;
	region_t *segment;

	#define MEGABYTE 0x100000

	/* for each defined segment */
	for(i = 0; i < space->segments_number; i++)
	{
		segment = &space->segments[i];
		if(!(segment->seg_type == AVL_RAM &&
			segment->seg_start >= MEGABYTE))
		{
			continue;
		}

		printk("Available segment 0x%p - 0x%p\n",
				space->segments[i].seg_start,
				space->segments[i].seg_end);

		/*
		 * 1) Verificar que el segmentos no tengan en el medio al kernel
		 * en el caso de que sea asi, dividirlo en 2 y colocar las
		 * partes que no se superponen en la lista de memoria libre.
		 *
		 * Para agregar el segmento libre vamos a usar algo similar a lo que
		 * usa Cranor en UVM uvm_page_physload()
		 *
		 * 2) Una vez que tengamos esto armadito, vamos a poder usar el
		 * UVM.
		 *
		 * 3) Para armar estas estructuras, vamos a hacer uso del bootmem allocator
		 *
		 * 4) Acordarse de que nosotros con el bootmem allocator usamos memoria
		 * antes de llegar a este punto. Esto significa que parte de los
		 * segmentos que estamos agregando a la lista de libres, fueron usados.
		 * Lo que debemos hacer es fijarnos cuales son las paginas usadas y marcarlas
		 * como en uso dentro del sistema UVM
		 */

		/* por ahora supongo que estan alineados a la PAGINA */
		for(curraddr = segment->seg_start;
			curraddr < segment->seg_end;
			curraddr += PAGE_SIZE)
		{
			/*
			 * Ahora por cada pagina dentro del segmento vamos a llamar
			 * a una funcion como uvm_pagefree() que va a agregar
			 * la pagina libre al pool de free pages.
			 */

			*page_list++ = curraddr;
			numpages++;
		}
	}

	/*
	 * Mark the last available frame as null so we can
	 * detect when we run out of free frames.
	 */
	*page_list = 0;


	printk("Pages added: %u\n", numpages);

	HARD_MALLOC(page_list, vaddr_t *, numpages * 4);
}


#include <kernel/scheduler.h>	/* current() */
#include <asm/types.h> 			/* vaddr_t */

void
page_fault_handler(icontext_t *c)
{
	pde_t *pde 		= NULL;
	pte_t *pte 		= NULL;
	pte_t *ptable 	= NULL;

	vm_space_t *current_vma;
	uint32_t access;

	/*
	 * Necesito el current por que tiene el cr3 que
	 * es el pgd del proceso actual. Ojo que puede
	 * ser que el fallo de pagina halla sido dentro
	 * del kernel. Por lo tanto no existe.
	 */

	/*
	 * Primero tenemos que obtener el directorio de
	 * paginas del proceso corriente
	 */
	task_t  *current_task 	= task_current();
	pde_t   *task_pgd 		= NULL;

	/*
	 * Sacamos la direccion del fallo de pagina
	 * este nos va a servir como indice en el directorio de
	 * paginas.
	 */
	vaddr_t  fault_address 	= get_page_fault_address();
	printk("Faulty address %p\n", fault_address);

	if(current_task)
	{
		task_pgd = (pde_t *) phys_to_virt(current_task->context->cr3);
        printk("EIP: %p\n", current_task->context->eip);
	}
	else
	{
		task_pgd = (pde_t *) phys_to_virt(read_cr3());
	}

	/* Address exported by the linker */
	extern vaddr_t K_VIR_START;

	/* address is inside kernel space */
	if(fault_address >= (vaddr_t) &K_VIR_START)
	{
		/* kernel mode exception and non present page */
		if(PF_KERN_MODE(c->err_code))
		{
			#if 0
			if(fault addres belongs to vmalloc_space)
			{
				handle_vmalloc();
				return;
			}

			#endif

			if(PF_PROT_FAULT(c->err_code))
			{
				panic("PANIC: Kernel page protection error.\n");
			}

			if(PF_PAGE_FAULT(c->err_code))
			{
				/* todas las paginas restantes del kernel deben estar mapeadas */
				panic("PANIC: Kernel page not present.\n");
			}
		}

		/*
		 * here we now that an usermode exception occurred
		 * we have to send a SIGSEGV
		 */
		panic("PANIC: User Space process tried to access Kernel Space address.\n");
	}

	access = PF_RD_ACCESS(c->err_code) ? VMA_READ : VMA_WRITE;

	/* FIXME: Temporal hasta que agreguemos los vmas a los procesos */
	if(current_task->mm)
	{
		/*
		 * Sanity checks
		 */
		current_vma = vma_find(current_task->mm, fault_address);
		if(!current_vma)
		{
			panic("PANIC: The current task has no VMA\n");
		}

		if(!vma_access_check(current_vma, access))
		{
			panic("PANIC: The current process tried to"
				  "access a vma with no permissions\n");
		}
	}

	/*
	 * All the sanity checks are passed so allocate
	 * the new page.
	 */
    pde = pdir_get_entry(task_pgd, fault_address);
    if(!pde_present(*pde))
    {
    	ptable = (pte_t *)NULL;
        if(!ptable)
        {
        	panic("pdir_get_entry(): OOM condition. "
        			"Cannot create page directory entry\n");
        }

        pdir_set_entry(pde, (paddr_t) ptable);
    }

    /*
     * Ahora obtenemos la entrada en la pagina de tablas.
     * En caso de que no exista la creamos y ya esta.
     */
    pte = ptable_get_entry((pte_t *) phys_to_virt(pde->pd_address), fault_address);
    if(!pte_present(*pte))
    {
    	if(pte_new(*pte))
    	{
    		/* la pagina es nueva */
    	}
    	else if(pte_swapped(*pte))
    	{
    		/* la pagina esta swapeada en el disco */
    	}
    	else
    	{
    		panic("Ni idea que paso aca");
    	}

    	paddr_t *free_frame = NULL;
    	if(!free_frame)
    	{
        	panic("ptable_get_entry(): OOM condition. "
        			"Cannot create page table entry\n");
    	}

    	ptable_set_entry(pte, (paddr_t) free_frame);
    }
    else
    {
    	/* the page is present, so it may be a COW page */
    	if(access == VMA_WRITE)
	    {
    		/** si la pagina es de solo lectura */
    		if (!pte_write(*pte))
    		{
    			/*
    			 * aca maneja COW (copy on write)
    			 * para las paginas compartidas SHARED
    			 */
    		}

    		pte_mkdirty(pte);
	    }

    	//pte_mkyoung(pte);
    }

    printk("added page:    %p\n", pte->pd_address);
    printk("faulty address %p\n", fault_address);

    return;
}

#include <kernel/bootmm.h>
#include <lib/bitmap.h>
#include <kernel/printk.h>
#include <arch/paging.h>
#include <bug.h>
#include <kernel/kernel.h>
#include <debug.h>

#include "lib/bitmap.h"

static bootmem_t bootmemory;

/*
 * Returns the size of boot memory
 */
uint32_t
bootmem_size(void)
{
	BUG_ON(bootmemory.mem_bitmap == NULL);
	return bootmemory.size;
}

uint32_t
bootmem_usage(void)
{
	BUG_ON(bootmemory.mem_bitmap == NULL);
	return bootmemory.used;
}

/*
 * Given a size, returns a pointer to free boot
 * memory or null in case we ran out of resources.
 */
vaddr_t*
bootmem_alloc(size_t size)
{
	vaddr_t *address = NULL;
	size_t pages = PFN_UP(size);

	int i = 0,
		j = 0,
		start = 0,
		found = 0;

	BUG_ON(bootmemory.mem_bitmap == NULL);
	
	while(i + pages <= bootmemory.size)
	{
		i = bitmap_next_zero(bootmemory.mem_bitmap, 
				bootmemory.size, i);
	
		if(i == -1)
		{
			BUG();
			return NULL;
		}
		
		start = i;
		
		for(j = 1; j < pages; j++)
		{
			if (bitmap_is_set(bootmemory.mem_bitmap, i+j))
			{
				i += j + 1;
				break;
			}
		}
		
		if(j == pages)
		{
			found = 1;
			break;
		}
	}
	
	if(found)
	{
		address = phys_to_virt((start + bootmemory.start_pfn) * PAGE_SIZE);
		bootmem_reserve(address, size);
	}
	
	return address;
}

/*
 * Free's memory allocated by bootmem_alloc()
 */
void
bootmem_free(vaddr_t *addr, size_t size)
{
	unsigned long sidx, eidx;
	unsigned long i;

	BUG_ON(bootmemory.mem_bitmap == NULL);
	
	/*
	 * round down end of usable mem, partially free pages are
	 * considered reserved.
	 */
	BUG_ON(!size);
	BUG_ON(PFN_DOWN(virt_to_phys(addr)) < bootmemory.start_pfn);
	BUG_ON(PFN_DOWN(virt_to_phys(addr) + size) > bootmemory.end_pfn);

	sidx = PFN_DOWN(virt_to_phys(addr)) - bootmemory.start_pfn;
	eidx = PFN_UP(virt_to_phys(addr) + size) - bootmemory.start_pfn;
	
	for (i = sidx; i < eidx; i++)
	{
		if (unlikely(bitmap_unset(bootmemory.mem_bitmap, i) == -1))
			BUG();
	}
	
	bootmemory.used -= PFN_UP(size);
}

/*
 * Free's all the pages allocated by bootmem allocator
 */

void
bootmem_free_all(void)
{
	BUG_ON(bootmemory.mem_bitmap == NULL);
	
	bitmap_zero(bootmemory.mem_bitmap,
			bootmemory.size);
	
	bootmemory.used = 0;
}

/*
 * Marks all the pages to be in use
 */
void
bootmem_use_all(void)
{
	BUG_ON(bootmemory.mem_bitmap == NULL);
	
	bitmap_fill(bootmemory.mem_bitmap,
		bootmemory.size, 0xffffffff);
	
	bootmemory.used = bootmemory.size;
}

/*
 * Marks a given region as used so no one can
 * mess the system up.
 * 
 * For example map as used from pfn 10 to pfn 20 including the 20
 * reserved.
 */
int
bootmem_reserve(vaddr_t *addr, size_t size)
{
	unsigned long sidx, eidx;
	unsigned long i;

	BUG_ON(bootmemory.mem_bitmap == NULL);
	
	/*
	 * round up, partially reserved pages are considered
	 * fully reserved.
	 */
	BUG_ON(!size);
	BUG_ON(PFN_DOWN(virt_to_phys(addr)) < bootmemory.start_pfn);
	BUG_ON(PFN_DOWN(virt_to_phys(addr) + size) > bootmemory.end_pfn);

	sidx = PFN_DOWN(virt_to_phys(addr)) - bootmemory.start_pfn;
	eidx = PFN_UP(virt_to_phys(addr) + size) - bootmemory.start_pfn;
	
	for (i = sidx; i < eidx; i++)
	{
		if (unlikely(bitmap_set(bootmemory.mem_bitmap, i) == -1))
			BUG();
	}
	
	bootmemory.used += PFN_UP(size);
	return 0;
}

#include <vm/buddy_allocator.h>

/* 
 * Retira el bootmemory allocator, ademas retorna
 * el tamano del bitmap para que podamos reaprovechar 
 */
uint32_t
bootmem_retire(void)
{
	uint32_t freed_pages = 0;
	int i;
	
	BUG_ON(bootmemory.mem_bitmap == NULL);
	
	extern page_t *mem_map;
	
	#define pfn_to_page(x) &mem_map[x]
	#define virt_to_page(x)	pfn_to_page(virt_to_phys(x) >> PAGE_SHIFT)
	
	for (i = bootmemory.start_pfn; i < bootmemory.end_pfn; i++)
	{
		if(bitmap_is_set(bootmemory.mem_bitmap, i))
			continue;

		freed_pages++;
		
		/* 
		 * TODO Me gustaria que esta funcion sea independiente del
		 * buddy allocator
		 */
		buddy_page_free(pfn_to_page(i), 0);
	}
	
	for(i = 0; i < PFN_UP(bootmemory.size); i++)
	{
		freed_pages++;
		buddy_page_free(pfn_to_page(PFN_DOWN(virt_to_phys(bootmemory.mem_bitmap)) + i), 0);
	}
	
	/* No necesitamos mas la informacion del bitmap */
	bootmemory.mem_bitmap = NULL;
	
	return 0;
}

/*
 * 1. init_bootmm():
 * 		parsea la informacion pasada por el grub
 * 		acerca de la memoria disponible del sistema
 * 		Ademas va a inicializar las estructuras necesarias
 * 		para el manejo de memoria de booteo
 */

int
init_bootmm(sysinfo_t *si)
{
	bootmemory.start_pfn 	= si->sys_minpfn;
	bootmemory.end_pfn 	= si->sys_maxpfn;
	bootmemory.size		= si->sys_maxpfn - si->sys_minpfn + 1;
	bootmemory.used		= 0;
	bootmemory.last_offset 	= 0;
	bootmemory.last_pos 	= 0;
	bootmemory.last_success	= 0;

	/* Alocamos memoria para el bitmap */
	HARD_MALLOC(bootmemory.mem_bitmap, bitmap_t,
		BITS2BYTES(bootmemory.size));
	
	/* Seteamos en 'usada' todas las paginas del bitmap */
	bootmem_use_all();

	space_t *space = &si->sys_addr_space;
	int i = 0;
	
	/* Habilitamos los segmentos libres que nos paso GRUB */
	for(i = 0; i < space->segments_number; i++)
	{
		vaddr_t * start = phys_to_virt(space->segments[i].seg_start);
		size_t size = (space->segments[i].seg_end -
			space->segments[i].seg_start);

		bootmem_free(start, size);
	}

	/* Por compatibilidad con hardware */
	bootmem_reserve((vaddr_t *) phys_to_virt(0), PAGE_SIZE);
	
	/* Reservamos la memoria usada por el kernel */
	bootmem_reserve((vaddr_t *) si->sys_kern_start,
		si->sys_kern_end - si->sys_kern_start);
	
	return 0;
}

#include <vm/vm_init.h>
#include <lib/queue.h>

/*
 * NOTE: Todo el codigo que esta aca adentro debe ser 
 * PLATFORM INDEPENDENT
 */

/*
 * vm_set_page_size: initialize all the values that depends on page size
 */
void
vm_set_page_size(void)
{
	vm.page_size = PAGE_SIZE;
	vm.page_mask = vm.page_size - 1;
	
	vm.page_shift = 0;

	while((1 << vm.page_shift) != vm.page_size)
		vm.page_shift++;
}

extern sysinfo_t sys_info;

void
vm_segments_init(uint32_t kernel_end)
{
	uint32_t i;
	uint32_t seg_start, seg_end;
	uint32_t seg_start1, seg_end1;
	uint32_t avail_start = PAGE_SIZE;
	
	uint32_t count = sys_info.sys_addr_space.segments_number; 
	region_t regions = sys_info.sys_addr_space.segments;
	
	/* End of the kernel */
	kernel_end = round_page(kernel_end);
	
	/* Por cada uno de los segmentos que nos paso el GRUB */
	for (i = 0; i < count; i++) 
	{
		seg_start 	= regions[i].seg_start;
		seg_end 	= regions[i].seg_end;
		seg_start1 	= 0;
		seg_end1 	= 0;

		/* Skip memory before our available starting point. */
		if (seg_end <= avail_start)
			continue;

		if (avail_start >= seg_start && avail_start < seg_end) 
		{
			if (seg_start != 0)
				panic("init386: memory doesn't start at 0");

			/* Skipeamos la primera pagina de memoria */
			seg_start = avail_start;
			if (seg_start == seg_end)
				continue;
		}

		/**
		 * If this segment contains the kernel, split it
		 * in two, around the kernel.
		 */
		#define IOM_END 0x100000
		if (seg_start <= IOM_END && kernel_end <= seg_end) 
		{
			/* Savemos que seg_start es menor que IOM_END */
			seg_end = IOM_END;
			
			/*
                         * El otro pedazo seria desde el final del kernel 
                         * hasta el fin de segmento 
                         */
			seg_start1 = kernel_end;
			seg_end1 = seg_end;
		}

		#define ZONE_DMA_END 16*1024*1024 /* 16 MB*/
		
		/* First chunk */
		if (seg_start != seg_end)
		{
			/* 
			 * Si seg_start < 16MB lo ponemos en ZONE_DMA
			 */
			if (seg_start < ZONE_DMA_END) 
			{
				u_int32_t tmp;

				/*
				 * Nos fijamos si el fin del segmento se 
                                 * pasa de los 16 MB
				 */
				
				tmp = (seg_end > ZONE_DMA_END) ? ZONE_DMA_END : seg_end;
				if (tmp != seg_start) 
				{
					/*
                                         * Agregar el segmento a la lista 
                                         * de libres 
                                         */
					vm_add_segment
					(
						PFN_DOWN(seg_start), 
						PFN_DOWN(tmp),
						PFN_DOWN(seg_start), 
						PFN_DOWN(tmp),
						VM_FREELIST_DMA
					);
				}
				
				seg_start = tmp;
			}
			
			/*
			 * Si sobro un pedazo del codigo anterior o si ni siquiera 
			 * entro agregamos el segmento a ZONE_NORMAL
			 */
			if (seg_start != seg_end) 
			{
				/** Agregar el segmento a la lista de libres */
				vm_add_segment
				(
					PFN_DOWN(seg_start), 
					PFN_DOWN(seg_end),
					PFN_DOWN(seg_start), 
					PFN_DOWN(seg_end),
					VM_FREELIST_NORMAL
				);
			}
		}

		/* Second chunk */
		if (seg_start1 != seg_end1) 
		{
			if (seg_start1 < ZONE_DMA_END) 
			{
				u_int32_t tmp;

				tmp = (seg_end1 > ZONE_DMA_END) ? ZONE_DMA_END : seg_end1;
				if (tmp != seg_start1) 
				{
					/** Agregar el segmento a la lista de libres */
					vm_add_segment
					(
						PFN_DOWN(seg_start1), 
						PFN_DOWN(tmp), 
						PFN_DOWN(seg_start1), 
						PFN_DOWN(tmp), 
						VM_FREELIST_DMA
					);
				}
				
				seg_start1 = tmp;
			}

			if (seg_start1 != seg_end1) 
			{
				/** Agregar el segmento a la lista de libres */
				vm_add_segment
				(
					PFN_DOWN(seg_start1), 
					PFN_DOWN(seg_end1),
					PFN_DOWN(seg_start1), 
					PFN_DOWN(seg_end1),
					VM_FREELIST_NORMAL
				);
			}
		}
	}
	
	return;
}

/*
 * vm_page_t: Each page on the system is described by this 
 * structure.
 */
typedef struct vm_page 
{
	TAILQ_ENTRY(vm_page) pageq;		/* queue info for FIFO queue or free list (P) */
	TAILQ_ENTRY(vm_page) hashq;		/* hash table links (O)*/
	TAILQ_ENTRY(vm_page) listq;		/* pages in same object (O)*/
	
	struct vm_anon *uanon;		/* anon (O,P) */
	struct uvm_object *uobject;	/* object (O,P) */
	
	uint64_t offset; /* offset into object (O,P) */
	
	uint16_t flags; /* object flags [O] */
	uint16_t loan_count;
	uint16_t wire_count; /* wired down map refs [P] */
	uint16_t pqflags; /* page queue flags [P] */
	
	paddr_t phys_addr; /* physical address of page */
} vm_page_t;

/*
 * vm_page_init: initialize the page queues, allocate vm page structures for
 * available physical memory and place them on the free list.
 */
void
vm_page_init(void)
{
	uint32_t i, j;
	uint32_t pagecount;
	uint32_t freepages = 0;
	
	vm_page_t *pagearray;

	if (vm.nsegments == 0)
		panic("uvm_page_bootstrap: no memory pre-allocated");

	/* calculate the number of free pages */
	for (i = 0 ; i < vm.nsegments ; i++)
		freepages += (vm.segments[i].end - vm.segments[i].start);
	
	/** pagecount = the total number of pages we can use */
	pagecount = ((freepages + 1) << PAGE_SHIFT) /
	    (PAGE_SIZE + sizeof(struct vm_page));
	
	/* TODO Aca usar el bootmem allocator */
	bucketarray = (void *) uvm_pageboot_alloc
	(
		(bucketcount * sizeof(struct pgflbucket)) +
		(pagecount * sizeof(struct vm_page))
	);
	
	if(!bucketarray)
		panic("vm_page_init: Cannot allocate memory for Page Array and Bucket");

	/* pagearray esta despues del bucket array */
	pagearray = (struct vm_page *) (bucketarray + bucketcount);

	for (i = 0; i < VM_NFREELIST; i++)
	{
		uvm.page_free[i].pgfl_buckets =
		    (bucketarray + (i * uvmexp.ncolors));

		uvm_page_init_buckets(&uvm.page_free[i]);
	}

	/* inicializamos a 0 el array de paginas */
	memset(pagearray, 0, pagecount * sizeof(struct vm_page));

	/*
	 * init the vm_page structures and put them in the correct place.
	 */

	/** por cada segmento fisico detectado */
	for (i = 0 ; i < vm.nsegments ; i++)
	{
		/** calculamos la cantidad de paginas que hay */
		n = vm.segments[i].end - vm.segments[i].start;

		/* set up page array pointers */
		vm.segments[i].pages = pagearray;
		vm.segments[i].last_page = vm.segments[i].pages + (n - 1);

		/* now increment the pagearray pointer to skip 'n' vm_page structs */
		pagearray += n;
		pagecount -= n;
		
		/* init and free vm_pages (we've already zeroed them) */
		paddr = PFN_ADDR(vm.segments[i].start);

		/** por cada pagina en el segmento */
		for (j = 0 ; j < n ; j++, paddr += PAGE_SIZE)
		{
			/* asignamos la direccion fisica */
			vm.segments[i].pages[j].phys_addr = paddr;
			
			/* add page to free pool */
			uvm_pagefree(&vm.segments[i].pages[j]);
		}
	}
}

/*
 * vm_init: init the VM system
 */

void
vm_init(void)
{
	/* Step 0: Zero the VM structure */
	memset(&vm, 0, sizeof(vm));
	
	/* Step 1: Set page size and values that depends on it */
	vm_set_page_size();
	
	/* Step 2: Parse boot info and extract physical segments */
	vm_segments_init();
	
	/* Step 3: Initialize the page structures for each segments */
	vm_page_init();
}

/* 
 * vm_segment_t: describes one segment of physical memory 
 */
typedef struct vm_physseg
{
	paddr_t	start;				/* PF# of first page in segment */
	paddr_t	end;				/* (PF# of last page in segment) + 1 */
	paddr_t	avail_start;		/* PF# of first free page in segment */
	paddr_t	avail_end;			/* (PF# of last free page in segment) +1  */
	
	#define VM_FREE_LIST_NORMAL 0
	#define VM_FREE_LIST_DMA 1
	#define VM_FREE_LIST_HIGH 2
	uint32_t free_list;				/* which free list they belong on */
	
	struct	vm_page *pages;		/* vm_page structures (from start) */
	struct	vm_page *last_page;	/* vm_page structure for end */
	
	#define VM_PHYS_SEGMENT_AVAILABLE 1
	#define VM_PHYS_SEGMENT_UNAVAILABLE 0
	uint32_t available;			/* 1 if available 0 if not */
} vm_segment_t;

/* 
 * vm_system_t: contains information about the VM manager 
 */
typedef struct vm_main
{
	#define VM_MAX_SEGMENTS 10
	
	/* Segments describing the physical memory */
	vm_segment_t segments[VM_MAX_SEGMENTS];
	
	/* Actual number of physical segments loaded into the VM manager */
	uint32_t nsegments = 0;
	
	/* Page size related stuff, usefull for some calculations */
	uint32_t page_size;
	uint32_t page_shift;
	uint32_t page_mask;
} vm_system_t;

/* 
 * GLOBALS 
 */
vm_system_t vm;

/* 
 * vm_add_segment(): Add a physical segment to the VM manager 
 */
void
vm_add_segment
(
	paddr_t start, 
	paddr_t end, 
	paddr_t avail_start, 
	paddr_t avail_end, 
	uint32_t free_list
)
{
	vm_segment_t *ptr;
	
	if (free_list >= VM_FREE_LIST_NORMAL || free_list < VM_FREE_LIST_HIGH)
	{
		panic("vm_add_segment: bad free list %d", free_list);
	}
	
	if (start >= end)
	{
		panic("vm_add_segment: start >= end");
	}
	
	if(vm.nsegments == VM_MAX_SEGMENTS)
	{
		printk("vm_add_segment: unable to load physical memory\n");
		printk("\t%d segments allocated, ignoring 0x%llx -> 0x%llx\n",
		    VM_PHYSSEG_MAX, (long long)start, (long long)end);
		printk("\tincrease VM_PHYSSEG_MAX\n");
		return;

		return;
	}
	
	ptr = &vm.segments[nsegments];
	
	ptr->avail_start 	= avail_start;
	ptr->avail_end 		= avail_end;
	ptr->start 			= start;
	ptr->end 			= end;
	
	ptr->free_list 		= free_list;
	
	// TODO Agregar esto
	ptr->last_page 		= NULL;
	ptr->pages 			= NULL;
	
	ptr->available 		= VM_PHYS_SEGMENT_AVAILABLE;
	
	vm.nsegments++;
	
	return;
}

/* 
 * vm_del_segment(): Del a physical segment from the VM manager 
 */
void
vm_del_segment(uint32_t segno)
{
	vm_segment_t tmp;
	
	if(segno >= vm.nsegments)
	{
		// TODO Agregar codigo de error aca
		return;
	}
	
	/* mark as unavailable */
	vm.segments[segno].available = VM_PHYS_SEGMENT_UNAVAILABLE;
	
	/* Here we swap the segments in case we want to recover the unavailable segment */
	tmp = vm.segments[segno];
	vm.segments[segno] = vm.segments[vm.nsegments - 1];
	vm.segments[vm.nsegments - 1] = tmp;
	
	vm.nsegments--;
}

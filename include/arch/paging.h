#ifndef PAGING_H_
#define PAGING_H_

#include <lib/string.h>
#include <lib/list.h>
#include <asm/types.h>
#include <arch/specialreg.h>
#include <arch/registers.h>
#include <arch/asm.h>
#include <arch/irq.h>
#include <locking/atomic.h>

/*
 * Page fault error codes, these are used inside the page fault
 * handler.
 * 
 * bit 0 == 0 means no page found, 1 means protection fault
 * bit 1 == 0 means read, 1 means write
 * bit 2 == 0 means kernel, 1 means user-mode
 * bit 3 == 1 means use of reserved bit detected
 * bit 4 == 1 means fault was an instruction fetch
 */
#define PF_PROT_FAULT(x) (x & 1)
#define PF_PAGE_FAULT(x) (!(PF_PROT_FAULT(x)))
#define PF_WR_ACCESS(x) (x & 2)
#define PF_RD_ACCESS(x) (!(PF_WR_ACCESS(x)))
#define PF_USER_MODE(x) (x & 4)
#define PF_KERN_MODE(x) (!(PF_USER_MODE(x)))
#define PF_INST_FETCH(x) (x & 16)

/*
 * Page dir and page tables number of entries
 */
#define PAGE_DIR_ENTRIES 	1024
#define PAGE_TABLE_ENTRIES 	1024
#define PAGE_DIR_SIZE		(PAGE_DIR_ENTRIES*sizeof(pde_t))
#define PAGE_TABLE_SIZE		(PAGE_TABLE_ENTRIES*sizeof(pte_t))

#define PAGE_SHIFT			12
#define PAGE_SIZE 			(1UL << PAGE_SHIFT)
#define PAGE_MASK			(~(PAGE_SIZE - 1))
#define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1) & PAGE_MASK)
#define PAGE_ROUND(x) 		((x) & PAGE_MASK)
#define PAGE_ROUND_UP(x)	(PAGE_ROUND(x) + PAGE_SIZE)
#define PAGE_INDEX(x)		((x) >> PAGE_SHIFT)

#define PAGE_OFFSET 		0xc0000000

#define PFN_ALIGN(x)		(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
#define PFN_UP(x)			(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
#define PFN_DOWN(x)			((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)			((x) << PAGE_SHIFT)
#define PFN_VIRT(x)			(((x) << PAGE_SHIFT) + PAGE_OFFSET)
#define PFN_ADDR(x)			((x) << PAGE_SHIFT)

/* Amount of virtual memory available */
#define VIRTUAL_MEMORY		(((PAGE_SIZE) << 20) - 1)

/* helpful defines */
#define PAGE_ABSENT			0
#define PAGE_PRESENT		1
#define PAGE_READ_ONLY 		0
#define PAGE_READ_WRITE 	1

#define PAGE_USER_PRIV 		1
#define PAGE_KERNEL_PRIV 	0

#define PAGE_ACCESSED 		1
#define PAGE_NOT_ACCESSED 	0
#define PAGE_CLEAN 			0 
#define PAGE_DIRTY 			1
#define PAGE_SIZE_4KB 		0
#define PAGE_SIZE_4MB 		1
#define PAGE_GLOBAL 		1
#define PAGE_LOCAL  		0
#define PAGE_NOWTHR			0
#define PAGE_WTHR 			1
#define PAGE_NOCACHE		0
#define PAGE_CACHE			1

/* FIXME: Por ahi estan alrevez estos campos, averiguar */
typedef struct page_directory_entry
{
	unsigned pd_present:1;			/* is currently loaded ? */
	unsigned pd_read_write:1;		/* 0 = RO; 1 = RW */
	unsigned pd_user_supervisor:1;	/* 0 = SUPERVISOR; 1 = USER */
	unsigned pd_write_through:1;	/* TODO: research */
	unsigned pd_cache_disabled:1;	/* TODO: research */
	unsigned pd_accessed:1;			/* indicates accesses */
	unsigned pd_dirty:1;			/* exists only if PSE is on */
	unsigned pd_page_size:1; 		/* 0 = 4kb pages ; 1 = 2 or 4 MB pages */
	unsigned pd_global_page:1;		/* exists only if PSE is on */
	unsigned pd_unused:3; 			/* 3 + 1 bit from pd_global_page */
	unsigned pd_address:20;			/* base address of a page table */
} pde_t;

/* FIXME: Por ahi estan alrevez estos campos, averiguar */
typedef struct page_table_entry
{
	unsigned pd_present:1;			/* is currently loaded ? */
	unsigned pd_read_write:1;		/* 0 = RO; 1 = RW */
	unsigned pd_user_supervisor:1;	/* 0 = SUPERVISOR; 1 = USER */
	unsigned pd_write_through:1;	/* TODO: research */
	unsigned pd_cache_disabled:1;	/* TODO: research */
	unsigned pd_accessed:1;			/* indicates accesses */
	unsigned pd_dirty:1;			/* indicates whether a page 
									 * has been written */
	unsigned pd_pat:1; 				/* page size extension */
	unsigned pd_global_page:1;		/* page will not be invalidated
									 * in the tlb, it might be useful
									 * to mark kernel pages as global*/
	unsigned pd_unused:3;			/* not used */
	unsigned pd_address:20;			/* base address of a page frame */ 
} pte_t;

/*
 * Every page in the system has an associated
 * page_t structure to describe and keep track
 * of its usage.
 */

typedef struct page
{
    #define PG_ACTIVE		1
    #define PG_DIRTY		1	/* La pagina necesita ser dumpeada a disco */
    #define PG_IOERROR		1	/* Sucedio un error luego de I/o */
    #define PG_LOCKED		1	/* Cuando la pagina es siendo usada para I/O */
    #define PG_REFERENCED 	1	/* */
    #define PG_RESERVED		1	/* La pagina no se puede swapear */
    
    /*
     * El flag en uso (usado por ejemplo en el buddy lo podriamos interpretar
     * como pg_refcount == 0. hay que fijarse si realmente se usa refcoutn
     * para llevar el numero de referencias siempre. Por que por ahi hay casos
     * en los que una pagina esta en uso pero no teien referencias. VERIFICAR
     */
    
    int		pg_flags;
    atomic_t	pg_refcount;		/* para implementar COW */
    atomic_t 	pg_map_count;		/* the number of pte's that refer to the page */
    
    /* 
     * La direccion virtual no la necesitamos en realidad
     * por que tenemos el indice dentro del mem_map, si tenemos el indice
     * tenemos por lo tanto el PFN de la pagina.
     * Por lo unico que la podemos llegar a necesitar es para
     * implementar el concepto de HIGH_MEM o si estamos en una arquitectura
     * NUMA o algo asi (Creo)
     */
    vaddr_t 	pg_virtual_addr;
    
    /* Esta opcion es para implementar el buddy allocator sin el bitmap como en 2.6 */

    /*
     * TODO: EL orden como mucho va a ser 10d, es decir 1010b (4 bits, lo
     * podriamos encodear en alguno de los campos de flags de la struct
     * El orden del bloque de paginas que comienza en esta pagina 
     */
    uint32_t	pg_order;
    atomic_t	pg_count;
    
    struct list_head head;
} page_t;

static inline unsigned long 
virt_to_phys(volatile void *address)
{
	return ((unsigned long)(address) - PAGE_OFFSET);
}

static inline void *
phys_to_virt(unsigned long address)
{
	return ((void *)((unsigned long)(address) + PAGE_OFFSET));
}

extern page_t *mem_map;

static inline void *
page_to_virt(page_t *page)
{
    return phys_to_virt(PFN_PHYS(page - mem_map));
}

static inline page_t *
virt_to_page(void *address)
{
    return &mem_map[PFN_DOWN(virt_to_phys(address))];
}

/*
 * Page table helper functions.
 */
static inline int pte_user	  (pte_t pte)	{ return pte.pd_user_supervisor; }
static inline int pte_read	  (pte_t pte)	{ return (!pte.pd_read_write); }
static inline int pte_dirty	  (pte_t pte)	{ return pte.pd_dirty; }
static inline int pte_write	  (pte_t pte)	{ return pte.pd_read_write; }
static inline int pte_huge	  (pte_t pte)	{ return pte.pd_pat; }
static inline int pte_present (pte_t pte)	{ return pte.pd_present; }

static inline int pte_swapped (pte_t pte)   { return (*((uint32_t *) &pte) & 0x40); }
static inline int pte_new     (pte_t pte)   { return (*((uint32_t *) &pte) == 0); }

/*
 * The following only works if pte_present() is not true.
 */

/*
static inline void pte_rdprotect	(pte_t *pte)	{ pte.pte_low &= ~_PAGE_USER; }
static inline void pte_exprotect	(pte_t *pte)	{ pte.pte_low &= ~_PAGE_USER; }
static inline void pte_mkread		(pte_t *pte)	{ pte.pte_low |= _PAGE_USER; }
static inline void pte_mkexec		(pte_t *pte)	{ pte.pte_low |= _PAGE_USER; }
*/
static inline void pte_mkclean		(pte_t *pte)	{ pte->pd_dirty      = PAGE_CLEAN; }
static inline void pte_mkold		(pte_t *pte)	{ pte->pd_accessed   = PAGE_ACCESSED; }
static inline void pte_mkdirty		(pte_t *pte)	{ pte->pd_dirty 		= PAGE_DIRTY; }
static inline void pte_mkwrite		(pte_t *pte)	{ pte->pd_read_write = PAGE_READ_WRITE; }
static inline void pte_wrprotect	(pte_t *pte)	{ pte->pd_read_write = PAGE_READ_ONLY; }
static inline void pte_mkhuge		(pte_t *pte)	{ pte->pd_pat 		= PAGE_SIZE_4MB; }

/*
 * Funciones para acceder a las entradas del Directorio de paginas
 */
static inline int pde_present (pde_t pde)	{ return pde.pd_present; }

#define clear_page(page)	memset((void *)(page), 0, PAGE_SIZE)
#define copy_page(to,from)	memcpy((void *)(to), (void *)(from), PAGE_SIZE)

/*
 * The page directory can reside at any 4K boundary
 * since the low order 12 bits of
 * the address are set to zero.
 */
typedef struct page_directory
{
	pde_t entry[PAGE_DIR_ENTRIES] 
	                  __attribute__ ((aligned (4096)));	
} pdir_t;

/*
 * The same goes for page_table :)
 */
typedef struct page_table
{
	pte_t entry[PAGE_TABLE_ENTRIES]
	                __attribute__ ((aligned (4096)));
} ptable_t;

typedef int flag_t;

/*
 * Zone modifiers
 */
#define PG_NORMAL		/*  */
#define PG_HIGHMEM		/* Allocate from ZONE_HIGHMEM */

/*
 * Action Modifiers
 */
#define PG_NOFAIL		/* The allocation cannot fail. */
#define PG_HIPRIORITY	/* The allocator can access emergency pools. */
#define PG_WAIT			/* The allocator can sleep. */
#define PG_IO		/* The allocator can start disk I/O. */
#define PG_FS		/* The allocator can start filesystem I/O. */

/*
 * Type Flags
 */
#define PG_ATOMIC	/* high priority and must not sleep (interrupt handlers) */
#define PG_KERNEL	/* normal, might block */
#define PG_USER		/* normal, might block (for user-space processes) */
#define PG_DMA		/* allocation from ZONE_DMA */
#define PG_NOIO		/* can block, but not initiate disk I/O. */
#define PG_NOFS		/* can block, init disk I/O but not initiate fs I/O */

void 	init_temp_paging(void);
void    init_paging(void);
void 	page_fault_handler	(icontext_t *);

#define PAGE_DIR_SHIFT		22
#define PAGE_TABLE_SHIFT	12

/*
 * each virtual address is an index into the 
 * page directory. This funcion returns this index
 */
static inline uint16_t pdir_get_index(vaddr_t address)
{
	return ((address >> PAGE_DIR_SHIFT) & (PAGE_DIR_ENTRIES-1));
}

/*
 * Returns the associated page directory entry (pde)
 * to address.
 */
static inline pde_t *pdir_get_entry(pde_t *pdir, vaddr_t address)
{
	return &pdir[pdir_get_index(address)];
}

static inline void pdir_set_entry(pde_t *e, paddr_t address)
{
	e->pd_address = address;
	e->pd_present = 1;
}

static inline uint16_t ptable_get_index(vaddr_t address)
{
	return ((address >> PAGE_TABLE_SHIFT) & (PAGE_TABLE_ENTRIES-1));
}

static inline pte_t *ptable_get_entry(pte_t *ptable, vaddr_t address)
{
	return &ptable[ptable_get_index(address)];
}

static inline void ptable_set_entry(pte_t *e, paddr_t address)
{
	e->pd_address = address;
	e->pd_present = 1;
}

/*
 * sets cr3 register to point to a valid pagedir
 */

static inline void set_page_directory(pdir_t *page_dir)
{
	asm
	(
		"movl %0, %%cr3\n"
			:
			: "r" ((long) page_dir)
	);
}

static inline pdir_t *get_page_directory(void)
{
	pdir_t *addr = NULL;
	
	asm
	(
		"movl %%cr3, %0"
			: "=r" ((long) addr)
	);
	
	return addr;
}

/*
 * Enables paging and protection (PG Flag)
 * by setting the bit 31 of cr0 register
 * 
 * FIXME: Add clobbered registers
 */
static inline void enable_paging(void)
{
	__asm__ __volatile__ 
	(
		"movl %%cr0, %%eax\n"
		"orl %0, %%eax\n"
		//"orl $0x80000000, %%eax\n"
		"movl %%eax, %%cr0\n"
		:
		: "r" (CR0_PE|CR0_PG|CR0_NE|CR0_TS|CR0_EM|CR0_MP)
		: "%eax"
	);
}

/*
 * Returns 1 if paging is enabled, 0 if not.
 */
static inline int paging_enabled(void)
{
	return (read_cr0() & 0x80000000);
}

/*
 * Returns the address of the page fault
 */
static inline vaddr_t get_page_fault_address(void)
{
	return (read_cr2());
}

/*
 * Enabes PSE (PAge size extension), 
 * if PSE is enabled we can use 4MB pages
 *
 * FIXME: Add clobbered registers
 */
static inline void enable_pse(void)
{
	asm
	(
		"movl %%cr4,%%eax\n\t"
		"orl $0x00000010,%%eax\n\t"
		"movl %%eax,%%cr4\n\t"
			:
			:
			: "%eax"
	);
}

/*
 * Enables PGE (page global extension) so we can
 * mark kernel pages among other as global and so 
 * we can avoid TLB misses.
 */

static inline void enable_pge(void)
{
	asm
	(
		"movl %%cr4, %%eax\n\t"
		"orl $0x00000080, %%eax\t\n"
		"movl %%eax,%%cr4\n\t"
			:
			:
			: "%eax"
	);
}

/*
 * Enables physical address extension (PEA: 36 bit addressing)
 * 
 * FIXME: Add clobbered registers
 */
static inline void enable_pae(void)
{
	asm
	(
		"movl %%cr4, %%eax\n\t"
		"orl $0x00000020, %%eax\t\n"
		"movl %%eax,%%cr4\n\t"
			:
			:
			: "%eax"
	);
}

/* NOTE:
 * When the present flag is clear for a page-table or 
 * page-directory entry, the operating
 * system or executive may use the rest of the entry 
 * for storage of information such as
 * the location of the page in the 
 * disk storage system
 *
 *  -------------------------- 
 * |31        ...          1|0|
 *  --------------------------
 * |                        |0|
 *  --------------------------
 * 
 */

static void inline set_page_directory_entry
(
	pde_t *ent,
	unsigned int addr,
	unsigned accessed,
	unsigned cache,
	unsigned write_through,
	unsigned priv,
	unsigned access,
	unsigned present,
	unsigned size,
	unsigned global,
	unsigned dirty
)
{
	/* Page table address */
	ent->pd_address				= addr >> 12;
	
	ent->pd_page_size			= size;
	ent->pd_accessed			= accessed;
	ent->pd_cache_disabled		= cache;
	ent->pd_write_through		= write_through;
	ent->pd_user_supervisor		= priv;
	ent->pd_read_write			= access;
	ent->pd_present				= present;
	ent->pd_unused 				= 0;
	ent->pd_global_page			= global;
	ent->pd_dirty				= dirty;
}

static inline void set_page_table_entry
(
	pte_t *ent,
	unsigned int addr,
	unsigned global,
	unsigned pat,
	unsigned dirty,
	unsigned accessed,
	unsigned cache,
	unsigned wt,
	unsigned priv,
	unsigned access,
	unsigned present
)
{
	ent->pd_address 		= addr >> 12;
	ent->pd_unused 			= 0;
	ent->pd_global_page 	= global;
	ent->pd_pat 			= pat;
	ent->pd_dirty 			= dirty;
	ent->pd_accessed 		= accessed;
	ent->pd_cache_disabled 	= cache;
	ent->pd_write_through 	= wt;
	ent->pd_user_supervisor = priv;
	ent->pd_read_write 		= access;
	ent->pd_present 		= present;
}


#include <kernel/bootmm.h>

static inline pte_t *pgtable_create()
{
	return (pte_t *) bootmem_alloc(PAGE_SIZE);
}

static inline pde_t *pgdir_create()
{
	return (pde_t *) bootmem_alloc(PAGE_SIZE);
}

#endif /*PAGING_H_*/

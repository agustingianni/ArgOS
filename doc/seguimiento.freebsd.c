locore.S
	main();
	
init_main.c
	main():
		uvm_init();
uvm_init.c
	uvm_init(void):
		uvm_page_init(&kvm_start, &kvm_end);
		
		
Manejo de memoria

/*
 * A page free list consists of free pages of unknown contents and free
 * pages of all zeros.
 */
#define	PGFL_UNKNOWN	0
#define	PGFL_ZEROS	1
#define	PGFL_NQUEUES	2

struct pgflbucket
{
	struct pglist pgfl_queues[PGFL_NQUEUES];
};

struct pgfreelist
{
	struct pgflbucket *pgfl_buckets;
};

struct vm_page
{
	TAILQ_ENTRY(vm_page)	pageq;		/* queue info for FIFO queue or free list (P) */
	TAILQ_ENTRY(vm_page)	hashq;		/* hash table links (O)*/
	TAILQ_ENTRY(vm_page)	listq;		/* pages in same object (O)*/

	struct vm_anon			*uanon;		/* anon (O,P) */
	struct uvm_object		*uobject;	/* object (O,P) */
	voff_t					offset;		/* offset into object (O,P) */
	uint16_t				flags;		/* object flags [O] */
	uint16_t				loan_count;	/* number of active loans
						 				 * to read: [O or P]
						 				 * to modify: [O _and_ P] */
	uint16_t				wire_count;	/* wired down map refs [P] */
	uint16_t				pqflags;	/* page queue flags [P] */
	paddr_t					phys_addr;	/* physical address of page */

	#ifdef __HAVE_VM_PAGE_MD
	struct vm_page_md	mdpage;		/* pmap-specific data */
	#endif
};

/*
 * vm_physseg: describes one segment of physical memory
 */
struct vm_physseg
{
	paddr_t	start;				/* PF# of first page in segment */
	paddr_t	end;				/* (PF# of last page in segment) + 1 */
	paddr_t	avail_start;		/* PF# of first free page in segment */
	paddr_t	avail_end;			/* (PF# of last free page in segment) +1  */
	int	free_list;				/* which free list they belong on */
	struct	vm_page *pages;		/* vm_page structures (from start) */
	struct	vm_page *last_page;	/* vm_page structure for end */
	
	#ifdef __HAVE_PMAP_PHYSSEG
	struct	pmap_physseg pmseg;	/* pmap specific (MD) data */
	#endif
};

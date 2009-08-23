#ifndef _KERNEL_H
#define _KERNEL_H

#include <stdarg.h>
#include <stddef.h>
#include <types.h>

#include <mm/space.h>

typedef struct system_info
{
	size_t sys_memory;		/* Amount of system physical memory */
	size_t sys_lomem;		/* Amount of lower memory           */
	size_t sys_himem;		/* Amount of higher memory          */

	size_t sys_low_pages;		/* Ammount of low memory (normal)   */
	size_t sys_high_pages;		/* Ammount of high memory pages     */
	size_t sys_pages;		/* sum of all pages                 */
	
	size_t sys_page_size;		/* The page size ei. PAGE_SIZE = 4k */

	size_t sys_minpfn;
	size_t sys_maxpfn;
	
	paddr_t sys_page_dir;		/* Kernel page directory */
	
	vaddr_t sys_kern_start;		/* Kernel base address		*/
	vaddr_t sys_kern_end;		/* Kernel end address		*/
	
	paddr_t sys_idt_addr;		/* Address of the current IDT 	*/
	paddr_t sys_gdt_addr;		/* Address of the current GDT 	*/
	
	int	sys_cpu;		/* Cpu class            	*/
	size_t  sys_cpus_number;	/* The number of processors 	*/
	
	/*
	 * El espacio de direcciones actual.
	 * Describe como esta dividida la memoria
	 */
	space_t sys_addr_space;
	
} sysinfo_t;

/* 
 * Esta macro lo que hace es mover el final del kernel 
 * una cantidad de bytes fija hacia adelante. Esto
 * vendria a ser como un malloc primitivo. No checkea
 * que haya suficiente memoria fisica ni nada, dado que
 * es para uso inicial del sistema. Hay que poner
 * atencion cuando se usa.
 */
#define HARD_MALLOC(var, type, size) do				\
{								\
	extern sysinfo_t sys_info;				\
	var = (type) sys_info.sys_kern_end;			\
	sys_info.sys_kern_end += size;				\
} while (0)

#define BITS2BYTES(bits) ((bits + 7) / 8)

#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define LONG_MIN	(-LONG_MAX - 1)
#define ULONG_MAX	(~0UL)
#define LLONG_MAX	((long long)(~0ULL>>1))
#define LLONG_MIN	(-LLONG_MAX - 1)
#define ULLONG_MAX	(~0ULL)

#define ALIGN(x,a)		__ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))

#define abs(x) ({				\
		int __x = (x);			\
		(__x < 0) ? -__x : __x;		\
	})

/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max at all, of course.
 */
#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })


/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})

/*
 * Check at compile time that 'function' is a certain type, or is a pointer
 * to that type (needs to use typedef for the function type.)
 */
#define typecheck_fn(type,function) \
({	typeof(type) __tmp = function; \
	(void)__tmp; \
})

#endif

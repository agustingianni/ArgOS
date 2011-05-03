#ifndef BOOTMM_H_
#define BOOTMM_H_

#include <kernel/kernel.h>
#include <lib/bitmap.h>

typedef struct boot_mem_descriptor
{
	unsigned long start_pfn;
	unsigned long end_pfn;
	unsigned long size;				/* in number of pages */
	unsigned long used;

	unsigned long last_offset;		/* last allocation remain; 0 = FULL */ 
	unsigned long last_pos;
	unsigned long last_success;		/* Previous allocation point. */
	
	bitmap_t mem_bitmap;
} bootmem_t;

int      init_bootmm(sysinfo_t *);
uint32_t bootmem_size(void);
uint32_t bootmem_usage(void);
vaddr_t* bootmem_alloc(size_t size);
void     bootmem_free(vaddr_t *addr, size_t size);
void     bootmem_free_all(void);
int      bootmem_reserve(vaddr_t *addr, size_t size);
uint32_t bootmem_retire(void);

#endif /*BOOTMM_H_*/

#ifndef TSS_H_
#define TSS_H_

#include <mm/mm.h>

#define TSS_DEFAULT_SIZE 104

typedef struct tss_segment
{
   unsigned short backlink, __blh;
   unsigned long esp0;              /*Ring 0 Stack*/
   unsigned short ss0, __ss0h;
   unsigned long esp1;              /*Ring 1 Stack*/
   unsigned short ss1, __ss1h;
   unsigned long esp2;              /*Ring 2 Stack*/
   unsigned short ss2, __ss2h;
   unsigned long cr3, eip, eflags;
   unsigned long eax,ecx,edx,ebx,esp,ebp,esi,edi;
   unsigned short es, __esh;
   unsigned short cs, __csh;
   unsigned short ss, __ssh;
   unsigned short ds, __dsh;
   unsigned short fs, __fsh;
   unsigned short gs, __gsh;
   unsigned short ldt, __ldth;
   unsigned short trace, iomapbase;
} __attribute__ ((packed)) tss_t;

static inline void
load_task_register(struct segment_selector x)
{
    asm volatile
    (
    	"ltr %0"
    	:
    	: "r" (x)
    );
}

#endif /*TSS_H_*/

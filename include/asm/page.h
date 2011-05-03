#ifndef _I386_PAGE_H
#define _I386_PAGE_H

#define PAGE_SHIFT	12
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#endif /* _I386_PAGE_H */

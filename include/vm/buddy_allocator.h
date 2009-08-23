/*
 * File:   buddy_allocator.h
 * Author: gr00vy
 *
 * Created on April 9, 2008, 3:19 AM
 */

#ifndef _BUDDY_ALLOCATOR_H
#define	_BUDDY_ALLOCATOR_H

#include <lib/list.h>
#include <types.h>
#include <arch/paging.h>

/*
 * Devuelve el tamano necesario para el bitmap que va a representar el
 * estado de los buddies (en bytes)
 */

#define BITMAP_SIZE(free_pages, order) \
    ((((((free_pages + (1 << order) - 1) / (1 << order))+1)/2)+7)/8)

typedef struct buddy_freelist
{
    /* Aca tendriamos una lista de paginas libres */
    struct list_head head;

    /* Numero de paginas libres */
    uint32_t free_pages;

    /* Bitmap que contiene informacion acerca de los buddies */
    bitmap_t page_bitmap;
} buddy_list_t;

/*
 * Aca mantenemos informacion imporante sobre el buddy allocator
 */
typedef struct buddy_allocator
{
    /*
     * Aca definimos el maximo orden que vamos a poder alocar
     * en este caso 2**MAX_ORDER paginas es decir si MAX_ORDER es
     * 10 podremos alocar como maximo 4MB de memoria contigua
     */
    #define MAX_ORDER 11
    buddy_list_t free_lists[MAX_ORDER];

} buddy_t;

typedef unsigned int pgflags_t;

page_t *buddy_get_address(page_t *page, uint32_t page_idx, uint32_t order);
page_t *buddy_split(page_t *block, uint32_t buddy_order, uint32_t requested_order);
page_t *buddy_join(page_t *buddy1, page_t *buddy2, uint32_t order);
page_t *buddy_page_alloc(uint32_t order, pgflags_t flags);
void    buddy_page_free(page_t *page, uint32_t order);

void    init_mem_map(void);

void init_buddy(void);

#endif	/* _BUDDY_ALLOCATOR_H */


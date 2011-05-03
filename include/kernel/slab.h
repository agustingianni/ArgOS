#ifndef SLAB_H_
#define SLAB_H_

#include <lib/list.h>
#include <locking/spinlock.h>
#include <types.h>

/*
 * Como en linux, aprovechamos que los campos prev y next
 * no son usados cuando una pagina esta siendo utilizada por
 * el buddy allocator. Ahi ponemos a que cache y slab pertenece la pagina
 */
#define PAGE_GET_CACHE(x) ((cache_t *) (x)->head.next)
#define PAGE_SET_CACHE(x, y) ((x)->head.next = (struct list_head *)(y))

#define PAGE_GET_SLAB(x) ((slab_t *) (x)->head.prev)
#define PAGE_SET_SLAB(x, y) ((x)->head.prev = (struct list_head *)(y))

/*
 * A cache object contains lots of slabs
 */
typedef struct kernel_cache
{
    struct list_head slabs_full;
    struct list_head slabs_partial;
    struct list_head slabs_free;

    /* Alinear a la cache L1 del CPU */
    #define CACHE_ALIGN_L1      1

    /* Alinear a sizeof(void *); */
    #define CACHE_ALIGN_WORD    2

    /* Esta cache no va a ser procesada por kcache_reap() */
    #define CACHE_NO_REAP       4

    /* unir en una misma cache objetos de similar tamano */
    #define CACHE_MERGE_OBJS    8

    /* La struct slab_t va a estar fuera del buffer de objetos */
    #define CACHE_SLAB_OFF_BUFFER 16

    unsigned int cache_flags;

    /* This is the size of each object packed into the slab. */
    unsigned int object_size;

    /* object alignment */
    size_t object_align;

    /* Number of objects per slab */
    unsigned int num;

    /* Protect the structure from concurrent accessses */
    spinlock_t lock;

    /* Size of the slab in pages (2**order) */
    unsigned int order;

    /* Each slab stores objects in different cache lines */
    size_t colour; /* # of different offsets that can be used */
    unsigned int colour_off; /* amount to offset each slab by  */
    unsigned int colour_next; /* the next offset to be used */

    struct kernel_cache *slab_cache;

    /* constructor function to be called to initialize each new object */
    void (*object_ctor) (void *, struct kernel_cache *, unsigned long);

    /* de-constructor func */
    void (*object_dtor)(void *, struct kernel_cache *, unsigned long);

    void (*cache_reclaim)(void *);

    /* Next cache on the cache chain. */
    struct list_head next;

    #define KCACHE_NAME_LEN 20
    char cache_name[KCACHE_NAME_LEN + 1];
} cache_t;

/*
 * A slab contains lots of kernel objects
 */
typedef struct kernel_slab {
    /* This is the linked list the slab belongs to */
    struct list_head list;

    /* Color offset from the base address of the first object */
    unsigned long colouroff;

    /* Starting address of the first object within the slab */
    void *memory;
    unsigned int inuse;

    #define SLAB_LAST_ELEMENT 0xffffffff
    #define SLAB_FREE_LIST(x) ((unsigned int *) (((slab_t *) (x)) + 1))
    unsigned int free; /* Index of the next free element */

} slab_t;

cache_t *kcache_create
(
        const char *name,
        size_t size,
        size_t offset,
        unsigned int flags,
        void (*ctor)(void*, cache_t *, unsigned long),
        void (*dtor)(void*, cache_t *, unsigned long)
);

int init_slab(void);
void kcache_destroy(cache_t *cache);
void *kcache_alloc(cache_t *cache);
void kcache_free(cache_t *cache, void *object);
void kcache_grow(cache_t *cache);

cache_t *kcache_find(size_t size);
void *kmalloc(size_t size, unsigned int flags);
void *kcalloc(size_t nmemb, size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t size);

#endif /*SLAB_H_*/

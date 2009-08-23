#include <kernel/slab.h>
#include <kernel/compiler.h>
#include <arch/cache.h>
#include <arch/paging.h>
#include <bug.h>
#include <lib/list.h>
#include <lib/string.h>

#include "vm/buddy_allocator.h"

/* Cache of cache_t objects */
static cache_t cache_cache =
{
    slabs_full      : LIST_HEAD_INIT(cache_cache.slabs_full),
    slabs_partial   : LIST_HEAD_INIT(cache_cache.slabs_partial),
    slabs_free      : LIST_HEAD_INIT(cache_cache.slabs_free),
    object_size     : sizeof (cache_t),
    cache_flags     : CACHE_ALIGN_L1 | CACHE_NO_REAP,
    lock            : {1},
    colour_off      : L1_CACHE_BYTES,
    cache_name      : "kmem_cache",
    next            : LIST_HEAD_INIT(cache_cache.next)
};

/* Size description struct for general caches. */
typedef struct kmalloc_cache_sizes
{
    size_t size;
    cache_t *cache;
} kmalloc_cache_t;

/* Caches usadas por kmalloc de diferentes tamanos */
static kmalloc_cache_t kmalloc_caches[] =
{
    { 32, NULL},
    { 64, NULL},
    { 128, NULL},
    { 256, NULL},
    { 512, NULL},
    { 1024, NULL},
    { 2048, NULL},
    { 4096, NULL},  /* One page */
    { 8192, NULL},  /* Two pages */
    { 0, NULL}
};

/*
 * Funcion que enmascara al buddy allocator para ser usada por
 * el slab allocator
 */
static void *
kcache_alloc_pages(int order)
{
    return (void *) page_to_virt(buddy_page_alloc(order, 0));
}

/*
 * Igual que arriba
 */
static void
kcache_free_pages(void *pages, int order)
{
    buddy_page_free(virt_to_page(pages), order);
}

static void
kcache_estimate
(
    unsigned long gfporder,
    size_t size,
    int flags,
    size_t *left_over,
    unsigned int *num
)
{
    unsigned int buffer_size = PAGE_SIZE << gfporder;
    unsigned int slab_size = 0;
    unsigned int buffctl_size = 0;
    unsigned int i = 0;

    /* La cache esta dentro del mismo buffer de objetos */
    if(!(flags & CACHE_SLAB_OFF_BUFFER))
    {
        slab_size = sizeof (slab_t);
        buffctl_size = sizeof (unsigned int);
    }

    while((i * size) + ((i * buffctl_size) + slab_size) < buffer_size)
        i++;

    if(i > 0)
        i--;

    *num = i;
    *left_over = buffer_size - ((i * size) + ((i * buffctl_size) + slab_size));
}

/*
 * Inicializa la cache de objetos cache_t, es necesario
 * por que para inicializar otras caches, vamos a hacer
 * uso de estructuras cache_t y de algun lado hay que
 * sacarlas
 */
static void
kcache_init(void)
{
    size_t left_over;

    kcache_estimate
    (
        0,
        cache_cache.object_size,
        cache_cache.cache_flags,
        &left_over,
        &cache_cache.num
    );

    cache_cache.colour = left_over / cache_cache.colour_off;
    cache_cache.colour_next = 0;
    
    printk("Cache name = %s | Order = %u | Objs = %u | Size = %u\n",
           cache_cache.cache_name, cache_cache.order, cache_cache.num, 
           cache_cache.object_size);
}

extern int snprintf(char * buf, size_t size, const char *fmt, ...);

static void
kcache_init_basic_caches()
{
    char name[KCACHE_NAME_LEN + 1];

    int i = 0;

    while(kmalloc_caches[i].size)
    {
        snprintf(name, sizeof (name), "kmalloc_%u_bytes",
                 kmalloc_caches[i].size);

        kmalloc_caches[i].cache =
            kcache_create(name, kmalloc_caches[i].size,
                          0, CACHE_ALIGN_L1, NULL, NULL);

        if(!kmalloc_caches[i].cache)
            BUG();

        i++;
    }
}

/*
 * Rutina principal que se encarga de inicializar todo el
 * subsistema del slab
 */
int
init_slab(void)
{
    printk("Initializing cache_cache slab\n");
    kcache_init();

    printk("Initializing general purpose caches\n");
    kcache_init_basic_caches();

    printk("Finished initializing SLAB Allocator, now its safe to use it\n");
    return 0;
}

/*
 * Returns wether exists or not a cache_t with the given name
 */
static int
kcache_exists(const char *name)
{
    struct list_head *pos;
    int ret = 0;

    list_for_each(pos, &cache_cache.next)
    {
        cache_t *t = list_entry(&pos, cache_t, next);
        if(!strcmp(t->cache_name, name))
        {
            ret = 1;
            printk("Invalid cache_t name, it already exists\n");
            break;
        }
    }

    return ret;
}

/*
 * Creates a new cache and adds it to the cache chain.
 */
cache_t *
kcache_create
(
    const char *name,
    size_t size,
    size_t offset,
    unsigned int flags,
    void (*ctor)(void*, cache_t *, unsigned long),
    void (*dtor)(void*, cache_t *, unsigned long)
)
{
    size_t left_over, align;

    /* Inicializarla cxon una cache de cache_t objects */
    cache_t *cache = NULL;

    /*
     * Verificamos que el nombre este bien, que haya
     * contructor si hay destructor y que el offset no se pase
     */

    BUG_ON(!name);
    BUG_ON(strlen(name) > KCACHE_NAME_LEN);
    BUG_ON(dtor && !ctor);
    BUG_ON(offset > size);

    /* Nos fijamos si ya existe una cache con ese nombre */
    if(kcache_exists(name))
        return NULL;

    /* Alocamos la cache */
    cache = (cache_t *) kcache_alloc(&cache_cache);
    if(!cache)
        return NULL;

    /* Inicializamos */
    /* TODO Reemplazar esto por un ctor que haga esto */
    memset(cache, 0x00, sizeof (cache_t));

    /* seteamos los constructores y destructores */
    cache->object_ctor = ctor;
    cache->object_dtor = dtor;

    cache->cache_flags = flags;

    /* Inicializamos las listas de slabs */
    INIT_LIST_HEAD(&cache->slabs_full);
    INIT_LIST_HEAD(&cache->slabs_partial);
    INIT_LIST_HEAD(&cache->slabs_free);

    #define BYTES_PER_WORD sizeof(void *)
    #define WORD_ALIGN(x) ((x + (BYTES_PER_WORD - 1)) & ~(BYTES_PER_WORD - 1))
    #define SIZE_ALIGN(x, y) ((x + ((y) - 1)) & ~((y) - 1))

    /* Align the object size to the hardware cache or wordsize */
    align = BYTES_PER_WORD;
    if(flags & CACHE_ALIGN_L1)
    {
        /*
         * Si el tamano del objeto es mucho mas chico que el tamano
         * de la linea de cache, probamos reducir el tamano
         * de alineacion para ver si podemos meter mas de un objeto
         * por linea de cache
         */
        align = L1_CACHE_BYTES;
        while(2 * size < align)
            align /= 2;
    }

    size = SIZE_ALIGN(size, align);
    cache->object_size = size;

    strcpy(cache->cache_name, name);
    cache->cache_name[KCACHE_NAME_LEN] = '\0';

    #define SLAB_MAX_ORDER 5
    /* Calculate how many objects will fit on a slab */
    for(cache->order = 0; cache->order < SLAB_MAX_ORDER; cache->order++)
    {
        kcache_estimate(cache->order, size, flags,
                        &left_over, &cache->num);

        /* Fragmentacion menor a 1/8 del tamano del bloque */
        if((left_over * 8) <= (PAGE_SIZE << (cache->order)))
            break;
    }

    if(!cache->num)
    {
        printk("Couldn't create cache %s.\n", name);
        kcache_free(&cache_cache, cache);
        return NULL;
    }

    /* Offset must be a multiple of the alignment. */
    offset = SIZE_ALIGN(offset, align);
    if(!offset)
        offset = L1_CACHE_BYTES;

    /* Calculate color offsets. */
    cache->colour_off = offset;
    cache->colour = left_over / offset;

    /* Si el objeto es mas grande que 1/8 del tamano de pagina */
    if(cache->object_size >= (PAGE_SIZE >> 3))
    {
        /* Calcular slab_t size, sizeof(slab_t) + size of buffcntls */
        size_t slab_size = cache->num * sizeof (unsigned int)
            + sizeof (slab_t);
        
        cache->cache_flags |= CACHE_SLAB_OFF_BUFFER;

        /* Los slab_t son creados desde esta chache */
        cache->slab_cache = kcache_find(slab_size);
        if(unlikely(!cache->slab_cache))
            BUG();
    }

    /* Add the new cache to the cache chain. */
    list_add(&cache->next, &cache_cache.next);

    printk("Created cache: name = %s | Order = %u | Objs = %u | Size = %u\n",
       cache->cache_name, cache->order, cache->num, 
       cache->object_size);

    
    return cache;
}

static void
kcache_slab_destroy(cache_t *cache, slab_t *slabs)
{
    struct list_head *p;
    void *object;
    int i;

    list_for_each(p, &slabs->list)
    {
        slab_t *tmp = list_entry(p, slab_t, list);

        list_del(p);

        if(cache->object_dtor)
        {
            for(i = 0; i < cache->num; i++)
            {
                object = tmp->memory + i * cache->object_size;
                cache->object_dtor(object, cache, 0);
            }
        }

        for(i = 0; i < cache->order; i++)
        {

        }

        kcache_free_pages(tmp->memory, cache->order);
        kfree(tmp);
    }
}

/*
 * Destroys all objects in all slabs and frees up all associated memory
 * before moving the cache from the chain.
 */
void
kcache_destroy(cache_t *cache)
{
    slab_t *tmp;

    /* Desencadenamos la cache de la lista de caches */
    list_del(&cache->next);

    /* Liberamos los slabs, free, parciales, full */
    tmp = list_entry(&cache->slabs_free, slab_t, list);
    kcache_slab_destroy(cache, tmp);

    /* En teoria el modulo que uso el slab deberia limpiar solo los objetos */
    if(!list_empty(&cache->slabs_partial))
    {
        tmp = list_entry(&cache->slabs_partial, slab_t, list);
        kcache_slab_destroy(cache, tmp);
        printk("Lista de slabs parciales no estaba limpia\n");
    }

    /* Lo mismo pasa aca, no deberia quedar ningun objeto */
    if(!list_empty(&cache->slabs_full))
    {
        tmp = list_entry(&cache->slabs_full, slab_t, list);
        kcache_slab_destroy(cache, tmp);
        printk("Lista de slabs full no estaba limpia\n");
    }

    /* Liberamos el objeto de la cache de cache_t's */
    kcache_free(&cache_cache, cache);
    return;
}

/*
 * Allocates a single object from the cache and returns it to the caller.
 */
void *
kcache_alloc(cache_t *cache)
{
    void *object;
    slab_t *slab;

    /* Si no tenemos slabs parciales, sacamos memoria de los libres */
    if(unlikely(list_empty(&cache->slabs_partial)))
    {
        /* si no tenemos slabs libres, creamos un slab nuevo */
        if(unlikely(list_empty(&cache->slabs_free)))
            kcache_grow(cache);

        slab = list_entry(cache->slabs_free.next, slab_t, list);

        /* Agregamos el libre a la lista de parciales*/
        list_del(&slab->list);
        list_add(&slab->list, &cache->slabs_partial);
    }
    else
    {
        /* Obtener uno de los slabs parciales*/
        slab = list_entry(cache->slabs_partial.next, slab_t, list);
    }

    /* incrementar la cantidad de objetos en el slab */
    slab->inuse++;

    /* Tenemos un puntero al objeto */
    object = slab->free * cache->object_size + slab->memory;

    /* como sacamos un objeto actualizamos el indice al proximo obj libre */
    slab->free = SLAB_FREE_LIST(slab)[slab->free];

    /* Se lleno el Slab, lo mandamos a slabs full */
    if(unlikely(slab->free == SLAB_LAST_ELEMENT))
    {
        list_del(&slab->list);
        list_add(&slab->list, &cache->slabs_full);
    }
    
    printk("Allocated from: cache = %s | object = %p | inuse = %u\n",
           cache->cache_name, object, slab->inuse);

    return object;
}

/*
 * Frees an object and returns it to the cache.
 */
void
kcache_free(cache_t *cache, void *object)
{
    slab_t *slab = PAGE_GET_SLAB(virt_to_page(object));

    unsigned int objnr = (object - slab->memory) / cache->object_size;

    SLAB_FREE_LIST(slab)[objnr] = slab->free;
    slab->free = objnr;

    /* 3 Decrementar el numero de objetos usados en el slab */
    int inuse = slab->inuse;
    if(unlikely(!--slab->inuse))
    {
        /* El objeto liberado era el unico que habia alocado */
        list_del(&slab->list);
        list_add(&slab->list, &cache->slabs_free);
    }
    else if(unlikely(inuse == cache->num))
    {
        /* El slab estaba lleno, ahora esta parcialmente lleno. */
        list_del(&slab->list);
        list_add(&slab->list, &cache->slabs_partial);
    }

    /*
     * TODO
     * Ver que pasaria si el inuse llega a 0 y mandamos a
     * kcache_reap(cache);
     */
    return;
}

/*
 * This function creates new slabs to store more objects
 */
void
kcache_grow(cache_t *cache)
{
    slab_t *slab;

    void *buffer = kcache_alloc_pages(cache->order);
    if(unlikely(!buffer))
        BUG();

    /* El calculamos el offset que actualmente podemos usar */
    unsigned int offset = cache->colour_next * cache->colour_off;

    /* Ahora el proximo slab se va a crear desde un color mas arriba*/
    cache->colour_next++;

    /* Si el color se pasa de rango, volver a 0 */
    if(cache->colour_next >= cache->colour)
        cache->colour_next = 0;

    /* slab_t se encuentra fuera del buffer de objetos */
    if(likely(cache->cache_flags & CACHE_SLAB_OFF_BUFFER))
    {
        /* Alocamos memoria para el slab */
        slab = kcache_alloc(cache->slab_cache);
        if(unlikely(!slab))
        {
            kcache_free_pages(buffer, cache->order);
            BUG();
        }
    }
    else
    {
        /* El tamano de slab_t es tenido en cuenta por kcache_estimate */
        /* FIXME Aca mejor va a ser slab + SLAB_SIZE - (cache->object_size * cache->num); */
        slab = buffer + offset + (cache->object_size * cache->num);
    }

    /* Este slab uso este offset*/
    slab->colouroff = offset;

    /* Seteamos la memoria que va a usar el slab, le aplicamos el color */
    slab->memory = buffer + offset;

    /* Indice del primer objeto libre */
    slab->free = 0;

    /* Cantidad de objetos usados */
    slab->inuse = 0;

    /* inicializar los ctl buffers */
    int i;
    for(i = 0; i < cache->num; i++)
    {
        SLAB_FREE_LIST(slab)[i] = i + 1;
    }

    /* Es necesario poner un tag para finalizar la busqueda de elementos libres */
    SLAB_FREE_LIST(slab)[i - 1] = SLAB_LAST_ELEMENT;

    /* Agregamos el nuevo slab a la lista de frees */
    list_add(&slab->list, &cache->slabs_free);

    /* Ahora necesitamos setear los punteor del mem_map correctamente */
    size_t pages = (1 << cache->order);
    page_t *page = virt_to_page(buffer);

    for(i = 0; i < pages; i++)
    {
        PAGE_SET_CACHE(page, cache);
        PAGE_SET_SLAB(page, slab);
        page++;
    }
}

/*
 * Used when the system need to get some memory back
 * to the main memory allocator
 */
void
kcache_reap(cache_t *cache)
{

}

/*************************************************
 * KMALLOC & KFREE STUFF
 *************************************************/

/*
 * Busca una cache que pueda almacenar un
 * objeto de un determinado tamano
 */
cache_t *
kcache_find(size_t size)
{
    cache_t *cache = NULL;
    int i = 0;

    while(kmalloc_caches[i].size)
    {
        if(kmalloc_caches[i].size >= size)
        {
            cache = kmalloc_caches[i].cache;
            break;
        }

        i++;
    }

    return cache;
}

/*
 * Allocates size bytes and returns a pointer
 * to the allocated memory.
 *
 * Accede a una cache del slab por defecto
 */
void *
kmalloc(size_t size, unsigned int flags)
{
    void *mem_ptr = NULL;
    cache_t *cache_ptr = kcache_find(size);

    if(likely(cache_ptr))
        mem_ptr = kcache_alloc(cache_ptr);

    return mem_ptr;
}

/*
 * Free memory allocated with kmalloc
 */
void
kfree(void *ptr)
{
    kcache_free(PAGE_GET_CACHE(virt_to_page(ptr)), ptr);
}

/*
 * Changes the size of the memory block pointed to by ptr to size bytes.
 * The contents will be unchanged to the minimum of the old and new  sizes;
 * newly  allocated  memory will be uninitialized.
 * If ptr is NULL, the call is equivalent to malloc(size);
 * if size is equal to zero, the call is equivalent to free(ptr).
 * Unless ptr is NULL, it must have been returned by an earlier call
 * to kmalloc(), kcalloc() or krealloc().  If the area pointed to was
 * moved, a free(ptr) is done.
 */
void *
krealloc(void *ptr, size_t size)
{
    if(unlikely(!ptr))
        return kmalloc(size, 0);

    if(unlikely(size == 0))
    {
        kfree(ptr);
        return NULL;
    }

    cache_t *cache = PAGE_GET_CACHE(virt_to_page(ptr));
    cache_t *tmp = kcache_find(size);

    if(cache == tmp)
        return ptr;

    void *new_mem = kcache_alloc(tmp);
    if(unlikely(!new_mem))
        BUG();

    /* Aca sabemos que las direcciones estan alineadas, usar multiple memcpy */
    memcpy(new_mem, ptr, cache->object_size);

    kfree(ptr);

    return new_mem;
}

/*
 * FIXME: Integer overflows
 *
 * Same as kmalloc but it initializes to zero the
 * allocated memory
 */
void *
kcalloc(size_t n, size_t size)
{
    void *memptr = NULL;

    if(likely((memptr = kmalloc(n * size, 0))))
        memset(memptr, 0x00, n * size);

    return memptr;
}

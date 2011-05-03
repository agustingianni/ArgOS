#ifndef VM_H_
#define VM_H_

#include <arch/paging.h>
#include <lib/list.h>

/*
 * vma_flags
 */
#define VMA_READ	0x00000001
#define VMA_WRITE	0x00000002
#define VMA_EXEC	0x00000004
#define VMA_SHARED	0x00000008

#define VMA_GROWSDOWN	0x00000100	/* general info on the segment */
#define VMA_GROWSUP	0x00000200

typedef struct vm_page
{

} vm_page_t;

/*
 *
 */
typedef struct vm_object
{

} vm_object_t;

/*
 * Cada proceso tiene de 0 a muchos vma's. Estos
 * describen las secciones de memoria mapeadas que
 * posee cada proceso.
 * Cada vm_map_entry_t describe un conjunto de direcciones
 * que tiene las mismas caracteristicas como por ejemplo:
 * todas las direcciones tienen permiso de ejecucion y lectura.
 */
typedef struct vm_entry
{

} vm_map_entry_t;

/*
 * Describe el espacio de direcciones de un proceso en particular
 * Es machine-independent.
 * Tiene una lista de vm_entry_t's que describen los segmentos
 * que posee este espacio de direcciones.
 */
typedef struct vm_map
{
    //vm_space_t *parent;

    vaddr_t     vma_start;
	vaddr_t     vma_end;

	uint32_t    vma_flags;

	struct vma *vma_next;   /* ? */
} vm_map_t;

/*
 *
 */
typedef struct vm_statistics
{

} vm_stats_t;

/*
 * Cada proceso tiene una estructura mm que es el
 * descriptor general de todo lo relacionado con la
 * memoria que usa un proceso. Contiene un puntero al
 * directorio de paginas, la lista de vma's y demas.
 *
 * La estructura esta dentro de la estructura task,
 * y va a ser utilizada seguramente en el manejador
 * de fallos de paginas
 *
 * En esta estructura estan las estructuras machine
 * dependent y machine independent que describen el
 * espacio de memoria virtual de un proceso.
 *
 */
typedef struct vm_space
{
	pde_t *pg_dir;		/* process page directory */

	vm_map_t *vma_list;	/* list of VMAs */

	atomic_t count; 	/* many processes can share the same address space */
} vm_space_t;

int    vma_grow_stack   (vm_map_t *vma);
int    vma_growsdown    (vm_map_t *vma);
int    vma_access_check (vm_map_t *vma, uint32_t access);

vm_map_t *vma_find         (vm_space_t *mm, vaddr_t address);


/* Crea un address space para un proceso */
vm_space_t *vm_create();

/* Destruye un address space */
void vm_destroy(vm_space_t *vm);

/* Crean un map que corresponde al 'vm' dado */
vm_map_t *vm_map_create(vm_space_t *vm);

/* destruye el vm_map */
void vm_map_destroy(vm_map_t *vm_map);

#endif /*VM_H_*/

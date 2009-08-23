#include <vm/buddy_allocator.h>
#include <lib/list.h>
#include <types.h>
#include <lib/bitmap.h>
#include <kernel/bootmm.h>
#include <kernel/utils.h>
#include <arch/paging.h>
#include <bug.h>
#include <debug.h>
#include <assert.h>
#include <arch/bitops.h>

/*
 * Por cada orden creamos un mapa de bits de 2**(order-1) bits
 *
 * Cada bit nos va a representar un conjunto de buddies
 * Si esta en 1 quiere decir que uno de los buddies
 * esta ocupado. Si el bit esta en 0 quiere decir que o los dos
 * estan libres o los dos estan usados. Si al liberar un buddy marcamos
 * en el mapa y luego ese bit quedo en 0 quiere decir que los dos estan libres
 * y podemos promocionarlos un nivel hacia arriba.
 * Esta operacion se raealiza de manera recursiva.
 *
 * Ejemplo: Suponemos que tenemos una memoria de 8 Paginas
 * La dividimos en cuatro bloques de 1, 2, 4 y 8 paginas
 *
 * El tamano del mapa de bit de cada order vendra dado por la sieguiente
 * ecuacion:
 *
 * #bits = #paginas_disponibles / (2**(order-1))
 *
 * |0|0|0|0|0|0|0|0| <--- 8 bits = 2**(4 - 1)
 * |0|0|0|0|
 * |0|0|
 * |0|
 *
 * Ejemplo de alocacion: Suponemos que el sistema empieza
 * con un bloque de memoria de orden 4. Si entra una alocacion
 * de orden 4 listo, la servimos. Ahora, las subsiguientes alocaciones
 * no podran ser sevidas por que todas las listas de paginas libres
 * de todos y cada uno de los ordenes estan vacias.
 * En cambio si entra una alocacion de orden 2 lo que haremos sera
 * recorrer el arreglo de bloques hasta encontrar un bloque lo suficientemente
 * grande como para servir la peticion. Como el unico bloque que esta disponible
 * es de orden 4 lo dividimos en dos bloques de orden 2 y tomamos uno para servir
 * la peticion. El otro, lo agregamos a la lista de libres pero no a la de orden 4
 * sino a la de orden 2. Ahora en el mapa de bits de orden 2 llamamos a la funcion
 * MARK_USED y le pasamos en idx de la pagina usada y el mapa de bits correspondiente
 * al orden (en este caso 2).
 */

buddy_t buddy;

extern sysinfo_t sys_info;

/* Como en linux */
page_t *mem_map;

void
init_mem_map(void)
{
	size_t mem_map_size = sizeof(page_t) * sys_info.sys_pages;

	mem_map = (page_t *) bootmem_alloc(mem_map_size);
	if(!mem_map)
		panic("Error alocando page_t *mem_map");

	int i;
	for(i = 0; i < sys_info.sys_pages; i++)
	{
		atomic_set(&mem_map[i].pg_count, 0);
		atomic_set(&mem_map[i].pg_map_count, 0);
		atomic_set(&mem_map[i].pg_refcount, 0);

		mem_map[i].pg_flags = 0;
		mem_map[i].pg_virtual_addr = (vaddr_t) phys_to_virt(PFN_ADDR(i));
		mem_map[i].pg_order = 0;

		/* TODO Ver que onda con esto */
		INIT_LIST_HEAD(&mem_map[i].head);
	}
}

/*
 * Inicializa el Buddy Allocator
 *
 * Aca lo que hacemos es inicializar las listas de bloques libres
 * de tamano 2^n. Tambien le asignamos memoria a los bitmaps
 * que van a representar el estado de cada bloque de paginas.
 * Los bitmaps en realidad no son necesarios. En un futuro los voy
 * a sacar y voy a implementarlo de la forma que esta en el kernel
 * 2.6
 */
void
init_buddy(void)
{
        int i;
        buddy_list_t *freelist = buddy.free_lists;

        /*
         * No aseguramos que corremos en un sistema de al menos 2^MAX_ORDER
         * paginas. MAX_ORDER = 10 asi que el minimo de memoria seria 4Mbytes
         */
        assert(sys_info.sys_pages >= (1 << MAX_ORDER));

        size_t nbytes = 0;
        for (i = 0; i < MAX_ORDER; i++)
                nbytes += BITMAP_SIZE(sys_info.sys_pages, i);

        char *memory = (char *) bootmem_alloc(nbytes);
        if (!memory)
                panic("Error alocando bitmap para el buddy allocator");

        /* Inicializar los bitmap con los buddies */
        for (i = 0; i < MAX_ORDER; i++)
        {
                /* Inicializamos la lista en vacio */
                INIT_LIST_HEAD(&freelist->head);
                freelist->free_pages = 0;

                freelist->page_bitmap = (bitmap_t) memory;

                /*
                 * Las paginas en un principio las ponemos como todas en uso
                 * asi cuando llamamos a bootmem_retire liberamos solo
                 * las paginas que no estaban en uso y no tenemos que marcar
                 * puntualmente las que estaban siendo usadas.
                 */
                bitmap_zero(freelist->page_bitmap, sys_info.sys_pages);

                memory += BITMAP_SIZE(sys_info.sys_pages, i);

                freelist++;
        }

        return;
}

/*
 * buddy_page_alloc(): Aloca 2**order paginas de memoria fisica y contigua.
 * Los flags son importantes, contienen caracteristicas de la memoria
 * que sera entregada por el alocador.
 *
 * TODO Aca lo que fala es poder decirle de que zona tomar la memoria
 *
 * NOTE: Lo que puedo hacer aca es declarar una variable como static
 * para llevar un indice a el minimo orden que tiene paginas libres asi
 * por ahi nos evitamos un par de ciclos del for y obtenemos memoria
 * un toque mas rapido.
 *
 * NOTE: Uno de los problemas qeu  puede tener el buddy allocator es que
 * en determinadas circunstancias, si agrupamos los buddies y promocionamos
 * en el peor de los casos vamos a tener MAX_ORDER-1 "juntadas"
 * luego si viene una alocacion, vamos a tener MAX_ORDER-1 "spliteadas"
 * se podria ver de posponer las "juntadas" de acuerdo a un cierto criterio
 * para que esto no ocurra. Uno de los criterios podria ser poner un minimo
 * de bloques para cada orden y si ese ciretrio no se cumple empezar a juntar
 * bloques.
 */

page_t *
buddy_page_alloc(uint32_t order, pgflags_t flags)
{
        /*
         * TODO: Aca tendria que adquirir un lock
         * para protejer el manejo de listas. En linux
         * usa un interrupt safe spinlock
         * Lo que seria bueno es lockear el acceso cuando es absolutamente
         * necesario, esto nos permitiria que se puedan satisfaces
         * multiples alocaciones al mismo tiempo.
         */

        if (order >= MAX_ORDER)
		return NULL;

	uint32_t current_order = order;

	buddy_list_t *free_list = &buddy.free_lists[order];

	struct list_head *head;
	struct list_head *curr;

	page_t *page;

	do
	{
		/* Lista de bloques de paginas libres del orden actual */
		head = &free_list->head;

		/* el primer bloque de paginas libre */
		curr = head->next;

		/* Si la lista no esta vacia, quiere decir que hay paginas para sacar */
		if (!list_empty(head))
		{
			/* Indice de la pagina dentro del mem_map */
			unsigned int index;

			/* Obtenemos la estructura page (head no es la variable, es el miembro) */
			page = list_entry(curr, page_t, head);

			/* Eliminamos el bloque actual de paginas de la lista */
			list_del(curr);

			/* Calculamos el indice en el mem_map de la primer pagina del bloque */
			index = page - mem_map;

			#define MARK_USED(index, order, free_list) \
				bit_complement((index) >> (1+(order)), (free_list)->page_bitmap)

			/* Si current order es MAX_ORDER no existe un buddy */
			if (current_order != MAX_ORDER-1)
				MARK_USED(index, current_order, free_list);

			/* Tamano del bloque actual */
			uint32_t size = 1 << current_order;

			/*
			 * Si current_order > order, quiere decir que no habia un bloque
			 * de justo tamano 2**order por lo que se busco un order mayor
			 * para dividirlo.
			 */
                        while (current_order > order)
			{
				/* Obtenemos la free list de un orden anterior */
                                free_list--;

				/* Decrementamos el current order */
                                current_order--;

				/* Calculamos el tamano del bloque de 2**current_order */
				size >>= 1;

				/* Agregamos al buddy1 a la lista de bloques libres */
				list_add(&(page)->head, &free_list->head);

				/*
				 * Aca marcamos que uno de los dos buddies
				 * esta siendo usado, lo que no quiere decir
				 * cual de los dos.
				 */
                                MARK_USED(index, current_order, free_list);

				/* Seguimos con la siguiente */
                                index += size;

				/*
				 * Siguiente pagina, size es en realidad  la
				 * cantidad de paginas que vamos a saltear, es
				 * decir el el tamano del buddy1
				 */
                                page += size;
                        }

			/* current_order = order -> tenemos una pagina */
			//set_page_count(page, 1);

			return page;
		}

		current_order++;
		free_list++;
	} while (current_order < MAX_ORDER);

	return NULL;
}

/*
 * LAZY BUDDY ALGORITHM
 *
 * Unfortunately, this scheme requires coalescing at every opportunity
 * -- potentially every free(). This can get expensive -- it wastes a lot
 * of time. SYSVR4 improved this approach to include lazy coalescing.
 *
 * Under this approach, the system has three different modes per list:
 *
 *      - Lazy:
 *        Don't bother to coalesce -- we'll probably need
 *        the buffer again soon.
 *
 *      - Reclaiming:
 *        We've got a lot of extra buffers in this list,
 *        so let's try to coalesce this one.
 *
 *      - Accelerated:
 *        We've got way too many extra buffers in
 *        this list, so let's try to coalesce this one,
 *        plus another one.
 *
 * It makes the decision about whcih mode to be in based on the amount
 * of slack in the list -- the number of extra buffers.
 *
 * Slack is defined as follows:
 * slack = (number of buffers in the list) - 2*(number of buffers cached
 * in the free list) - (the number of marked free in the bit map)
 *
 * Basically, in lazy mode, the system assumes that it will reuse the buffer,
 * so it doesn't mark it free in the global array. Whereas it does in the
 * other two modes.
 *
 * If the slack parameter is 2 or more, the system is in lazy mode.
 * If it equals 1, the system is in reclaim mode.
 * If it is 0, it is in accelerated mode.
 */


/*
 * TODO:
 *  - Agregar spinlocks
 *  - Implementar el Lazy Buddy Allocator
 *  - Remover el argumento order, no tiene sentido por que el order
 *    lo tenemos dentro de la estructura page
 *  - Remover los bitmaps del buddy por que tampoco tiene sentido ya que
 *    podemos calcular la direccion y el estado del buddy usando los datos
 *    de la struct page
 */
void
buddy_page_free(page_t *page, uint32_t order)
{
	/* La pagina no esta mas en uso asi que sacamos los bits de dirty y referencia */
	//page->flags &= ~((1<<PG_referenced) | (1<<PG_dirty));

	uint32_t index;

	/* mascara con 2**order 0's en el final */
	uint32_t mask = (~0UL) << order;

	/* Sacamos el indice restandole la base */
	uint32_t page_idx = page - mem_map;

	/* Nos fijamos si el indice esta alineado */
	if (page_idx & ~mask)
		BUG();

	/* Indice dentro del bitmap, es como dividir por 2 */
	index = page_idx >> (1 + order);

	/* Obtenemos la free_list[order] */
	buddy_list_t *free_list = &buddy.free_lists[order];

	/* -mask = # de paginas a liberar .: zone->free_pages += -mask; */
	/*zone->free_pages -= mask;*/

	/** Loopeamos hasta llegar al MAX_ORDER */
	while (mask + (1 << (MAX_ORDER-1)))
	{
		struct page *buddy1, *buddy2;

		/*
		 * Cuando map esta en 0, los dos buddyes estan en uso o libres.
		 * Cuando map esta en 1, alguno de los dos esta libre. Como estamos
		 * liberando uno si el map esta en 1 quiere decir que los dos
		 * estan libres y los podemos juntar
		 */

		/* Devuelve el bit actual y lo modifica */
		if (!bit_test_complement(index, free_list->page_bitmap))
			break;

		/* buddy del bloque actual ; -mask = 1+~mask */
		buddy1 = mem_map + (page_idx ^ -mask);

		/*
		 * buddy2 = bloque actual, lo que podriamos hacer es
		 * usar algun tipo de flag que nos diga el estado del buddy
		 * (free, inuse, etc). Si esta free, los juntamos, sino seguimos
		 * Esto nos permite deshacernos del bitmap
		 */
		buddy2 = mem_map + page_idx;

		/* Ahora sacamos el buddy1 de la lista enlazada */

		// Uhm me parece que el buddy1 en realidad no es el bloque que
		// estamos dealocando sino el buddy de ese bloque. Eso si tendria sendido
		// por que sino estariamos dejando el buddy2 dentro de una lista.
		list_del(&buddy1->head);

		/** Ahora seguimos con el siguiente orden */
		mask <<= 1;

		/** area es el puntero a la freelist de orden order */
		free_list++;

		/** index en el mapa de un orden mas arriba */
		index >>= 1;

		/** page index en el mem_map para el buddy a mergear */
		page_idx &= mask;
	}

	/** Ahora agregamos el buddy a la lista de frees correspondiente */
	list_add(&(mem_map + page_idx)->head, &free_list->head);

	return;
}

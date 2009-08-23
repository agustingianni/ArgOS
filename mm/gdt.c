#include <types.h>
#include <mm/mm.h>
#include <arch/irq.h>
#include <lib/string.h>
#include <arch/tss.h>
#include <kernel/printk.h>

/*
 * Variables globales que van a tener los
 * valores por defecto de la gdt y un puntero
 * que describe la gdt
 *
 * en teoria cada procesador tiene que tener su
 * gdt pero con los contenidos iguales salvo
 * la TSS que es propia de cada procesador
 */

/*
 * The base addresses of the GDT
 * should be aligned on an eight-byte boundary to
 * yield the best processor performance.
 * The limit value for the GDT is expressed in bytes.
 */

/* The Global Descriptor Table (GDT) */
static struct gdt_entry gdt[GDT_ENTRIES];

/* This has the base address of the gdt and its size */
static struct region_descriptor gdtp;

/* The actual number of GDT entries */
static unsigned int gdt_entry_count = 0;

/*
 * The regions are used by set_ldt, set_idt and
 * set_gdt
 */
void
set_region(struct region_descriptor *rd,
	void *base, size_t limit)
{
	rd->rd_limit 	= (int) limit;
	rd->rd_base 	= (int) base;
}

/*
 * Deallocate a new gdt entry
 */
void
gdt_deallocate_entry(struct gdt_entry *descriptor)
{
	int iflags = begin_atomic();
	{
		descriptor->sd_avl = 1;
		gdt_entry_count--;
	}
	end_atomic(iflags);
}

/*
 * Create a new gdt entry
 */
struct gdt_entry
*gdt_allocate_entry(void)
{
	/* The first entry of the GDT must be NULL */
	int i = 1, iflags;
	struct gdt_entry *descriptor = NULL;

	/* TODO: Maybe we need to do this atomically */
	iflags = begin_atomic();
	{
		while (i < GDT_ENTRIES)
		{
			descriptor = &gdt[i];
			if(descriptor->sd_avl == 1)
			{
				descriptor->sd_avl = 0;
				gdt_entry_count++;
				end_atomic(iflags);

				return descriptor;
			}
		}
	}
	end_atomic(iflags);

	return NULL;
}

/*
 * Fills a gdt_entry
 */
void
gdt_set_entry
(
	struct gdt_entry *sd,
	void *base,
	size_t limit,
	int type,
	int dpl,
	int def32,
	int gran
)
{
	sd->sd_lolimit 	= (limit & 0xFFFFL);
	sd->sd_hilimit 	= (limit >> 16) & 0x0FL;

	sd->sd_lobase 	= (((int) base) & 0xFFFFL);
	sd->sd_midbase  = (((int) base) >> 16) & 0xFFL;
	sd->sd_hibase 	= (((int) base) >> 24) & 0xFFL;

	sd->sd_type 	= type;

	sd->sd_dpl 		= dpl;
	sd->sd_p 		= SD_PRESENT;

	/* unused values */
	sd->sd_avl 		= 0;
	sd->sd_l 		= 0;

	sd->sd_def32 	= def32;
	sd->sd_gran 	= gran;
}

/*
 * Setea los valores por defecto de la gdt
 */
void
init_gdt(void)
{
	int iflags = begin_atomic();
	{
 		/* zero out the array */
		memset((void *) gdt, 0x00, sizeof(gdt[0])*GDT_ENTRIES);

		/* prepare the segment descriptor that will be loaded into GDTR */
		set_region
		(
			&gdtp,
			&gdt,
			(sizeof(gdt[0]) * GDT_ENTRIES) - 1
		);

		/* create NULL descriptor */
    	gdt_set_entry//#0
    	(
    		&gdt[GDT_NULL_SEL],
    		0,			/* base 		*/
    		0,			/* limit 		*/
    		SDT_SYSNULL,		/* type 		*/
    		0,			/* dpl 			*/
    		0,			/* def32 		*/
    		0			/* granularity 	*/
    	);

		/* create a kernel code segment descriptor */
    	gdt_set_entry//#1
    	(
    		&gdt[GDT_KCODE_SEL],
    		SD_LIMIT_LOWER,			/* 0x00000000 */
    		SD_LIMIT_UPPER,			/* 0xffffffff */
    		SDT_MEMER,				/* exec + read */
    		SD_DPL_0,				/* Kernel Mode DPL */
    		SD_OPSIZE_32,			/* 32 bit operand */
    		SD_GRAN_4K				/* 4 kbyte  granularity */
    	);

		/* create a kernel data segment descriptor */
    	gdt_set_entry//#2
    	(
    		&gdt[GDT_KDATA_SEL],	/* */
    		SD_LIMIT_LOWER,			/* 0x00000000 */
    		SD_LIMIT_UPPER,			/* 0xffffffff */
    		SDT_MEMRW,				/* read + write */
    		SD_DPL_0,				/* Kernel Mode DPL */
    		SD_OPSIZE_32,			/* 32 bit operand */
    		SD_GRAN_4K				/* 4 kbyte  granularity */
    	);

		/* create a user code segment descriptor */
    	gdt_set_entry//#3
    	(
    		&gdt[GDT_UCODE_SEL],	/* */
    		SD_LIMIT_LOWER,			/* 0x00000000 */
    		SD_LIMIT_UPPER,			/* 0xffffffff */
    		SDT_MEMER,				/* exec + read */
    		SD_DPL_3,				/* User Mode DPL */
    		SD_OPSIZE_32,			/* 32 bit operand */
    		SD_GRAN_4K				/* 4 kbyte  granularity */
    	);

		/* create a user data segment descriptor */
    	gdt_set_entry//#4
    	(
    		&gdt[GDT_UDATA_SEL],	/* */
    		SD_LIMIT_LOWER,			/* 0x00000000 */
    		SD_LIMIT_UPPER,			/* 0xffffffff */
    		SDT_MEMRW,				/* read + write */
    		SD_DPL_3,				/* User Mode DPL */
    		SD_OPSIZE_32,			/* 32 bit operand */
    		SD_GRAN_4K				/* 4 kbyte  granularity */
    	);

        /*
         * Esta entrada en la GDT es usada cuando hay un cambio
         * de privilegios de ring3 a ring0. El procesador detecta
         * esta situacion, lee el registro TR, de ahi obtiene un
         * indice que le permite ubicar este segmento en la GDT
         * que tiene informacion sobre SS:ESP de RING0
         */
        #define TSS_ARRAY_SIZE 1
    	extern tss_t tss_array[TSS_ARRAY_SIZE];
        gdt_set_entry//#5
    	(
    		&gdt[GDT_CPU0TSS_SEL],
    		&tss_array[0],    	         /* init addr */
    		TSS_DEFAULT_SIZE,        /* Offset */
    		SDT_SYS386BSY,	         /* i386 TSS */
    		SD_DPL_0,		         /* Kernel Mode DPL */
    		SD_OPSIZE_32,            /* 32 bit operand */
    		SD_GRAN_1B		         /* 4 kbyte  granularity */
    	);

    	/* load the new selector values by callint "lgdt" */
    	gdt_flush(&gdtp);
    	gdt_entry_count += 6;
	}
	end_atomic(iflags);

	return;
}

/*Para conservar un poco el encapsulamiento*/
struct gdt_entry *
gdt_get_entry(unsigned short index)
{
    return (&(gdt[index]));
}

void
gdt_dump(void)
{

}

void
gdt_dump_entry(struct gdt_entry *e)
{
}

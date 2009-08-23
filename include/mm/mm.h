 #ifndef MM_H_
 #define MM_H_

 #include <types.h>

 /*
  * The number of default entries in the GDT
  *
  * TODO: Linux pone 32, para dejar todo alineado
  * en multiplos de 256 bytes
  */
 /*Una mas para probar un TSS*/
 #define GDT_ENTRIES 6

 /* Entries in the Global Descriptor Table (GDT) */
 #define GDT_NULL_SEL	0	/* default null descriptor 	*/
 #define GDT_KCODE_SEL	1	/* ring0 code descriptor   	*/
 #define GDT_KDATA_SEL	2	/* ring0 data descriptor   	*/
 #define GDT_UCODE_SEL	3	/* ring3 code descriptor   	*/
 #define GDT_UDATA_SEL	4	/* ring3 data descriptor   	*/
 #define GDT_CPU0TSS_SEL	5	/* task state segment desc  (one per processor) */
 #define GDT_LDT_SEL	6	/* local descriptor table   */

 #define USER_CS		(GDT_UCODE_SEL * 8)
 #define USER_DS		(GDT_UDATA_SEL * 8)
 #define KERNEL_CS		(GDT_KCODE_SEL * 8)
 #define KERNEL_DS		(GDT_KDATA_SEL * 8)
 #define CPU0_TSS       (GDT_CPU0TSS_SEL   * 8)

 /*
  * TODO:
  * 	Definir la estructura tss_entry y
  * 	tss_descriptor o algo asi
  *
  * 	TLS (Thread local storage) ver que es
  * 	y tratar de implementar algo
  */

 /*
  * this structure is used by the asm
  * instruction lgdt (struct region_descriptor)
  * and also for lidt
  */
 struct region_descriptor
 {
	 unsigned rd_limit:16;		/* segment extent 	*/
     unsigned rd_base:32;		/* base address   	*/
 } __attribute__((packed));

 /*
  * utility struct that helps us to use the
  * information inside the cpu selectors
  */
struct segment_selector
{
    unsigned ss_privilege:2; 	/* request privilege level */
    unsigned ss_table:1;	/* table indicator 0 = GDT ; 1 = LDT */
    unsigned ss_index:13;	/* table index */
} __attribute__((packed));

 /* Segment selector table indicator */
 #define SS_TABLE_GDT 0
 #define SS_TABLE_LDT 1

 /* Segment selector request privilege level */
 #define SS_RPL_0     0
 #define SS_RPL_1     1
 #define SS_RPL_2     2
 #define SS_RPL_3     3

 /*
  * From Intel System developer Manual:
  * 3.4.5 Segment Descriptors
  * A segment descriptor is a data structure in a GDT or
  * LDT that provides the processor
  * with the size and location of a segment, as well as access
  * control and status information.
  * Segment descriptors are typically created by compilers,
  * linkers, loaders, or the operating system or executive,
  * but not application programs.
  */
 struct gdt_entry
 {
 	 /*
 	  * segment limit = 20 bits
 	  * base address  = 32 bits
	  */

	 unsigned sd_lolimit:16;	/* segment extent		 				*/
	 unsigned sd_lobase:16;		/* segment base address		 			*/
	 unsigned sd_midbase:8;		/* segment base address		 			*/
	 unsigned sd_type:5;		/* segment type	+ s field				*/
	 /*unsigned sd_s:1;			 desc type 0 = sys; 1 = code/data 		*/
	 unsigned sd_dpl:2;			/* segment descriptor priority level 	*/
	 unsigned sd_p:1;			/* segment descriptor present 			*/
	 unsigned sd_hilimit:4;		/* segment extent (msb) 				*/
	 unsigned sd_avl:1;			/* unused 								*/
	 unsigned sd_l:1;			/* unused 								*/
	 unsigned sd_def32:1;		/* default 32 vs 16 bit size 			*/
	 unsigned sd_gran:1;		/* limit granularity (byte/page) 		*/
	 unsigned sd_hibase:8;		/* segment base address (msb) 			*/
 } __attribute__((packed));

 #define SD_LIMIT_LOWER  0L
 #define SD_LIMIT_UPPER  0xFFFFFFFFL

 /* segment is present flags */
 #define SD_ABSENT       0
 #define SD_PRESENT      1

 /* segment descriptor priority levels */
 #define SD_DPL_0 		 0
 #define SD_DPL_1 		 1
 #define SD_DPL_2 		 2
 #define SD_DPL_3 		 3

 /* segment descriptor granularity */
 #define SD_GRAN_1B		 0
 #define SD_GRAN_4K 	 1

 /* operand size */
 #define SD_OPSIZE_16 	 0
 #define SD_OPSIZE_32 	 1

 /*
  * S = 0; system segments and gate types
  * S flag is not set (descriptor type = system)
  */
 #define SDT_SYSNULL	0x00	/* system null */
 #define SDT_SYS286TSS	0x01	/* system 286 TSS available */
 #define SDT_SYSLDT		0x02	/* system local descriptor table */
 #define SDT_SYS286BSY	0x03	/* system 286 TSS busy */
 #define SDT_SYS286CGT	0x04	/* system 286 call gate */

 /* used on idt */
 #define SDT_SYSTASKGT	0x05	/* system task gate */

 #define SDT_SYS286IGT	0x06	/* system 286 interrupt gate */
 #define SDT_SYS286TGT	0x07	/* system 286 trap gate */
 #define SDT_SYSNULL2	0x08	/* system null again */
 #define SDT_SYS386TSS	0x09	/* system 386 TSS available */
 #define SDT_SYSNULL3	0x0a	/* system null again */
 #define SDT_SYS386BSY	0x0b	/* system 386 TSS busy */
 #define SDT_SYS386CGT	0x0c	/* system 386 call gate */
 #define SDT_SYSNULL4	0x0d	/* system null again */

 /* used on idt */
 #define SDT_SYS386IGT	0x0e	/* system 386 interrupt gate */
 #define SDT_SYS386TGT	0x0f	/* system 386 trap gate */

 /* S = 1; data segment types */
 #define SDT_MEMRO		16	/* memory read only */
 #define SDT_MEMROA		17	/* memory read only accessed */
 #define SDT_MEMRW		18	/* memory read write */
 #define SDT_MEMRWA		19	/* memory read write accessed */
 #define SDT_MEMROD		20	/* memory read only expand dwn limit */
 #define SDT_MEMRODA	21	/* memory read only expand dwn limit accessed */
 #define SDT_MEMRWD		22	/* memory read write expand dwn limit */
 #define SDT_MEMRWDA	23	/* memory read write expand dwn limit acessed */

 /* S = 1; code segment types */
 #define SDT_MEME		24	/* memory execute only */
 #define SDT_MEMEA		25	/* memory execute only accessed */
 #define SDT_MEMER		26	/* memory execute read */
 #define SDT_MEMERA		27	/* memory execute read accessed */
 #define SDT_MEMEC		28	/* memory execute only conforming */
 #define SDT_MEMEAC		29	/* memory execute only accessed conforming */
 #define SDT_MEMERC		30	/* memory execute read conforming */
 #define SDT_MEMERAC	31	/* memory execute read accessed conforming */

 /*
  * fills a gdt_entry entry
  */
 void gdt_set_entry
 (
	 struct gdt_entry *sd,
 	 void *base,
	 size_t limit,
	 int type,
	 int dpl,
	 int def32,
	 int gran
 );

 void set_region(struct region_descriptor *, void *, size_t);

 /*
  * initializes the gdt, with the default values
  */
 void init_gdt(void);

 /*
  * asm routine that loads the new GDT also it
  * flushes the segment selectors
  */
 extern void gdt_flush(struct region_descriptor *);

 extern struct gdt_entry *get_gdt_entry(unsigned short index);

 /* TODO:
  * hay que implementar estas rutinas
  * para ordenar el acceso a la GDT
  */
 #if 0
 static inline void gdt_lock(void);
 static inline void gdt_unlock(void);
 #endif

 #endif /*MM_H_*/

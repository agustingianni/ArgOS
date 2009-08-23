#ifndef MULTIBOOT_H_
#define MULTIBOOT_H_

#include <types.h>
#include <kernel/kernel.h>

#define MULTIBOOT_HEADER_MAGIC			0x1BADB002
#define MULTIBOOT_HEADER_MODS_ALIGNED	0x00000001
#define MULTIBOOT_HEADER_WANT_MEMORY	0x00000002
#define MULTIBOOT_HEADER_HAS_VBE		0x00000004
#define MULTIBOOT_HEADER_HAS_ADDR		0x00010000

/* The magic number passed by a Multiboot-compliant boot loader. */
#define MULTIBOOT_INFO_MAGIC			0x2BADB002

/* Multiboot FLAGS */
#define MULTIBOOT_INFO_HAS_MEMORY		0
#define MULTIBOOT_INFO_HAS_BOOT_DEVICE	1
#define MULTIBOOT_INFO_HAS_CMDLINE		2
#define MULTIBOOT_INFO_HAS_MODS			3
#define MULTIBOOT_INFO_HAS_AOUT_SYMS	4
#define MULTIBOOT_INFO_HAS_ELF_SYMS		5
#define MULTIBOOT_INFO_HAS_MMAP			6
#define MULTIBOOT_INFO_HAS_DRIVES		7
#define MULTIBOOT_INFO_HAS_CONFIG_TABLE	8
#define MULTIBOOT_INFO_HAS_LOADER_NAME	9
#define MULTIBOOT_INFO_HAS_APM_TABLE	10
#define MULTIBOOT_INFO_HAS_VBE			11

#define CHECK_BIT(flags, bit) ((flags) & (1 << (bit)))
#define CHECK_MASK(flags, mask) (flags & mask)

struct multiboot_header
{
	/* required */
	uint32_t mh_magic;
	uint32_t mh_flags;
	uint32_t mh_checksum;

	/* required if mh_flags sets MULTIBOOT_HEADER_HAS_ADDR. */
	uint32_t mh_header_addr; 	/* addr of mb header */
	uint32_t mh_load_addr; 		/* address of the text segment */
	uint32_t mh_load_end_addr; 	/* end of the data segment */
	uint32_t mh_bss_end_addr; 	/* end of the bss segment */
	uint32_t mh_entry_addr; 	/* kernel entry point */

	/* required if mh_flags sets MULTIBOOT_HEADER_HAS_VBE. */
	uint32_t mh_mode_type;		/* 0 = linear ; 1 = EGA */
	uint32_t mh_width;			/* number of columns */
	uint32_t mh_height;			/* number of rows */
	uint32_t mh_depth;			/* number of bits per pixels */
};

/* The symbol table for a.out. */
struct multiboot_aout_symtab
{
	uint32_t ma_tabsize;
	uint32_t ma_strsize;
	uint32_t ma_addr;
	uint32_t ma_reserved;
};
     
 /* The section header table for ELF. */
struct multiboot_elf_sh_table
{
	uint32_t me_num;
	uint32_t me_size;
	uint32_t me_addr;
	uint32_t me_shndx;
};

/*
 * Upon entry to the operating system, the EBX register 
 * contains the physical address of a Multiboot information 
 * data structure, through which the boot loader 
 * communicates vital information to the operating system.
 */
typedef struct multiboot_information
{
	uint32_t	mi_flags;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_MEMORY. */
	uint32_t	mi_mem_lower;
	uint32_t	mi_mem_upper;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_BOOT_DEVICE. */
	uint8_t		mi_boot_device_part3; /*  */
	uint8_t		mi_boot_device_part2; /* sub-partition */
	uint8_t		mi_boot_device_part1; /* top-level partition number, */
	uint8_t		mi_boot_device_drive; /* bios drive number */

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_CMDLINE. */
	char *		mi_cmdline;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_MODS. */
	uint32_t	mi_mods_count; /* the number of modules loaded */
	vaddr_t		mi_mods_addr;  /* the physical address of the first module structure */

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_{AOUT,ELF}_SYMS. */
	union
    {
    	struct multiboot_aout_symtab  aout_sym;
        struct multiboot_elf_sh_table elf_sec;
    }u;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_MMAP. */
	uint32_t	mi_mmap_length;
	vaddr_t		mi_mmap_addr;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_DRIVES. */
	uint32_t	mi_drives_length;
	vaddr_t		mi_drives_addr;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_CONFIG_TABLE.  */
	/* indicates the address of the rom configuration table     *
	 * returned by the GET CONFIGURATION bios call. If the bios *
	 * call fails, then the size of the table must be zero.     */
	void *		unused_mi_config_table;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_LOADER_NAME. */
	char *		mi_loader_name;		/* ASCIIZ name of loader */

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_APM. */
	void *		mi_apm_table;		/* Address of an multiboot_apm struct */

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_VBE. */
	void *		mi_vbe_control_info;
	void *		mi_vbe_mode_info;
	paddr_t		mi_vbe_interface_seg;
	paddr_t		mi_vbe_interface_off;
	uint32_t	mi_vbe_interface_len;
} mbootinfo_t;

/* The module structure. */
struct multiboot_module
{
	uint32_t    mm_mod_start;	/* start address of module */
	uint32_t    mm_mod_end;		/* end address of module */
	char *	    mm_string;		/* ASCIIZ with the name*/
	uint32_t	mm_reserved;	/* 0 */
};

/*
 * Apm information. 
 * @see: http://www.microsoft.com/hwdev/busbios/amp_12.htm
 */

struct multiboot_apm
{
	uint16_t ma_version;		/* version */
	uint16_t ma_cseg;			/* protected mode 32-bit code segment */
	uint32_t ma_offset;			/* offset of the entry point */
	uint16_t ma_cseg_16;		/* protected mode 16-bit code segment */
	uint16_t ma_dseg;			/* protected mode 16-bit data segment */
	uint16_t ma_flags;			/* flags */
	uint16_t ma_cseg_len;		/* length of the protected mode 32-bit code segment */
	uint16_t ma_cseg_16_len;	/* length of the protected mode 16-bit code segment */
	uint16_t ma_dseg_len;		/* length of the protected mode 16-bit data segment */
};

/*
 * Drive information.  This describes an entry in the drives table as
 * pointed to by mi_drives_addr.
 */
struct multiboot_drive
{
	uint32_t	md_size;		/* the size of the multiboot_drive structure*/
	uint8_t		md_number;		/* BIOS drive number */
	uint8_t		md_mode;		/* 0 = CHS ; 1 = LBA */
	uint16_t	md_cylinders;	/* disk geometry detected by BIOS */
	uint8_t		md_heads;		/* same */
	uint8_t		md_sectors;		/* same */

	/* The variable-sized 'ports' field comes here, so this structure
	 * can be longer. */
};

/*
 * Memory mapping.  This describes an entry in the memory mappings table
 * as pointed to by mi_mmap_addr.
 *
 * Be aware that mm_size specifies the size of all other fields *except*
 * for mm_size.  In order to jump between two different entries, you
 * have to count mm_size + 4 bytes.
 */
struct multiboot_mmap
{
	uint32_t	mm_size;
	/* uint64_t	mm_base_addr; 	*/
	/* uint64_t	mm_length; 		*/
	uint32_t 	mm_base_addr_low;
	uint32_t 	mm_base_addr_high;
	uint32_t 	mm_length_low;
	uint32_t 	mm_length_high;
	
	/* Ram types indicated by multiboot loader */
	#define	AVL_RAM			1	/* available RAM usable by OS */
	#define	RESERVED_RAM	2	/* in use or reserved by the system */
	#define	ACPI_RAM		3	/* ACPI Reclaim memory */
	#define	NVS_RAM			4	/* ACPI NVS memory */

	uint32_t	mm_type;
};


int init_boot_information
(
 	uint32_t magic,
 	mbootinfo_t *mbi,
	sysinfo_t *sys_info
);

#endif /*MULTIBOOT_H_*/

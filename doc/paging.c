Paging:

	- There will be a single page table for all kernel only 
	  threads, and a page table for each user process

	- Flags:
		pg flag:
		bit 31 of cr0 (i386)
		enables general paging (also protection)

		pse flag:
		bit 4 of cr4 (pentium)
		enables large pages (4mb)

		pae flag:
		bit 5 of cr4 (pentium pro)
		physical address extension (36 bit addressing)

		

	The information that the processor uses to translate linear 
	addresses into physical addresses is contained in
	four data structures:
	
	Page directory (register cr3 points to the current page directory)
	- An array of 32-bit page-directory entries (PDEs) contained in
	a 4-KByte page. Up to 1024 page-directory entries can be 
	held in a page directory.
	
	Page table 
	- An array of 32-bit page-table entries (PTEs) contained in a
	4-KByte page. Up to 1024 page-table entries can be held in a page table. (Page
	tables are not used for 2-MByte or 4-MByte pages. These page sizes are mapped
	directly from one or more page-directory entries.)

	Page
	- A 4-KByte, 2-MByte, or 4-MByte flat address space.

	Page-Directory-Pointer Table 
	- An array of four 64-bit entries, each of which
	points to a page directory. This data structure is only used when the physical
	address extension is enabled


	DIRECTORY + TABLE + OFFSET = PHYSICAL ADDRESS

	/* NOTE: Performance */
	A typical example of mixing 4-KByte and 4-MByte pages is 
	to place the operating system or executive's kernel in a
	large page to reduce TLB misses and thus improve
	overall system performance.
	
	The processor maintains 4-MByte page entries and 4-KByte page 
	entries in separate TLBs. So, placing often used code such as 
	the kernel in a large page, frees up
	4-KByte-page TLB entries for application programs and tasks.


Protection:

	Fields used by the protection mechanisms:

	    Segments:

		- Descriptor type (S) flag
		- Type field (Type)
		- Limit field
		- Granularity field (G)
		- DPL Field (segment descriptor)
		- RPL Field (segment selector)
		- CPL Field (segment selector)
	
	    Pages:

		- U/S Flag
		- R/W Flag


	Privilege Levels:

		There are 4, ring0, ring1, ring2 and ring3

		CPL
		{
			It is the privilege of the current running program or task
			Stored in the bits 0 and 1 of CS and SS selectors.

			CPL == Code segment PL
		}

		DPL
		{
			Is the privilege of a segment or gate.
			Stored in the DPL field of the segment descriptor.
			cpm dpl, cpl
		}

		RPL
		{
			It is checked againts the CPL to ensure 
			we have enough privileges to access the segment.
		}
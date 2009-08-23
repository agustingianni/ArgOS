ENTRY(start)

SECTIONS
{
  /*
   * GRUB reads this information from the ELF headers
   * and loads the kernel at the correct address,
   * the address must be >= 1MB and < Physical RAM
   */
   
  K_PHYS_START = 0x100000;
  K_VIR_START  = 0xc0000000;
  K_STACK_SIZE = 0x1000;
  PAGE_SIZE    = 0x1000;

  . = K_PHYS_START;
  .setup :
  {
    *(.setup)
  }
  . += K_VIR_START;
  
  .text : AT(ADDR(.text) - K_VIR_START)
  {
    KERN_TEXT_BEGIN = .;
    *(.text)
    . = ALIGN(PAGE_SIZE);
    KERN_TEXT_END = .;
  }
  
  .data : AT(ADDR(.data) - K_VIR_START)
  {
    KERN_DATA_BEGIN = .;
    *(.data)
    *(.rodata)
    . = ALIGN(PAGE_SIZE);
    KERN_DATA_END = .;
  }
  
  .bss : AT(ADDR(.bss) - K_VIR_START)
  {
    KERN_BSS_BEGIN = .;
    *(.common)
    *(.bss)
    . = ALIGN(PAGE_SIZE);
    KERN_BSS_END = .;
  }
  
  . = ALIGN(PAGE_SIZE);
  KERN_TOP = .;
}

#include <boot/multiboot.h>
#include <assert.h>
#include <mm/mm.h>
#include <arch/cpuid.h>
#include <arch/idt.h>
#include <arch/irq.h>
#include <arch/paging.h>
#include <arch/apic.h>
#include <arch/timer.h>
#include <arch/tss.h>
#include <types.h>
#include <stddef.h>
#include <kernel/console.h>
#include <kernel/printk.h>
#include <kernel/kernel.h>
#include <kernel/scheduler.h>
#include <kernel/bootmm.h>
#include <kernel/slab.h>
#include <jiffies.h>
#include <vm/buddy_allocator.h>
#include <debug.h>
#include <arch/bitops.h>

const char *banner = "    _               ___  ____   __     __   ___   _\n"
    "   / \\   _ __ __ _ / _ \\/ ___|  \\ \\   / /  / _ \\ / |\n"
    "  / _ \\ | '__/ _` | | | \\___ \\   \\ \\ / /  | | | || |\n"
    " / ___ \\| | | (_| | |_| |___) |   \\ V /   | |_| || |\n"
    "/_/   \\_\\_|  \\__, |\\___/|____/     \\_/     \\___(_)_|\n"
    "             |___/                                  \n";

extern void init_gdt(void);
extern void print_cpu_information(cpuinfo_t *c);
extern void init_cpu(cpuinfo_t *c);
extern void exceptions_install(void);
extern void irq_install(void);
extern void init_video(void);
extern void init_keyboard(void);
extern void init_disks(void);
extern void init_vm(sysinfo_t *sys_info);

cpuinfo_t cpu_info;
sysinfo_t sys_info;

void init_bss(void)
{
    /*
     * Estas dos variables son exportadas por el linker
     * en el archivo kernel/main.ld
     *
     * ACA
     */
    extern paddr_t KERN_BSS_BEGIN, KERN_BSS_END;
    memset(&KERN_BSS_BEGIN, 0xcc, &KERN_BSS_END - &KERN_BSS_BEGIN);
}

/*
 * Called from entry.s::start(void)
 * This is the fase 2 of the kernel startup
 *
 * Now we check the information passed by
 * grub to the kernel and also load all the
 * needed stuff like GDT, IDT, etc.
 *
 */
void kernel_main(uint32_t magic, mbootinfo_t *mb_info)
{
    /* We do not want any interrups disturbing the initialization of the kernel */
    if (irq_enabled())
        irq_disable();

    /* The .bss section must be zeroed */
    init_bss();

    /* inicializamos la memoria de video */
    init_video();

    /* inicializamos la IDT */
    init_idt();

    /* we we relocate the kernel to 0xc0000000 */
    init_temp_paging();

    /* and here we replace the 'trick' gdt */
    init_gdt();

    screen_clear();

    /* Parse informacion passed by GRUB or multiboot capable bootmanager */
    init_boot_information(magic, phys_to_virt((paddr_t) mb_info), &sys_info);

    /* Get information about the current CPU's */
    init_cpu(&cpu_info);

    /* initialize boot memory allocator */
    init_bootmm(&sys_info);

    /*
     * Once we have initialized the boot memory allocator
     * we can procede with the rest of paging initialization
     */
    init_paging();

    init_mem_map();

    /* Page allocator */
    init_buddy();

    /* Give back space used by the boot mem allocator */
    bootmem_retire();

    /* From here now, we are using the SLAB Allocator, so we can use kmalloc :) */
    init_slab();

    /* We initialize the PIT and other time related stuff */
    init_timer();

    /* We want keboard access */
    init_keyboard();

    /* Aca empieza toda la tarea de inicializar el scheduler. */
    //init_scheduler();


    /* Now it's safe to receive interrups */
    irq_enable();

    return;
}

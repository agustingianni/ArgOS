#include <arch/cpuid.h>
#include <arch/asm.h>
#include <arch/irq.h>
#include <lib/string.h>
#include <kernel/console.h>
#include <kernel/printk.h>
#include <jiffies.h>
#include <arch/timer.h>

/*
 * Print all the information contained in the cpuinfo_t
 * structure.
 */
void _print_cpu_information(cpuinfo_t *c)
{
    printk("\nGeneric information:\n"
        "-------------------\n"
        "Vendor ID:    %s\n"
        "Brand string: %s\n"
        "Stepping ID:  %d\n"
        "Model:        %d\n"
        "Family:       %d\n"
        "Type:         %d\n", c->vendor_id, c->brand_string,
            c->signature.stepping_id,
            (c->signature.extended_model << 4) +c->signature.model_num,
            c->signature.family_code + c->signature.extended_family,
            c->signature.processor_type
            );

            printk
            (
                    "Version information:\n"
                    "-------------------\n"
                    "Brand:                 %d\n"
                    "Cache flush line size: %d\n"
                    "Processors:            %d\n"
                    "Initial APIC id:       %d\n",
                    c->version.brand_index,
                    c->version.cflush_line_size,
                    c->version.number_of_processors,
                    c->version.initial_apic_id
            );

            printk
            (
                    "Features information:\n"
                    "--------------------\n"
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | ",
                    c->features.sse3 ? "SSE3" : "!SSE3",
                    c->features.monitor ? "MON/WAIT" : "!MON/WAIT",
                    c->features.vmx ? "VMX" : "!VMX",
                    c->features.est ? "SPEED_STEP" : "!SPEED_STEP",
                    c->features.tm2 ? "TM2" : "!TM2",
                    c->features.ssse3 ? "SSSE3" : "!SSSE3"
            );

            printk
            (
                    "Features information:\n"
                    "--------------------\n"
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | "
                    "%s | ",
                    c->ext_features.fpu ? "FPU" : "!FPU",
                    c->ext_features.apic ? "APIC" : "!APIC",
                    c->ext_features.dbgext ? "DE" : "!DE",
                    c->ext_features.pse ? "PSE" : "!PSE",
                    c->ext_features.mmx ? "MMX" : "!MMX",
                    c->ext_features.sse ? "SSE" : "!SSE",
                    c->ext_features.sse2 ? "SSE2" : "!SSE2"
            );
        }

#include <drivers/rtc.h>

static int cpu_get_frequency(cpuinfo_t *c)
{
    uint64_t timestamp1, timestamp2;
    int iflags;

    /* Check if we have Time Stamp Counter */
    if (!c->ext_features.tsc)
    {
        c->frequency = 0;
        return -1;
    }

    iflags = begin_atomic();
    {
        // Wait until the second has precisely just started.
        sys_time(NULL);

        // Read the value of the current time-stamp counter (64-bit).
        timestamp1 = rdtsc();

        // Wait exactly one second.
        sys_time(NULL);

        // Read the value of the current time-stamp counter again.
        timestamp2 = rdtsc();

        // Now we can estimate the CPU frequency as follows.
        c->frequency = ((uint32_t) (timestamp2 - timestamp1) / 1000);
    }
    end_atomic(iflags);

    return 0;
}

/* initializes the structures related with the
 * cpu information */
void init_cpu(cpuinfo_t *c)
{
    unsigned int query, dummy, ecx, edx;

    /* First we check if we have CPUID instruction */
    if (!cpuid_check())
    {
        printk("[Error] CPUID instruction unnavailable\n");
        return;
    }

    memset((void *) c, 0x00, sizeof(cpuinfo_t));

    /*
     * Get the cpu vendor id string
     */
    cpuid(CPUID_QUERY_BASIC, &query, /* returns max value supported by cpuid instr */
    (unsigned int *) &(c->vendor_id[0]), (unsigned int *) &(c->vendor_id[8]),
            (unsigned int *) &(c->vendor_id[4]));

    /*
     * Get the Version and features
     */
    cpuid(CPUID_QUERY_VERSION, &query, /* this will contain more information */
    (unsigned int *) &(c->version), (unsigned int *) &(c->features),
            (unsigned int *) &(c->ext_features));

    /* Copy the values returned by cpuid instruction */
    *((unsigned int *) &(c->signature)) = query;

    if (cpu_get_frequency(c) == -1)
    {
        printk("Failed reading the CPU frequency\n");
    }

    /*
     * Check if the processor supports the brand string
     * query
     */
    cpuid(CPUID_QUERY_EXT_FUNCTION, &query, (unsigned int *) &dummy,
            (unsigned int *) &dummy, (unsigned int *) &dummy);

    /* We cannot get the brand string */
    if (query < CPUID_QUERY_EXT_PROC_BRAND2)
    {
        printk("processor brando not available");
        return;
    }

    /* Now get the processor brand string */
    unsigned int *t = (unsigned int *) c->brand_string;

    cpuid(CPUID_QUERY_EXT_PROC_BRAND0, &t[0], &t[1], &t[2], &t[3]);
    cpuid(CPUID_QUERY_EXT_PROC_BRAND1, &t[4], &t[5], &t[6], &t[7]);
    cpuid(CPUID_QUERY_EXT_PROC_BRAND2, &t[8], &t[9], &t[10], &t[11]);

    /* Null terminate the string */
    c->brand_string[48] = 0;

    /* Remove useless whitespaces */
    char *p, *q;
    p = q = &c->brand_string[0];

    while (*p == ' ')
        p++;

    if (p != q)
    {
        while (*p)
            *q++ = *p++;

        while (q <= &c->brand_string[48])
            *q++ = '\0'; /* Zero-pad the rest */
    }

    printk("CPU: %s\n", c->brand_string);

    if (query >= 0x80000005)
    {
        cpuid(0x80000005, &dummy, &dummy, &ecx, &edx);
        printk(
                "CPU: L1 I Cache: %dK (%d bytes/line), D cache %dK (%d bytes/line)\n",
                edx >> 24, edx & 0xFF, ecx >> 24, ecx & 0xFF);

        c->l1_cache_size = (ecx >> 24) + (edx >> 24);
    }

    if (query < 0x80000006)
        return;

    cpuid(0x80000006, &dummy, &dummy, &ecx, &dummy);
    c->l2_cache_size = ecx >> 16;

    printk("CPU: L2 Cache: %dK (%d bytes/line)\n", c->l2_cache_size, ecx & 0xFF);

    return;
}

void print_cpu_information(cpuinfo_t *id)
{
    static char *cputype[4] =
    { "0 (Original OEM processor)", "1 (OverDrive processor)",
            "2 (Dual processor)", "3 (Intel Reserved)" };

    /* struct cpuid_extended_features; */
    static const char *features[] =
    { "On-chip FPU", "Virtual 8086 mode enhancement", "I/O breakpoints",
            "4MB page size extensions", "Time stamp counter",
            "Model-specific registers", "36-bit physical address extension",
            "Machine check exception", "CMPXCHG8B instruction",
            "On-chip local APIC", NULL, "Fast system call (maybe)",
            "Memory type range registers", "Global bit in page table entries",
            "Machine check architecture", "CMOVcc instruction",
            "Page attribute table", "36-bit page size extension",
            "Processor Serial Number", NULL, NULL, NULL, NULL,
            "MMX instructions", "Fast FP save/restore", "SSE instructions",
            NULL, NULL, NULL, NULL, "IA-64 architecture", NULL, };

    const char *company = "";
    const char *modelname = "";
    char fam[4];
    const char *family = fam;
    char vendorname[13];
    unsigned i;

    /*
     * First try to guess a descriptive processor name
     * based on the vendor ID, family, and model information.
     * Much of this information comes from the x86 info file
     * compiled by Christian Ludloff, ludloff@anet-dfw.com,
     * http://webusers.anet-dfw.com/~ludloff.
     */

    /* Default family string: #86 */
    fam[0] = '0' + id->signature.family_code;
    fam[1] = '8';
    fam[2] = '6';
    fam[3] = 0;

    /* Check for specific vendors and models */
    if (memcmp(id->vendor_id, "GenuineIntel", 12) == 0)
    {
        static const char *m486[] =
        {
            "DX",
            "DX",
            "SX",
            "DX/2 or 487",
            "SL",
            "SX/2",
            "",
            "DX/2-WB",
            "DX/4"
        };

        static const char *mp5[] =
        {
            "",
            " 60/66", /* or overdrive for same */
            " 75/90/100/120/133/150/166/200",
            "",
            " MMX"
        };

        static const char *mp5o[] =
        {
            "",
            "", /* 60/66 Overdrive, in theory */
            "", /* 75-133 Overdrive, in theory */
            " Overdrive for 486",
            " MMX Overdrive"
        };

        static const char *mp6[] =
        {
            "",
            " Pro",
            "",
            " II",
            "",
            " II/Celeron/Xeon",
            " Celeron 300A/333",
            " III",
            " III Coppermine"
        };

        static const char *mp6o[] =
        {
            "",
            "",
            "",
            " Pro Overdrive"
        };

        company = "Intel ";
        switch (id->signature.family_code)
        {
            case 4:
            if (id->signature.model_num < sizeof(m486)/sizeof(m486[0]))
            {
                modelname = m486[id->signature.model_num];
            }

            break;
            case 5:
            family = "Pentium";

            if ((id->signature.model_num < sizeof(mp5)/sizeof(mp5[0])) &&
                    (id->signature.processor_type == 0))
            {
                modelname = mp5[id->signature.model_num];
            }

            if ((id->signature.model_num < sizeof(mp5o)/sizeof(mp5o[0])) &&
                    (id->signature.processor_type == 1))
            {
                modelname = mp5o[id->signature.model_num];
            }

            break;
            case 6:
            family = "Pentium (P6)";

            if ((id->signature.model_num < sizeof(mp6)/sizeof(mp6[0])) &&
                    (id->signature.processor_type == 0))
            {
                modelname = mp6[id->signature.model_num];
            }

            if ((id->signature.model_num < sizeof(mp6o)/sizeof(mp6o[0])) &&
                    (id->signature.processor_type == 1))
            {
                modelname = mp6o[id->signature.model_num];
            }

            break;
        }
    }
    else if (memcmp(id->vendor_id, "UMC UMC UMC ", 12) == 0)
    {
        static const char *u486[] =
        {   "", " U5D", " U5S"};

        company = "UMC ";
        switch (id->signature.family_code)
        {
            case 4:
            if (id->signature.model_num < sizeof(u486)/sizeof(u486[0]))
            {
                modelname = u486[id->signature.model_num];
            }

            break;
        }
    }
    else if (memcmp(id->vendor_id, "AuthenticAMD", 12) == 0)
    {
        static const char *a486[] =
        {
            "", "", "", "DX2", "", "", "", "DX2WB",
            "DX4", "DX4WB", "", "", "", "", "X5WT", "X5WB"
        };

        static const char *ak5[] =
        {   " SSA5", " 5k86"};

        company = "AMD ";
        switch (id->signature.family_code)
        {
            case 4:
            if (id->signature.model_num < sizeof(a486)/sizeof(a486[0]))
            {
                modelname = a486[id->signature.model_num];
            }

            break;
            case 5:
            family = "K5";

            if (id->signature.model_num < sizeof(ak5)/sizeof(ak5[0]))
            {
                modelname = ak5[id->signature.model_num];
            }

            break;
        }
    }
    else if (memcmp(id->vendor_id, "CyrixInstead", 12) == 0)
    {
        company = "Cyrix ";
        switch (id->signature.family_code)
        {
            case 4:
            family = "5x86";
            break;
            case 5:
            family = "6x86";
            break;
        }
    }
    else if (memcmp(id->vendor_id, "NexGenDriven", 12) == 0)
    {
        company = "NexGen ";
        switch (id->signature.family_code)
        {
            case 5:
            family = "Nx586";
            break;
        }
    }

    memcpy(vendorname, id->vendor_id, 12);
    vendorname[12] = 0;

    printk
    (
            "Processor : %s%s%s\n"
            "Vendor ID : %s\n"
            "Family    : %c86\n"
            "Model     : %d\n"
            "Stepping  : %c/%d\n"
            "Type      : %s\n"
            "Brand Str : %s\n",
            company,
            family,
            modelname,
            vendorname,
            '0' + id->signature.family_code,
            id->signature.model_num,
            'A' + id->signature.stepping_id,
            id->signature.stepping_id,
            cputype[id->signature.processor_type],
            id->brand_string
    );

    return;

    /* feature flags.  */
    for (i = 0; i < sizeof(features)/sizeof(features[0]); i++)
    {
        if (features[i])
        {
            printk("%s: %s\n",
                    features[i],
                    *((int *) &id->features) & (1 << i) ? "yes" : "no");
        }
    }
}

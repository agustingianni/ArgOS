#ifndef CPUID_H_
#define CPUID_H_

#define CPU_FAMILY_386			3
#define CPU_FAMILY_486			4
#define CPU_FAMILY_PENTIUM		5
#define CPU_FAMILY_PENTIUM_PRO	6
#define CPU_FAMILY_UNKNOWN		0xf

/*
 * CPUID operations 
 */
#define CPUID_QUERY_BASIC               0x00000000
#define CPUID_QUERY_VERSION             0x00000001
#define CPUID_QUERY_CACHE               0x00000002 /* Cache and TLB Descriptor Info */
#define CPUID_QUERY_PSN                 0x00000003 /* Processor Serial Number */
#define CPUID_QUERY_DETERMINISTIC_CACHE 0x00000004
#define CPUID_QUERY_MONITOR             0x00000005
#define CPUID_QUERY_THERMAL             0x00000006
#define CPUID_QUERY_PERFORMANCE         0x0000000a

/*
 * Get Highest Extended Function Supported
 */
#define CPUID_QUERY_EXT_FUNCTION        0x80000000

/*
 * Extended Processor Info and Feature Bits
 * get extended features bits (on EDX)
 */
#define CPUID_QUERY_EXT_FEATURES        0x80000001

/* returnded on EDX when EAX=CPUID_QUERY_EXT_FEATURES */
struct another_extended_features
{
	unsigned reserved0:2;
	unsigned _64bit:1;
	unsigned reserved1:8;
	unsigned xd:1;
	unsigned reserved2:8;
	unsigned syscallret:1;
	unsigned reserved3:11;
} __attribute__((packed));

/* returnded on ECX when EAX=CPUID_QUERY_EXT_FEATURES */
struct another2_extended_features
{
	unsigned reserved:31;
	unsigned lahf:1;
} __attribute__((packed));

/*
 * Get the processor brand string
 * 
 * These return the processor brand string in
 * EAX, EBX, ECX and EDX. CPUID must be issued 
 * with each parameter in sequence to get the 
 * entire 48-byte null-terminated ASCII 
 * processor brand string.
 */
#define CPUID_QUERY_EXT_PROC_BRAND0     0x80000002
#define CPUID_QUERY_EXT_PROC_BRAND1     0x80000003
#define CPUID_QUERY_EXT_PROC_BRAND2     0x80000004

#define CPUID_QUERY_EXT_RESERVED0       0x80000005
#define CPUID_QUERY_EXT_CACHE           0x80000006
#define CPUID_QUERY_EXT_RESERVED1       0x80000007
#define CPUID_QUERY_EXT_ADDR_SIZE       0x80000008

/* Used on cpuid_signature.type field */
#define CPUID_TYPE_OEM 			0
#define CPUID_TYPE_OVERDRIVE 	1
#define CPUID_TYPE_DUAL 		2
#define CPUID_TYPE_RESERVED 	3

/* returnded on EAX when EAX=CPUID_QUERY_VERSION */
/* family = extended_family + family_code */
struct cpuid_signature
{
	unsigned stepping_id	:4;
	unsigned model_num		:4;
	unsigned family_code	:4;
	unsigned processor_type	:2;
	unsigned reserved1		:2;
	unsigned extended_model :4;
	unsigned extended_family:8;
	unsigned reserved2		:4;
};

/* returnded on EBX when EAX=CPUID_QUERY_VERSION */
struct cpuid_version
{
	unsigned brand_index:8;
	unsigned cflush_line_size:8;
	unsigned number_of_processors:8;
	unsigned initial_apic_id:8;
} __attribute__((packed));

/* returnded on ECX when EAX=CPUID_QUERY_VERSION */
struct cpuid_features
{
	unsigned sse3:1;		/* sse3 extensions */
	unsigned __res0:2;		/* reserved */
	unsigned monitor:1;		/* monitor/mwait */
	unsigned ds_cpl:1;		/* cpl qualified debug store */
	unsigned vmx:1;		    /* virtual machine technology present? */
	unsigned __res1:1;		/* reserved */
	unsigned tm2:1;		    /* thermal monitor 2 */
	unsigned est:1;		    /* speedstep technology present? */
	unsigned ssse3:1;		/* ssse3 extensions present? */
	unsigned cnxt_id:1;		/* l1 context id */
	unsigned __res2:2;		/* reserved */
	unsigned CMPXCHG16B:1;	/*  */
	unsigned xtpr:1;		/* update control present? */
	unsigned __res3:17;		/* reserved */
}__attribute__((packed));

/* returnded on EDX when EAX=CPUID_QUERY_VERSION */
struct cpuid_extended_features
{
	unsigned fpu	: 1;	/* FPU present ? */
	unsigned vme	: 1;	/* Virtual mode extension. */
	unsigned dbgext	: 1;	/* Debugging extension. */
	unsigned pse	: 1;	/* Page size extension. */
	unsigned tsc	: 1;	/* Time stamp counter. */
	unsigned msr	: 1;	/* Model specific registers. */
	unsigned pae	: 1;	/* Physical address extension. */
	unsigned mce	: 1;	/* Machine check exception. */
	unsigned cx8	: 1;	/* cmpxchg8 instruction supported. */
	unsigned apic	: 1;	/* On-chip APIC hardware support. */
	unsigned __res0	: 1;	/* Reserved. */
	unsigned sep	: 1;	/* Fast system call. */
	unsigned mtrr	: 1;	/* Memory type range register. */
	unsigned pge	: 1;	/* Page Global enable. */
	unsigned mca	: 1;	/* Machine check architecture. */
	unsigned cmov	: 1;	/* Conditional move instruction. */
	unsigned pat	: 1;	/* Page attribute table. */
	unsigned pse_36	: 1;	/* 36-bit page size extension. */
	unsigned psn	: 1;	/* Processor serial number present. */
	unsigned clfsh	: 1;	/* clflush instruction supported. */
	unsigned __res1	: 1;	/* Reserved. */
	unsigned ds	    : 1;	/* Debug store. */
	unsigned acpi	: 1;	/* Thermal monitor and controlled clock support. */
	unsigned mmx	: 1;	/* MMX technology support. */
	unsigned fxsr	: 1;	/* Fast floating point save and restore. */
	unsigned sse	: 1;	/* Streaming SIMD extension support. */
	unsigned sse2	: 1;	/* Streaming SIMD extension 2 support. */
	unsigned ss	    : 1;	/* Self-snoop. */
	unsigned htt	: 1;	/* Hyper-threading technology. */
	unsigned tm	    : 1;	/* Thermal monitor supported. */
	unsigned __res2	: 1;	/* Reserved. */
	unsigned sbf	: 1;	/* Signal break on ferr. */
}__attribute__((packed));

typedef struct cpu_information
{
    struct cpuid_signature signature;
    struct cpuid_version version;       /* also called the brand id */
    struct cpuid_features features;
    struct cpuid_extended_features ext_features;

    char brand_string[48];
    char vendor_id[12];
    char name[64];

    unsigned int frequency;             /* in KHZ */
    unsigned int l1_cache_size;            /* in KBytes */
    unsigned int l2_cache_size;            /* in KBytes */
    
} cpuinfo_t;


extern cpuinfo_t cpu_info;

static inline int cpu_has_pse(void)
{
	return cpu_info.ext_features.pse; 
}

static inline int cpu_has_pge(void)
{
	return cpu_info.ext_features.pge; 
}

/*
 * Executes the cpuid intruction
 */
static inline void
cpuid
(
    unsigned int op, 
    unsigned int *eax, 
    unsigned int *ebx, 
    unsigned int *ecx, 
    unsigned int *edx
)
{
	asm
        (
            "cpuid"
            : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
            : "0" (op)
        );
}

/*
 * This routine checks if the CPUID instruction
 * is available by testing if we can set the ID
 * field of EFLAGS.
 */

static inline int
cpuid_check(void)
{
	int flag1, flag2;

	__asm__ __volatile__
	(
		"pushfl\n"
		"pushfl\n"
		"popl %0\n"
		"movl %0, %1\n"
		"xorl %2, %0\n"
		"pushl %0\n"
		"popfl\n"
		"pushfl\n"
		"popl %0\n"
		"popfl\n"
		: "=r"(flag1), "=r"(flag2)
		: "i"(0x00200000)
	);
	
	return ((flag1^flag2) & 0x00200000);
}

/*
 * Returns the EFLAGS register
 */
static inline unsigned
_get_eflags(void)
{
	unsigned eflags;
	asm volatile
	(
		"pushfl\n\t"
		"popl %0\n\t"
		: "=r" (eflags)
	);
	
	return eflags;
}

/*
 * Sets the EFLAGS register
 */
static inline void 
_set_eflags(unsigned eflags)
{
	asm volatile
	(
		"pushl %0\n\t"
		"popfl\n\t"
		:
		: "r" (eflags)
	);
}

/*
 * Returns true if cpu == i386 
 */
static inline int
cpu_is_386(void)
{
	int oeflags = _get_eflags();
	int ret = 0;
	
	_set_eflags(oeflags ^  0x00040000);
	ret = ((_get_eflags() ^ oeflags) & 0x00040000);
	_set_eflags(oeflags);
	
	return ret;
}

/*
 * Returns true if cpu >= i486 
 */
static inline int 
cpu_is_486(void)
{
	int oeflags = _get_eflags();
	int ret = 0;
	
	/* try to set the CPUID flag */
	_set_eflags(oeflags ^ 0x00200000);
	ret = ((_get_eflags() ^ oeflags) & 0x00200000);
	_set_eflags(oeflags);
	
	return ret;
}

extern cpuinfo_t cpu_info;

void print_cpu_information(cpuinfo_t *);
void init_cpu(cpuinfo_t *c);

#endif /*CPUID_H_*/

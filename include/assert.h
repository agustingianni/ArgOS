#ifndef ASSERT_H_
#define ASSERT_H_

#include <kernel/printk.h>

#if __STDC_VERSION__ < 199901L
#if __GNUC__ >= 2
#define __func__ __FUNCTION__
#else
#define __func__ "<unknown>"
#endif
#endif

#define assert(cond) 							\
do {									\
	if (!(cond)) {							\
		printk(	"Assertion Failed:\n"				\
				"  Function: %s\n"			\
				"  Condition: %s\n"			\
				"  File: %s\n"				\
				"  Line: %d\n"				\
				"  Return address: 0x%x\n",		\
			__func__, #cond, __FILE__, __LINE__,		\
			(unsigned long) __builtin_return_address(0));	\
	}								\
} while (0)

#endif /*ASSERT_H_*/

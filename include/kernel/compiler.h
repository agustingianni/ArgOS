#ifndef __COMPILER_H
#define __COMPILER_H

/*
 * These macros were extracted from the linux kernel
 * linux/compiler.h
 */

#define __user		__attribute__((noderef, address_space(1)))
#define __kernel	/* default address space */
#define __safe		__attribute__((safe))
#define __force	__attribute__((force))
#define __nocast	__attribute__((nocast))
#define __iomem	__attribute__((noderef, address_space(2)))
#define __acquires(x)	__attribute__((context(x,0,1)))
#define __releases(x)	__attribute__((context(x,1,0)))
#define __acquire(x)	__context__(x,1)
#define __release(x)	__context__(x,-1)
#define __cond_lock(x,c)	((c) ? ({ __acquire(x); 1; }) : 0)

/*extern void __chk_user_ptr(const void __user *);
extern void __chk_io_ptr(const void __iomem *);*/

/*
 * Generic compiler-dependent macros required for kernel
 * build go below this comment. Actual compiler/compiler version
 * specific implementations come from the above header files
 */

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#ifndef __deprecated
#define __deprecated		/* unimplemented */
#endif

#ifndef __must_check
#define __must_check
#endif

#endif /* __COMPILER_H */

#ifndef BUG_H_
#define BUG_H_

#include <kernel/utils.h>	/* panic is defined here */
#include <compiler.h>

#define BUG() do { \
	printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
	panic("BUG!"); \
} while (0)

#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)

#define WARN_ON(condition) ({						\
	typeof(condition) __ret_warn_on = (condition);			\
	if (unlikely(__ret_warn_on)) {					\
		printk("WARNING: at %s:%d %s()\n", __FILE__,		\
			__LINE__, __FUNCTION__);			\
	}								\
	unlikely(__ret_warn_on);					\
})

#endif /*BUG_H_*/

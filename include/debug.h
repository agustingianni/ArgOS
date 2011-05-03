/* 
 * File:   debug.h
 * Author: gr00vy
 *
 * Created on May 2, 2008, 1:36 PM
 */

#ifndef _DEBUG_H
#define	_DEBUG_H

#define DUMP(var, fmt) printk("%s():%u: %s=" fmt, __FUNCTION__, __LINE__, #var, var)

#endif	/* _DEBUG_H */


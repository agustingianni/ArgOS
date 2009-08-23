/* 
 * File:   cache.h
 * Author: gr00vy
 *
 * Created on May 25, 2008, 11:30 PM
 */

#ifndef _CACHE_H
#define	_CACHE_H

/* L1 cache line size */
#define L1_CACHE_SHIFT	(7)
#define L1_CACHE_BYTES	(1 << L1_CACHE_SHIFT)
#define L1_CACHE_ALIGN(x) (((x)+(L1_CACHE_BYTES-1)) &~ (L1_CACHE_BYTES-1))

#endif	/* _CACHE_H */


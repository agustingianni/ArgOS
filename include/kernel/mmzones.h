#ifndef MMZONES_H_
#define MMZONES_H_


typedef unsigned char zonetype_t;

/*
 * Zones for IA-32 just like linux :)
 * 
 * ZONE_DMA		DMA-able pages				< 16MB
 * ZONE_NORMAL	Normally addressable pages	16896MB
 * ZONE_HIGHMEM	Dynamically mapped pages	> 896MB
 */

#define ZONE_DMA		0	/*  */
#define ZONE_NORMAL		1	/*  */
#define ZONE_HIGHMEM	2	/*  */

struct zone
{
	zonetype_t 		zone_type;
    unsigned long	free_pages;   
} zone_t;

#endif /*MMZONES_H_*/

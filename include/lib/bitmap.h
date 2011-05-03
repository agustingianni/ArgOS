#ifndef BITMAP_H_
#define BITMAP_H_

#include <types.h>

typedef struct bitmap
{
	uint32_t *map;
	uint32_t size;	/* in bits */
} bitmap2_t;

typedef uint32_t * bitmap_t;

#define BITMAP_BITS_PER_ENTRY \
	(sizeof(uint32_t)*8)

#define BITMAP_REMAINING_BITS(x)\
	(x % BITMAP_BITS_PER_ENTRY)

/* FIXME: Overflow */
#define BITMAP_REMAINING_MASK(x)\
	((1UL << BITMAP_REMAINING_BITS(x))-1)

/* FIXME: Overflow */
#define BITMAP_MASK(bit) \
	((1UL << BITMAP_REMAINING_BITS(bit)))

#define BITMAP_SIZE_LONGS(x)\
	(x / BITMAP_BITS_PER_ENTRY)

#define BITMAP_SIZE_LONGS_ROUNDUP(x)\
	((x / BITMAP_BITS_PER_ENTRY) + (BITMAP_REMAINING_BITS(x) ? 1 : 0)) 


#define BITMAP_INDEX_LONGS(x) \
	(x / BITMAP_BITS_PER_ENTRY)

void bitmap_zero(bitmap_t bitmap, uint32_t size);
void bitmap_fill(bitmap_t bitmap, uint32_t size, uint32_t value);
int  bitmap_empty(const bitmap_t bitmap, uint32_t size);
int  bitmap_full(const bitmap_t bitmap, uint32_t size);
int  bitmap_equal(const bitmap_t bitmap1, const bitmap_t bitmap2, uint32_t size);
int  bitmap_set(bitmap_t bitmap, uint32_t bit);
int  bitmap_unset(bitmap_t bitmap, uint32_t bit);
int  bitmap_is_set(bitmap_t bitmap, uint32_t bit);
int  bitmap_set_region(bitmap_t bitmap, uint32_t position, uint32_t size);
int  bitmap_unset_region(bitmap_t bitmap, uint32_t position, uint32_t size);
int  bitmap_next_zero(bitmap_t bitmap, size_t size, size_t offset);

#endif /*BITMAP_H_*/

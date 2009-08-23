#include <types.h>
#include <lib/bitmap.h>

#ifdef BITMAP_UNIT_TEST
  #include <stdio.h>
  #include <string.h>
#endif

/*
 * 
 * Clears the bitmap, the size is specified in bits
 */
void
bitmap_zero(bitmap_t bitmap, uint32_t size)
{
	int i;
	
	/* Fill the first bits */
	for(i = 0; i < BITMAP_SIZE_LONGS_ROUNDUP(size); i++)
	{
		bitmap[i] = 0UL;
	}
	
	return;
}

void
bitmap_fill(bitmap_t bitmap, uint32_t size, uint32_t value)
{
	int i;
	
	for(i = 0; i < BITMAP_SIZE_LONGS_ROUNDUP(size); i++)
	{
		bitmap[i] = value;
	}
	
	return;
}

/*
 * Returns 1 if the whole bitmap is empty, 0 if its not
 */
int
bitmap_empty(const bitmap_t bitmap, uint32_t size)
{
	int i;
	
	for(i = 0; i < size / BITMAP_BITS_PER_ENTRY; i++)
	{
		if(bitmap[i])
		{
			return 0;
		}
	}
	
	/* no sobra ningun bit */
	if(BITMAP_REMAINING_BITS(size))
	{
		if(bitmap[i] & BITMAP_REMAINING_MASK(size))
		{
			return 0;
		}
	}
	
	return 1;
}

/*
 * Returns 1 if the whole bitmap is full, 0 if its not
 */
int
bitmap_full(const bitmap_t bitmap, uint32_t size)
{
	int i;
	
	for(i = 0; i < size / BITMAP_BITS_PER_ENTRY; i++)
	{
		if(~bitmap[i])
		{
			return 0;
		}
	}
	
	/* no sobra ningun bit */
	if(BITMAP_REMAINING_BITS(size))
	{
		if(~bitmap[i] & BITMAP_REMAINING_MASK(size))
		{
			return 0;
		}
	}
	
	return 1;
}

/*
 * Bit to bit compare between two bitmaps
 * 
 * TODO: Maybe we can use memcpm
 */
int
bitmap_equal
(
	const bitmap_t bitmap1,
	const bitmap_t bitmap2,
	uint32_t size
)
{
	int i;
	
	for(i = 0; i < size / BITMAP_BITS_PER_ENTRY; i++)
	{
		if(bitmap1[i] != bitmap2[i])
		{
			return 0;
		}
	}
	
	/* no sobra ningun bit */
	if(BITMAP_REMAINING_BITS(size))
	{
		if((bitmap1[i] ^ bitmap2[i]) & (~BITMAP_REMAINING_MASK(size)))
		{
			return 0;
		}
	}
	
	return 1;
}

/*
 * Sets a given bit in the bitmap. If the bit is already set
 * it returns -1, 0 otherwise.
 */
int
bitmap_set(bitmap_t bitmap, uint32_t bit)
{
	if(bitmap_is_set(bitmap, bit))
		return -1;
	
	bitmap[bit/BITMAP_BITS_PER_ENTRY] |= 
		BITMAP_MASK(BITMAP_REMAINING_BITS(bit));
	
	return 0;
}

/*
 * Unsets a given bit in the bitmap
 * Returns -1 if the bit was already unset, 0 otherwise
 */
int
bitmap_unset(bitmap_t bitmap, uint32_t bit)
{
	if(!bitmap_is_set(bitmap, bit))
		return -1;

	bitmap[bit/BITMAP_BITS_PER_ENTRY] &= 
			~BITMAP_MASK(BITMAP_REMAINING_BITS(bit));
	
	return 0;
}

/*
 * Returns if the bit is set or not
 */
int
bitmap_is_set(bitmap_t bitmap, uint32_t bit)
{
	return bitmap[bit / BITMAP_BITS_PER_ENTRY] &
		BITMAP_MASK(BITMAP_REMAINING_BITS(bit));
}

/*
 * Tries to find a free region of size 'size'. Returns negative
 * if it cannot find one. This negative number indicates
 * the biggest free region found. So the caller may decide what to 
 * do.
 */
#if 0
int
bitmap_find_region(const bitmap_t bitmap, uint32_t size)
{
	int i = 0;
	int idx = 0;
	int stop = 0;

	while(!stop)
	{
		while(i < (bits % BITMAP_BITS_PER_ENTRY) &&
			(bitmap[idx] == 0))
		{
			bits += BITMAP_BITS_PER_ENTRY;
			i++;
			idx++;
		}
		
		if(i == (bits % BITMAP_BITS_PER_ENTRY))
		{
			bitmap[idx]
		}
	}
	
	return -1;
}
#endif

/*
 * Allocates a region of a given size starting from position
 * 'position'
 * 
 * TODO: NOT WORKING
 */
int
bitmap_set_region
(
	bitmap_t bitmap, 
	uint32_t position,	/* position inside bitmap (in bits) */ 
	uint32_t size		/* size of the region (in bits) */
)
{
	uint32_t long_position 	= BITMAP_INDEX_LONGS(position);
	/*
	uint32_t remaining 		= BITMAP_REMAINING_BITS(position);
	uint32_t bitmask 		= BITMAP_REMAINING_MASK(size);
	*/
	
	int i;
	
	/* seteamos los primeros bits */
	bitmap[long_position] |= BITMAP_REMAINING_MASK(position);
	
	/* empezando desde long_position (inicio de la region) hasta 
	 * long_position + size */
	for
	(
		i = long_position + 1;
		i < BITMAP_SIZE_LONGS(size);
		i++
	)
	{
		bitmap[i] |= 0xffffffff;
	}
	
	bitmap[i] |= BITMAP_REMAINING_MASK(size);
	
	return -1;
}

int
bitmap_next_zero(bitmap_t bitmap, size_t size, size_t offset)
{
	int i;
	
	for(i = offset; i < size; i++)
	{
		if(bitmap_is_set(bitmap, i) == 0)
			return i;
	}
	
	return -1;
}

/*
 * Deallocates a region of bits
 */
int
bitmap_unset_region
(
	bitmap_t bitmap, 
	uint32_t position, 
	uint32_t size
)
{
	return -1;
}

#ifdef BITMAP_UNIT_TEST
void
bin_print(const uint8_t bits)
{
	uint8_t mask = 1UL;
	int i;
	
	for(i = 0; i < 8; i++)
	{
		printf("%d ", (bits & mask) ? 1 : 0);
		mask <<= 1;
	}
}

void
bitmap_print(const bitmap_t bitmap, uint32_t size)
{
	int i;
	
	printf("Bitmap size normal  %d\n", BITMAP_SIZE_LONGS(size));
	printf("Bitmap size roundup %d\n", BITMAP_SIZE_LONGS_ROUNDUP(size));
	
	for(i = 0; i < BITMAP_SIZE_LONGS_ROUNDUP(size)*4; i++)
	{
		bin_print(*(((uint8_t *) bitmap)+i));
		printf("\n");
	}
	
}

int
main(int argc, char **argv)
{
	char buffer[512];
	bitmap_t bmap;

	#define BSIZE 64-9
	
	memset((void *) buffer, 0xff, 512);
	bmap = (bitmap_t) buffer;
	
	printf("0x00000000 bitmap:\n");
	bitmap_zero(bmap, BSIZE);
	bitmap_print(bmap, BSIZE);
	printf("Is empty? %s\n", bitmap_empty(bmap, BSIZE) ? "Y" : "N");

	bitmap_set_region(bmap, 0, 2);
	bitmap_print(bmap, BSIZE);
	
	return 0;
}
#endif

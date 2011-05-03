#include <mm/vm.h>

/*
 * Finds a suitable vma region for the given address
 * */
vm_map_t *
vma_find(vm_space_t *mm, vaddr_t address)
{
	vm_map_t *curr;
	
	/* for each vma */
	for(curr = mm->vma_list; curr; curr = curr->vma_next)
	{
		if((address >= curr->vma_start) &&
			(address < curr->vma_end))
		{
			return curr;
		}
	}
	
	return NULL;
}

/*
 * Checks if we have the right permissions to 
 * access the vma region
 */
int
vma_access_check(vm_map_t *vma, uint32_t access)
{
	return (vma->vma_flags & access);
}

/*
 * Returns true if the region grows down (like a stack)
 */
int
vma_growsdown(vm_map_t *vma)
{
	return (vma->vma_flags && VMA_GROWSDOWN);
}

int
vma_grow_stack(vm_map_t *vma)
{
	return 0;
}

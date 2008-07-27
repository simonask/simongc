#include "SimonGCMemoryHeap.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
	This is a convenience macro, needed because simply adding pointers
	generates the wrong offsets.
	
	The reason is that for a C compiler
		ptr = otherptr + offset
	is equivalent to
		ptr = otherptr[offset]
	which translates to
		ptr = (void*)otherptr + offset * sizeof(*otherptr)
	
	Long story short, this macro eliminates the need for horrible amounts
	of void* casts. 
*/
#define PTR_OFFSET(ptr, offset) ((void*)(ptr) + offset)

MemoryHeap* memory_heap_create(size_t initial_heap_size)
{
	MemoryHeap* heap = (MemoryHeap*)malloc(sizeof(MemoryHeap));
	heap->size = initial_heap_size ? initial_heap_size : (1 << 20);	// Default to 1M
	heap->offset = 0;
	heap->data = malloc(heap->size);
	if (!heap->data)
	{
		fprintf(stderr, "Could not allocate heap of size %d, aborting!\n", heap->size);
		free(heap);
		return NULL;
	}
	printf("Allocated heap of size %d\n", heap->size);
	return heap;
}

void memory_heap_destroy(MemoryHeap* heap)
{
	// TODO: Call finalizers?
	free(heap->data);
	free(heap);
}

void* memory_heap_allocate(MemoryHeap* heap, size_t n)
{
	unsigned char padding = REQUIRED_PADDING(n);
	size_t required_size = sizeof(ObjectHeader) + n + padding;
	
	if (required_size > memory_heap_available(heap))
		return NULL;	// TODO: try compacting, see if it helps -- if not, the allocator should decide if the appropriate course of action is to enlarge this heap, or move existing objects to a different heap, and then compacting.
	
	ObjectHeader* header = (ObjectHeader*)PTR_OFFSET(heap->data, heap->offset);
	void* obj = PTR_OFFSET(heap->data, heap->offset + sizeof(*header));
	memset(obj, 0, n + padding);
	header->size = n;
	header->generation = 0;
	header->flags = 0;
	header->padding = padding;
	printf("allocation: header: 0x%x\tdata: 0x%x\n", header, obj);
	
	heap->offset += required_size;
	return (void*)obj;
}

void memory_heap_compact(MemoryHeap* heap)
{
	
}

void memory_heap_enlarge(MemoryHeap* heap, size_t new_size)
{
	
}

bool memory_heap_walk(MemoryHeap* heap, ObjectHeader** header, void** object)
{	
	/*
		Case 0: The never-going-to-happen.
		We did not allocate anything in this heap yet.
	*/
	if (heap->offset == 0)
		return false;
	
	/*
		Case 1: The first
		*header is NULL, so this is the first iteration. The first object's
		header is the very first thing that'll turn up on the stack.
	*/
	if (!*header)	// *header is NULL, so we're initializing
	{
		*header = (ObjectHeader*)PTR_OFFSET(heap->data, 0);
		*object = PTR_OFFSET(heap->data, sizeof(**header));
		return true;
	}
	
	/*
		Case 2: The scary.
		We assume *header is a valid pointer (that we probably set outselves),
		and use it's information to advance through the heap, if such an
		advance does not exceed the bounds of allocated space.
	*/
	*object = PTR_OFFSET(*header, sizeof(**header));
	void* upper_bound = PTR_OFFSET(heap->data, heap->offset);
	while (PTR_OFFSET(*object, (*header)->size + (*header)->padding) < upper_bound)
	{
		// Advance!
		*header = (ObjectHeader*)PTR_OFFSET(*header, sizeof(**header) + (*header)->size + (*header)->padding);
		*object = PTR_OFFSET(*header, sizeof(**header));
		return true;
		/*
			TODO: Add flag matching?
		
		if (!(*_header)->flags & OBJECT_UNREACHABLE)
			return true;
		*/
	}
	
	/*
		Case 3: The boring.
		None of the conditions were matched, so we're probably at the end.
	*/
	*header = NULL;
	*object = NULL;
	return false;
}

bool memory_heap_contains(MemoryHeap* heap, void* object)
{
	return (object > heap->data && object < PTR_OFFSET(heap->data, heap->offset));
}

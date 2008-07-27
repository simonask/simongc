#include "SimonGCMemoryHeap.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
	unsigned char padding = REQUIRED_PADDING(n, ALIGNMENT);
	size_t required_size = sizeof(ObjectHeader) + n + padding;
	
	if (required_size > memory_heap_available(heap))
		return NULL;	// TODO: try compacting, see if it helps -- if not, the allocator should decide if the appropriate course of action is to enlarge this heap, or move existing objects to a different heap, and then compacting.
	
	ObjectHeader* header = (ObjectHeader*)(&heap->data[heap->offset]);
	void* obj = (void*)(&heap->data[heap->offset] + sizeof(*header));
	memset(obj, 0, n + padding);
	header->size = n;
	header->generation = 0;
	header->flags = 0;
	header->padding = padding;
	
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
	if (heap->offset == 0)
		return false;	// We didn't allocate anything yet, so just bail out.
	
	if (!*header)	// *header is NULL, so we're initializing
	{
		*header = (ObjectHeader*)&heap->data[0];
		*object = (void*)(heap->data + sizeof(**header));
		return true;
	}
	
	*object = (char*)(*header + sizeof(**header));
	void* upper_bound = (void*)(heap->data + heap->offset);
	while (*object + (*header)->size + (*header)->padding < upper_bound)
	{
		// Advance!
		*header += sizeof(**header) + (*header)->size + (*header)->padding;
		*object = (char*)(*header + sizeof(**header));
		return true;
		/*
			TODO: Add flag matching?
		
		if (!(*_header)->flags & OBJECT_UNREACHABLE)
			return true;
		*/
	}
	
	// No more objects to walk. :(
	*header = NULL;
	*object = NULL;
	return false;
}

bool memory_heap_contains(MemoryHeap* heap, void* object)
{
	return (object > (void*)heap->data && object < (void*)(heap->data + heap->offset));
}

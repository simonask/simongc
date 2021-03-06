#include "SimonGCMemoryHeap.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

/*
	Width means allocated size + padding.
*/
#define OBJECT_DATA_WIDTH(obj) ((obj)->meta.size + (obj)->meta.padding)
#define OBJECT_WIDTH(obj) (sizeof(ObjectMeta) + OBJECT_DATA_WIDTH(obj))

/* I'm such a lazy slob */
#define WRLOCK(heap) pthread_rwlock_wrlock(&(heap)->lock)
#define RDLOCK(heap) pthread_rwlock_rdlock(&(heap)->lock)
#define UNLOCK(heap) pthread_rwlock_unlock(&(heap)->lock)


MemoryHeap* memory_heap_create(size_t initial_heap_size, ObjectMovedCallback callback)
{
	MemoryHeap* heap = (MemoryHeap*)malloc(sizeof(MemoryHeap));
	heap->size = initial_heap_size ? initial_heap_size : (1 << 20);	// Default to 1M
	heap->offset = 0;
	heap->object_moved = callback;
	pthread_rwlock_init(&heap->lock, NULL);
	
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
	pthread_rwlock_destroy(&heap->lock);
	free(heap->data);
	free(heap);
}

void* memory_heap_allocate(MemoryHeap* heap, size_t n)
{
	WRLOCK(heap);
	
	unsigned char padding = REQUIRED_PADDING(n);
	size_t required_size = sizeof(ObjectMeta) + n + padding;
	
	void* ret = NULL;
	
	if (required_size > memory_heap_available(heap))
		goto out; /* Let the caller decide what to do. */
		
	Object* object = (Object*)PTR_OFFSET(heap->data, heap->offset);
	memset(object->data, 0, n + padding);
	object->meta.size = n;
	object->meta.generation = 0;
	object->meta.flags = OBJECT_NO_FLAGS;
	object->meta.padding = padding;
	
	heap->offset += required_size;
	ret = (void*)object->data;
	
	out:
	UNLOCK(heap);
	return ret;
}

void memory_heap_compact(MemoryHeap* heap)
{
	WRLOCK(heap);
	void* upper_bound = PTR_OFFSET(heap->data, heap->offset);
	void* candidate = PTR_OFFSET(heap->data, 0);
	size_t new_offset = 0;
	while (candidate < upper_bound)
	{
		Object* object = (Object*)candidate;
		size_t width = OBJECT_WIDTH(object);
		
		if (!(object->meta.flags & OBJECT_UNREACHABLE))
		{
			if (PTR_OFFSET(heap->data, new_offset) != candidate)
			{
				Object* new_object = (Object*)PTR_OFFSET(heap->data, new_offset);
				
				register size_t i;
				for (i = 0; i < width; ++i)
				{
					((unsigned char*)new_object)[i] = ((unsigned char*)object)[i];
				}
				
				if (heap->object_moved)
					heap->object_moved(object->data, new_object->data);
			}
			
			new_offset += width;
		}
		
		printf("adding %d to %x\n", width, candidate);
		candidate = PTR_OFFSET(candidate, width);
	}
	heap->offset = new_offset;
	UNLOCK(heap);
}

void memory_heap_enlarge(MemoryHeap* heap, size_t new_size)
{
	WRLOCK(heap);
	// TODO: Stuff...
	UNLOCK(heap);
}

/*
	TODO: Reimplement to use a callback so we can read-lock the heap.
*/
bool memory_heap_walk(MemoryHeap* heap, Object** object)
{
	/*
		Case 0: The never-going-to-happen.
		We did not allocate anything in this heap yet.
	*/
	if (heap->offset == 0)
		return false;
	
	/*
		Case 1: The first
		*object is NULL, so this is the first iteration. The first object's
		metadata is the very first thing that'll turn up on the stack.
	*/
	if (!*object)
	{
		*object = (Object*)PTR_OFFSET(heap->data, 0);
		return true;
	}
	
	/*
		Case 2: The scary.
		We assume *header is a valid pointer (that we probably set outselves),
		and use it's information to advance through the heap, if such an
		advance does not exceed the bounds of allocated space.
	*/
	void* upper_bound = PTR_OFFSET(heap->data, heap->offset);
	void* candidate = PTR_OFFSET(*object, OBJECT_WIDTH(*object));
	if (candidate < upper_bound)
	{
		// Advance!
		*object = candidate;
		return true;
	}
	
	/*
		Case 3: The boring.
		None of the conditions were matched, so we're probably at the end.
	*/
	*object = NULL;
	return false;
}

bool memory_heap_contains(MemoryHeap* heap, void* object)
{
	return (object > heap->data && object < PTR_OFFSET(heap->data, heap->offset));
}

ObjectMeta* memory_heap_get_meta(MemoryHeap* heap, void* object)
{
	if (!memory_heap_contains(heap, object))
		return NULL;
	
	ObjectMeta* meta = PTR_OFFSET(object, -sizeof(ObjectMeta));

	assert(memory_heap_contains(heap, (void*)meta) && "Requested metadata outside of the heap! Are you sure object points to the beginning of some object data in this heap?");
	
	return meta;
}

#include "MemoryHeap.h"

#include <string.h>

MemoryHeap::MemoryHeap(size_t initial_size) : size(initial_size), offset(0)
{
	data = static_cast<int8_t*>(malloc(initial_size));
}

MemoryHeap::~MemoryHeap()
{
	// TODO (maybe): call finalizers
	free(data);
}

void* MemoryHeap::allocate(size_t n)
{
	unsigned char padding = REQUIRED_PADDING(n, ALIGNMENT);
	size_t required_size = sizeof(ObjectHeader) + n + padding;
	if (required_size > available())
		return NULL;	// TODO: try compacting, see if it helps
	
	ObjectHeader* header = (ObjectHeader*)(&data[offset]);
	int8_t* obj = (int8_t*)&header[sizeof(ObjectHeader)];
	memset(obj, 0, n + padding);
	header->size = n;
	header->generation = 0;
	header->flags = 0;
	header->padding = padding;
	
	offset += required_size;
	return obj;
}

bool MemoryHeap::walkObjects(ObjectHeader** _header, void** _object)
{
	// Convenience casts, lest the compiler give us the treatment
	int8_t** header = (int8_t**)_header;
	int8_t** object = (int8_t**)_object;
	
	if (offset == 0)
		return false;	// We didn't allocate anything yet, so just bail out.
	
	if (*header < data)	// *header is probably NULL, and we're initializing
	{
		*header = data;
		*object = &data[sizeof(**_header)];
		return true;
	}
	
	*object = *header + sizeof(**_header);
	int8_t* upper_bound = &data[offset];
	while (*object + (*_header)->size + (*_header)->padding < upper_bound)
	{
		// Advance!
		*header += sizeof(**_header) + (*_header)->size + (*_header)->padding;
		*object = *header + sizeof(**_header);
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

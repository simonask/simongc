#include <stdio.h>
#include "SimonGCRuntime.h"

#include <assert.h>

void finalizer(void* data)
{
	printf("Finalizer called for 0x%x!\n", data);
}

int main (int argc, char const *argv[])
{
	simon_gc_initialize(1024 * 8);
	
	char* lol = (char*)simon_gc_malloc(1024);
	assert(lol && "Out of memory, or error in simon_gc_malloc :(");
	simon_gc_stack_push(1, &lol);
	
	size_t i;
	for (i = 0; i < 1024; ++i)
	{
		lol[i] = 'a' + (i % ('z' - 'a'));
	}
	
	char* buffer = (char*)simon_gc_malloc(1024);
	while (buffer)
	{
		simon_gc_stack_push(1, &buffer);
		buffer = (char*)simon_gc_malloc(1024);
		simon_gc_stack_pop();
	}
	
	ObjectHeader* header = NULL;
	void* data;
	while (simon_gc_walk_objects(&header, &data))
	{
		printf("Object at 0x%x (size: %d, header: 0x%x)\n", data, header->size, header);
	}
	
	simon_gc_collect();
	simon_gc_stack_pop();
	simon_gc_collect();
	header = NULL;
	while (simon_gc_walk_objects(&header, &data))
	{
		// Should be none.
		if (header->flags & OBJECT_UNREACHABLE)
			printf("Object at 0x%x (size: %d) is unreachable!\n", data, header->size);
	}
	
	return 0;
}
#include <stdio.h>
#include "SimonGCRuntime.h"

int main (int argc, char const *argv[])
{
	simon_gc_initialize(0);
	
	char* lol = (char*)simon_gc_malloc(1024);
	printf("allocated 'lol' at 0x%x\n", lol);
	simon_gc_stack_push(1, &lol);
	
	size_t i;
	for (i = 0; i < 1024; ++i)
	{
		lol[i] = 'a' + (i % ('z' - 'a'));
	}
	printf("Yay: %s\n", lol);
	
	ObjectHeader* header = NULL;
	void* data;
	while (simon_gc_walk_objects(&header, &data))
	{
		printf("Object at 0x%x (size: %d)\n", data, header->size);
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
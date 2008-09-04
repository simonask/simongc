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
	printf("sizeof(Object): %d\n", sizeof(Object));
	
	char* lol = (char*)simon_gc_malloc(1024);
	assert(lol && "Out of memory, or error in simon_gc_malloc :(");
	simon_gc_stack_push(1, &lol);
	
	size_t i;
	for (i = 0; i < 1024; ++i)
	{
		lol[i] = 'a' + (i % ('z' - 'a'));
	}
	printf("YAY: %s\n", lol);
	
	char* buffer = (char*)simon_gc_malloc(1024);
	while (buffer)
	{
		simon_gc_stack_push(1, &buffer);
		buffer = (char*)simon_gc_malloc(1024);
		simon_gc_stack_pop();
	}
	
	Object* object = NULL;
	while (simon_gc_walk_objects(&object))
	{
		printf("Object at 0x%x (size: %d, header: 0x%x)\n", object->data, object->meta.size, object);
	}
	
	simon_gc_collect();
	simon_gc_stack_pop();
	simon_gc_collect();
	object = NULL;
	while (simon_gc_walk_objects(&object))
	{
		// Should be none.
		if (object->meta.flags & OBJECT_UNREACHABLE)
			printf("Object at 0x%x (size: %d) is unreachable!\n", object->data, object->meta.size);
	}
	simon_gc_collect();
	
	return 0;
}
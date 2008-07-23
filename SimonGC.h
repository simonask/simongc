#ifndef SIMON_GC_H
#define SIMON_GC_H

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*ObjectFinalizer)(void* obj);

	typedef enum ObjectFlags
	{
		OBJECT_UNREACHABLE = 1,
		OBJECT_ATOMIC      = 1 << 1,
		OBJECT_FINALIZED   = 1 << 2
	} ObjectFlags;
	
	#define ALIGNMENT 16
	#define REQUIRED_PADDING(min, alignment) ((alignment) - ((min) % (alignment)))
	
	typedef struct ObjectHeader
	{
		size_t size;
		ObjectFinalizer finalizer;
		unsigned int generation : 16;	/* How many GC passes has the object survived? */
		unsigned int flags : 16;
		
		/*
			The reason for the following hullaballooza is that we want our allocated
			objects to be aligned at 16 bytes. This is because it can give
			some performance benefits on some architectures, notably x86.
			TODO: Actually test if this makes a difference.
		*/
		union
		{
			/*
				The amount of padding in the allocated memory block.
				Note the irony: This field doubles as actual padding for the
				ObjectHeader struct. 
			 */
			unsigned int padding : 8;
			unsigned char reserved[REQUIRED_PADDING(sizeof(size_t) + sizeof(ObjectFinalizer) + 4, ALIGNMENT)];	/* pad up to ALIGNMENT */
		};

	} ObjectHeader;

	void simon_gc_init(size_t initial_heap_size);
	void* simon_malloc(size_t bytes);
	void simon_free(void* ptr);		/* Doesn't actually do anything */
	bool simon_walk_objects(ObjectHeader** header, void** data);
	ObjectFinalizer simon_set_finalizer(void* obj, ObjectFinalizer finalizer);
	void simon_gc();

#ifdef __cplusplus
}
#endif


#endif


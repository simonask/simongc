#ifndef SIMON_GC_RUNTIME_H
#define SIMON_GC_RUNTIME_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus__
namespace SimonGC {
extern "C" {
#endif

typedef void(*ObjectFinalizer)(void* obj);

typedef enum ObjectFlags
{
	OBJECT_NO_FLAGS    = 0,
	OBJECT_UNREACHABLE = 1,
	OBJECT_ATOMIC      = 1 << 1,
	OBJECT_FINALIZED   = 1 << 2
} ObjectFlags;

#define ALIGNMENT 0x10
#define REQUIRED_PADDING(min) (ALIGNMENT - ((min) % ALIGNMENT))
#define KINDA_LOOKS_LIKE_A_POINTER(maybe_ptr) (maybe_ptr & ALIGNMENT == 0)

typedef struct ObjectMeta
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
			Note the irony: This field doubles as actual padding for this struct.
		*/
		unsigned int padding : 8;

		/*
			The remaining amount of required padding.
			Note that "+5" is

			  sizeof(generation) + sizeof(flags) + sizeof(padding)

			The reason this field is in a union with 'padding' is that
			the array must be at least 1 byte long, so if sizeof all the
			other members is == 0 mod ALIGNMENT, the required padding will
			still be >= 1.
		*/
		unsigned char reserved[REQUIRED_PADDING(sizeof(size_t) + sizeof(ObjectFinalizer) + 5)];
	};
} ObjectMeta;

typedef struct Object {
	struct ObjectMeta meta;
	unsigned char data[];
} Object;

void simon_gc_initialize(size_t initial_heap_size);
void simon_gc_register_thread();
void* simon_gc_malloc(size_t n);
void simon_gc_collect();
void simon_gc_yield();

bool simon_gc_walk_objects(Object**);

/*
	simon_gc_stack_push/pop are for gluing the garbage
	collector onto C code.
	
	This is an accurate GC, so unfortunately it needs one of the following:
		a) Compiler support
		b) Explicit root annotation.
	
	Since I'm too lazy to actually modify the C compiler, option b is what
	you get in C. :)
	
	Note that things WILL break if you forget a call to simon_gc_stack_pop!
	This means calling it before EVERY exit point in your function, guys!
	
	Real language implementations will emulate the StackFrame data structure
	on their own stack, and maintain the linked list of stacks using some
	functions I haven't written yet.
 */
void simon_gc_stack_push(size_t count, ...); 
void simon_gc_stack_pop();

#ifdef __cplusplus__
}
}
#endif

#endif

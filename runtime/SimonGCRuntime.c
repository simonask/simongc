#include "SimonGCRuntime.h"
#include "SimonGCMemoryHeap.h"
#include <pthread.h>
#include <stdarg.h>

static MemoryHeap* young = NULL;
static MemoryHeap* elderly = NULL;

static pthread_mutex_t gc_mutex;
static pthread_key_t stack_frame_head_key;

static void gc_collect();
static inline void gc_lock() { pthread_mutex_lock(&gc_mutex); }
static inline void gc_unlock() { pthread_mutex_unlock(&gc_mutex); }
static inline bool gc_trylock() { return pthread_mutex_trylock(&gc_mutex); }


void simon_gc_initialize(size_t initial_heap_size)
{
	young = memory_heap_create(initial_heap_size);
	
	pthread_mutex_init(&gc_mutex, NULL);
	pthread_key_create(&stack_frame_head_key, NULL);
	
	simon_gc_register_thread();
}

void simon_gc_register_thread()
{
	// set thread-local root head
}

void* simon_gc_malloc(size_t n)
{
	gc_lock();
	// TODO: Maybe call gc_collect
	void* ptr = memory_heap_allocate(young, n);
	gc_unlock();
	return ptr;
}

void gc_collect()
{
	// TODO: Do it!
}

void simon_gc_collect()
{
	gc_lock();
	gc_collect();
	gc_unlock();
}

void simon_gc_yield()
{
	gc_lock();
	gc_unlock();
}

StackFrame* simon_gc_roots(size_t count, ...)
{
	StackFrame* frame = (StackFrame*)malloc(sizeof(StackFrame) + sizeof(void*) * count);
	frame->parent = NULL; // TODO: Current thread-local head
	
	va_list varg;
	va_start(varg, count);
	size_t i;
	for (i = 0; i < count; ++i)
	{
		frame->roots[i] = va_arg(varg, void**);
	}
	va_end(varg);
	
	// TODO: Append to linked list.
	return frame;
}

void simon_gc_cleanup_roots(StackFrame* frame)
{
	// TODO: Run through linked list and check if anything points here, and if it does, point it to this one's parent instead-
	free(frame);
}

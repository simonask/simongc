#include "SimonGCRuntime.h"
#include "SimonGCMemoryHeap.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

static MemoryHeap* young = NULL;
static MemoryHeap* elderly = NULL;

static pthread_mutex_t gc_mutex;
static pthread_key_t stack_frame_head_key;

static void gc_collect();
static void gc_mark(MemoryHeap* heap);
static void gc_sweep(MemoryHeap* heap);
static void gc_object_moved(void* from, void* to);
static inline void gc_lock() { pthread_mutex_lock(&gc_mutex); }
static inline void gc_unlock() { pthread_mutex_unlock(&gc_mutex); }
static inline bool gc_trylock() { return pthread_mutex_trylock(&gc_mutex); }

typedef struct StackFrame {
	struct StackFrame* parent;
	size_t num_roots;
	void** roots[];
} StackFrame;

void simon_gc_initialize(size_t initial_heap_size)
{
	young = memory_heap_create(initial_heap_size, gc_object_moved);
	
	pthread_mutex_init(&gc_mutex, NULL);
	pthread_key_create(&stack_frame_head_key, NULL);
	
	simon_gc_register_thread();
}

void simon_gc_register_thread()
{
	// TODO: Update global list of stackframe lists
	pthread_setspecific(stack_frame_head_key, NULL);
}

void* simon_gc_malloc(size_t n)
{
	gc_lock();
	
	void* ptr = memory_heap_allocate(young, n);
	if (!ptr)
	{
		gc_collect();
		ptr = memory_heap_allocate(young, n);
	}
	
	gc_unlock();
	return ptr;
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

void simon_gc_stack_push(size_t count, ...)
{
	StackFrame* frame = (StackFrame*)malloc(sizeof(StackFrame) + sizeof(void*) * count);

	StackFrame* current_head = (StackFrame*)pthread_getspecific(stack_frame_head_key);
	frame->parent = current_head;
	frame->num_roots = count;
	
	pthread_setspecific(stack_frame_head_key, frame);
	
	va_list varg;
	va_start(varg, count);
	size_t i;
	for (i = 0; i < count; ++i)
	{
		frame->roots[i] = va_arg(varg, void**);
	}
	va_end(varg);
}

void simon_gc_stack_pop()
{
	StackFrame* current_head = (StackFrame*)pthread_getspecific(stack_frame_head_key);
	if (current_head)
	{
		pthread_setspecific(stack_frame_head_key, current_head->parent);
	}
	
	free(current_head);
}

bool simon_gc_walk_objects(Object** object)
{
	return memory_heap_walk(young, object);
}

void gc_collect()
{
	gc_mark(young);
	gc_sweep(young);
}

void gc_mark(MemoryHeap* heap)
{
	StackFrame* head = (StackFrame*)pthread_getspecific(stack_frame_head_key);
	
	Object* object = NULL;
	while (memory_heap_walk(heap, &object))
	{
		if (object->meta.flags & OBJECT_UNREACHABLE)	// Marked by a previous iteration, or manually by the user.
			continue;
		
		StackFrame* frame = head;
		bool reachable = false;
		while (frame)
		{
			signed int i;	// size_t is unsigned, so reverse looping is risky business.
			for (i = head->num_roots - 1; i >= 0; --i)
			{
				if (*head->roots[i] == object->data)
					reachable = true;
			}
			frame = frame->parent;
		}
		
		// TODO: Look for pointers in data
		
		if (!reachable)
			object->meta.flags |= OBJECT_UNREACHABLE;
		else
			++(object->meta.generation);
	}
}

void gc_sweep(MemoryHeap* heap)
{
	Object* object = NULL;
	while (memory_heap_walk(heap, &object))
	{
		if (object->meta.flags & OBJECT_UNREACHABLE && !(object->meta.flags & OBJECT_FINALIZED))
		{
			if (object->meta.finalizer)
				object->meta.finalizer(object->data);
		}
	}
	
	memory_heap_compact(heap); // TODO: Keep list of moved objects
	// TODO: Update roots with new pointers
}

void gc_object_moved(void* from, void* to)
{
	printf("Object moved from 0x%x to 0x%x\n", from, to);
}

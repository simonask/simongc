#ifndef SIMON_MEMORY_HEAP
#define SIMON_MEMORY_HEAP

#ifdef __cplusplus__
extern "C" {
#endif

#include "SimonGCRuntime.h"

typedef struct MemoryHeap {
	char* data;
	size_t size;
	size_t offset;
} MemoryHeap;

MemoryHeap* memory_heap_create(size_t initial_heap_size);
void memory_heap_destroy(MemoryHeap*);
void* memory_heap_allocate(MemoryHeap*, size_t n);
void memory_heap_compact(MemoryHeap*);
void memory_heap_enlarge(MemoryHeap*, size_t new_size);

bool memory_heap_walk(MemoryHeap*, ObjectHeader** header, void** data);
bool memory_heap_contains(MemoryHeap*, void* object);
#define memory_heap_available(heap) ((heap)->size - (heap)->offset)
#define memory_heap_allocated_size(heap) ((heap)->offset)

#ifdef __cplusplus__
}
#endif

#endif

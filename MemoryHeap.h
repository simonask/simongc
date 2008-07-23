#ifndef SIMON_MEMORY_HEAP
#define SIMON_MEMORY_HEAP

#include <cstdlib>
using namespace std;

#include "SimonGC.h"

//namespace
//{
	class MemoryHeap
	{
	private:
		int8_t* data;
		size_t size;
		size_t offset;
	public:
		explicit MemoryHeap(size_t initial_size);
		~MemoryHeap();
		void* allocate(size_t n);
		bool compact();
		bool enlarge(size_t new_size);
		
		bool walkObjects(ObjectHeader** header, void** object);
		
		bool contains(void* object);
		inline size_t available() { return size - offset; }
		inline size_t allocatedSize() { return offset; }
	};
//}

#endif

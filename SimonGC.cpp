#include <llvm/CodeGen/Collector.h>
#include <llvm/CodeGen/Collectors.h>
#include <llvm/CodeGen/CollectorMetadata.h>
#include <llvm/Support/Compiler.h>

using namespace llvm;

#include <stdio.h>
#include <iostream>

using namespace std;

#include "MemoryHeap.h"
#include <assert.h>

namespace
{
	class VISIBILITY_HIDDEN SimonCollector : public Collector
	{
	private:
		MemoryHeap* young;
		MemoryHeap* elderly;
	public:
		SimonCollector() : young(NULL), elderly(NULL)
		{
			CustomWriteBarriers = true;
			NeededSafePoints = 1 << GC::Loop | 1 << GC::Return | 1 << GC::PreCall | 1 << GC::PostCall;
			std::cout << "LOL" << std::endl;
		}
		
		void initialize(size_t initial_heap_size)
		{
			std::cout << "ObjectHeader size: " << sizeof(ObjectHeader) << std::endl;
			young = new MemoryHeap(initial_heap_size);
		}
		
		void* allocate(size_t n)
		{
			assert(young != NULL && "GC not initialized! Run simon_gc_init() before doing anything else, please.");
			return young->allocate(n);
		}
		
		void collect()
		{
			std::cout << "COLLECTING LOL" << endl;
		}
		
		bool walkObjects(ObjectHeader** header, void** data)
		{
			return young->walkObjects(header, data);
		}
		
		virtual bool initializeCustomLowering(Module& m) { return false; }
		virtual bool performCustomLowering(Function& f) { return false; }
		void beginAssembly(std::ostream& os, AsmPrinter& AP, const TargetAsmInfo& TAI) {}
		void finishAssembly(std::ostream& os, AsmPrinter& AP, const TargetAsmInfo& TAI) {}
	};
	
	CollectorRegistry::Add<SimonCollector> X("simongc", "Simon's Generational Garbage Collector");
}

static SimonCollector collector;

/*
	C Interface Symbols
*/

#include "SimonGC.h"

extern "C"
{
	void simon_gc_init(size_t initial_heap_size)
	{
		collector.initialize(initial_heap_size);
	}
	
	void* simon_malloc(size_t n)
	{
		return collector.allocate(n);
	}
	
	void simon_free(void* ptr)
	{
	}
	
	void simon_gc()
	{
		collector.collect();
	}
	
	bool simon_walk_objects(ObjectHeader** header, void** data)
	{
		return collector.walkObjects(header, data);
	}
}

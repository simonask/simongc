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
	class VISIBILITY_HIDDEN SimonGCCompilerSupport : public Collector
	{
	public:
		SimonGCCompilerSupport() : young(NULL), elderly(NULL)
		{
			// CustomWriteBarriers = true; // Unnecessary?
			NeededSafePoints = 1 << GC::Loop | 1 << GC::Return | 1 << GC::PreCall | 1 << GC::PostCall;
			std::cout << "SimonCollector()" << std::endl;
		}
		
		void initialize(size_t initial_heap_size)
		{
			std::cout << "ObjectHeader size: " << sizeof(ObjectHeader) << std::endl;
			//young = new MemoryHeap(initial_heap_size);
		}
		
		void* allocate(size_t n)
		{
			assert(young != NULL && "GC not initialized! Run simon_gc_init() before doing anything else, please.");
			//return young->allocate(n);
		}
		
		void collect()
		{
			std::cout << "COLLECTING LOL" << endl;
		}
		
		bool walkObjects(ObjectHeader** header, void** data)
		{
			//return young->walkObjects(header, data);
		}

		virtual bool initializeCustomLowering(Module& m)
		{
			return false;
		}
		
		virtual bool performCustomLowering(Function& f)
		{
			return false;
		}
		
		void beginAssembly(std::ostream& os, AsmPrinter& AP, const TargetAsmInfo& TAI)
		{
		}
		
		void finishAssembly(std::ostream& os, AsmPrinter& AP, const TargetAsmInfo& TAI)
		{
		}

	};
	
	CollectorRegistry::Add<SimonGCCompilerSupport> X("simongc", "Simon's Generational Garbage Collector");
}

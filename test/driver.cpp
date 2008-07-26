/*
	The Chauffeur!
	This driver is needed, becase for some reason, lli doesn't have the GC
	libs linked in by default, so we need something that has, in order to
	test SimonGC.
*/

#include <iostream>
#include <vector>
#include <cstdio>
using namespace std;

#include <llvm/Module.h>
#include <llvm/ModuleProvider.h>
#include <llvm/Linker.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Function.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/System/DynamicLibrary.h>

using namespace llvm;

int main (int argc, char const *argv[])
{
	string error;
	if (sys::DynamicLibrary::LoadLibraryPermanently("../../../Release/lib/libSimonGC.dylib", &error))
	{
		std::cerr << "Could not load library: " << error << endl;
		exit(1);
	}
	
	
	const char* filename = (argc > 1 ? argv[1] : NULL);
	if (!filename)
	{
		std::cerr << "No filename given!" << endl;
		exit(1);
	}

	MemoryBuffer* buf = MemoryBuffer::getFile(filename, &error);
	if (!buf)
	{
		std::cerr << error << endl;
		exit(1);
	}
	
	Module* mod = ParseBitcodeFile(buf, &error);
	if (!mod)
	{
		std::cerr << error << endl;
		exit(1);
	}
	
	verifyModule(*mod, PrintMessageAction);
	
	ExecutionEngine* engine = ExecutionEngine::create(mod);
		
	Function* mod_main = mod->getFunction("main");
	if (mod_main)
	{
		int (*fp)(int, char const**) = (int (*)(int, char const**))engine->getPointerToFunction(mod_main);
		fp(argc, argv);
	}
	else
	{
		fprintf(stderr, "ERROR! Module '%s' does not define symbol 'main'!\n", filename);
		exit(1);
	}
	
	return 0;
}

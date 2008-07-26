#include <stdio.h>
#include "SimonGCRuntime.h"

int main (int argc, char const *argv[])
{
	simon_gc_initialize(0);
	
	char* lol = (char*)simon_gc_malloc(1024);
	StackFrame* frame = simon_gc_roots(1, &lol);
	
	size_t i;
	for (i = 0; i < 1024; ++i)
	{
		lol[i] = 'a' + (i % ('z' - 'a'));
	}
	printf("Yay: %s\n", lol);
	
	simon_gc_cleanup_roots(frame);
	
	return 0;
}
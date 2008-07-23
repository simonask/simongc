#include <stdio.h>
#include "SimonGC.h"

struct my_obj
{
	int n;
	float lol;
	char meh;
};

int main (int argc, char const *argv[])
{
	simon_gc_init(65535);
	
	char* str = (char*)simon_malloc(1024);
	char* str2 = (char*)simon_malloc(1024);
	
	printf("my_obj size: %d\n", sizeof(struct my_obj));
	struct my_obj* obj = (struct my_obj*)simon_malloc(sizeof(struct my_obj));
	
	struct my_obj* obj2 = (struct my_obj*)simon_malloc(sizeof(struct my_obj));
	
	ObjectHeader* header = NULL;
	void* data = NULL;
	while (simon_walk_objects(&header, &data))
	{
		printf("Object at 0x%x (header 0x%x), of size %d\n", data, header, header->size);
	}
	
	printf("test alignment: ");
	size_t mod = ((size_t)obj2 - (size_t)obj) % ALIGNMENT;
	if (mod)
	{
		printf("FAIL: obj2 is not aligned properly! ALIGNMENT = %d, obj = 0x%x, obj2 = 0x%x, (obj2 - obj) %% ALIGNMENT = %d\n", ALIGNMENT, obj, obj2, mod);
	}
	else
	{
		printf("OK\n");
	}
	
	simon_gc();
	return 0;
}

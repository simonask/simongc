all:
	g++ `llvm-config --ldflags --libs --cflags` -o driver driver.cpp 
	llvm-gcc -c --emit-llvm test.c -o test.bc -I..
	./driver test.bc
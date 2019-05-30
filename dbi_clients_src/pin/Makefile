COPT = -O3

all:	pin forkserver.so

all_32:	pin32 forkserver.so

all_32_debug:	pin32_debug forkserver.so

pin:
	make -f makefile.pin default

pin32:
	make -f makefile.pin default_x32

pin32_debug:
	make -f makefile.pin debug_x32

forkserver.so:	forkserver.c
	$(CC) $(COPT) -fpic -shared -o forkserver.so forkserver.c

clean:
	rm -rf obj* *~ *.so *.o

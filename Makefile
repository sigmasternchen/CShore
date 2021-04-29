CC = gcc
LD = gcc
CFLAGS = -Wall -g -std=c99 -ICFloor/src/ -ICson/src/ -D_POSIX_SOURCE
LDFLAGS = -lpthread -lrt

test: main.o router.o request.o test.o CFloor/libcfloor.a Cson/libcson.a
	$(LD) $(LDFLAGS) -o $@ $^

CFloor/libcfloor.a:
	$(MAKE) -C CFloor/ libcfloor.a
	
Cson/libcson.a:
	$(MAKE) -C Cson/ libcson.a

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
#main.o: main.c router.h
#router.o: router.c router.h
#test.o: controller.h
	
clean:
	rm -f *.o test
	$(MAKE) -C CFloor/ clean

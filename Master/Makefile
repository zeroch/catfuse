UNAME_S	:=	$(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	CCFLAGS	+=	-D	OSX
	CCFLAGS += -D_FILE_OFFSET_BITS = 64 -I/usr/local/include/osxfuse -L/usr/local/lib -lfuse -pthread
sys:
	@echo 'This is mac system'
else
	CCFLAGS += `pkg-config fuse --cflags --libs`
sys:
	@echo 'THIS IS LINUX'
endif

CCFLAGS +=	-DDEBUG
 
ifeq ($(MAKECMDGOALS),master)
	CCFLAGS +=	-DMASTER
else ifeq ($(MAKECMDGOALS),replica)
	CCFLAGS +=	-DREPLICA
endif



CC := gcc
master: hello.o md5.o testdb.o
	$(CC) -o master hello.o md5.o testdb.o $(CCFLAGS)

replica: hello.o md5.o testdb.o
	$(CC) -o replica hello.o md5.o testdb.o $(CCFLAGS)

hello.o: hello.c list.h md5.h testdb.h
	$(CC) -c -Wall hello.c  $(CCFLAGS)

md5.o: md5.c md5.h
	$(CC) -c -Wall md5.c

testdb.o: testdb.c testdb.h
	$(CC) -c -Wall testdb.c

clean:
	rm *.o master replica

test:
	@echo './hello ~/dev/tmp/cat -o defer_permissions'
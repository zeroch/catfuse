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

all:
	echo $(OPTION)
master: hello.o md5.o testdb.o filetransfer.o thread_server.o
	$(CC) -o master hello.o md5.o testdb.o filetransfer.o thread_server.o $(CCFLAGS)  -lcurl

replica: hello.o md5.o testdb.o filetransfer.o thread_server.o
	$(CC) -o replica hello.o md5.o testdb.o filetransfer.o thread_server.o $(CCFLAGS)  -lcurl

hello.o: hello.c list.h md5.h testdb.h filetransfer.h thread_server.h
	$(CC) -c -Wall hello.c  $(CCFLAGS) -lcurl

md5.o: md5.c md5.h
	$(CC) -c -Wall md5.c

testdb.o: testdb.c testdb.h
	$(CC) -c -Wall testdb.c

filetransfer.o: filetransfer.c filetransfer.h
	$(CC) -c -Wall filetransfer.c -lcurl  $(CCFLAGS)

thread_server.o: thread_server.c thread_server.h
	$(CC) -c -Wall thread_server.c  -lpthread  $(CCFLAGS)

clean:
	rm *.o master replica

test:
	@echo './hello ~/dev -d  -o nonempty'
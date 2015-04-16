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

CC := gcc
hello: hello.o
	$(CC) -o hello hello.o  $(CCFLAGS)

hello.o: hello.c list.h
	$(CC) -c -Wall hello.c  $(CCFLAGS)

clean:
	rm *.o hello

test:
	@echo './hello ~/dev/tmp/cat -o defer_permissions'

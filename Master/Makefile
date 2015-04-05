hello: hello.o
	gcc -o hello hello.o `pkg-config fuse --cflags --libs`

hello.o: hello.c list.h
	gcc -c -Wall hello.c `pkg-config fuse --cflags --libs`
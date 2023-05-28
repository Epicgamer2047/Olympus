main: main.c
	gcc main.c -o main -lncurses
	./main
server: server.c
	gcc -g server.c -o server
	./server
test: test.c
	gcc test.c -o test
	./test
cli: cli.c
	gcc cli.c -o cli
	./cli

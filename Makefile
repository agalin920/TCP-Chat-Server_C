default: program

program:
	gcc -o cliente cliente.c
	gcc -o server server.c -lpthread

clean:
	-rm -f cliente
	-rm -f server
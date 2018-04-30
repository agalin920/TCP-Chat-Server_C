default: program

program:
	gcc -o cliente cliente.c -lpthread
	gcc -o server server.c 

clean:
	-rm -f cliente
	-rm -f server

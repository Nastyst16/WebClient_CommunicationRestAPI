CC=gcc -g
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c parson.c requests.h
	$(CC) -o client client.c requests.c helpers.c buffer.c parson.c -Wall

run: client
	./client

clean:
	rm -f *.o client

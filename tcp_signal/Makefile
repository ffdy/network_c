server: server.c
	gcc server.c -o server
server-debug: server.c
	gcc server.c -o server -g
client: client.c
	gcc client.c -o client
client-debug: client.c
	gcc client.c -o client -g
all: server client
debug: server-debug client-debug

clean:
	rm server client
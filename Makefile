all: server client
debug: server.c client.c
	gcc server.c list.c -o server -g
	gcc client.c -o client -g
server: server.c list.c
	gcc server.c list.c -o server
client: client.c
	gcc client.c -o client

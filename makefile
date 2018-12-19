objects = tcp_net_server.o tcp_net_socket.o look_up.o

all: server

server: $(objects) 
	gcc -o server $(objects)
	rm $(objects)
tcp_net_server.o: look_up.h tcp_net_socket.h
tcp_net_socket.o: tcp_net_socket.h
loo_up.o: look_up.h

.PHONY: clean
clean:
	rm server $(objects) 

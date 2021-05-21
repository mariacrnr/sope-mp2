all: s c

s: server.o lib.o delay.c delay.h linkedList.c linkedList.h server_aux.c server_aux.h
	gcc -Wall -DDELAY=0 -o s delay.c linkedList.c server_aux.c lib.o server.o -pthread

c: client.c common.h
	gcc -Wall -o c client.c -pthread

server.o: server.c common.h
	gcc -Wall -c -o server.o server.c

lib.o: lib.c lib.h delay.h
	gcc -Wall -c -o lib.o lib.c

clean:
	rm -f s c server.o lib.o

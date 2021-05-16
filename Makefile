# CC = gcc
# CFLAGS =  -Wall -Wextra 

# PTHREAD = -pthread

# SDIR   = ./src
# OUTDIR  = ./out
# IDIR = ./include

# SERVER =s
# CLIENT =c

# all : $(SERVER) $(CLIENT)

# $(SERVER):  server.o lib.o $(SDIR)/delay.c $(IDIR)/delay.h
# 	$(CC) $(CFLAGS) -DDELAY=100 $(SDIR)/delay.c lib.o server.o -o s $(PTHREAD)

# $(CLIENT):  $(IDIR)/common.h $(SDIR)/client.c
# 	$(CC) $(CFLAGS) $(SDIR)/client.c -o c $(PTHREAD)

# server.o: $(SDIR)/server.c $(IDIR)/common.h 
# 	$(CC) $(CFLAGS) $(SDIR)/server.c -o server.o

# lib.o: $(SDIR)/lib.c $(IDIR)/lib.h $(IDIR)/delay.h
# 	$(CC) $(CFLAGS) $(SDIR)/lib.c -o lib.o

# clean :
# 	rm -f $(SERVER) $(CLIENT)

all: s c

s: server.o lib.o delay.c delay.h linkedList.c linkedList.h
	gcc -Wall -DDELAY=100 -o s delay.c linkedList.c lib.o server.o -pthread

c: client.c common.h
	gcc -Wall -o c client.c -pthread

server.o: server.c common.h
	gcc -Wall -c -o server.o server.c

lib.o: lib.c lib.h delay.h
	gcc -Wall -c -o lib.o lib.c

clean:
	rm -f s c server.o lib.o
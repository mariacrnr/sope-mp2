CC = gcc
CFLAGS =  -Wall -Wextra 

PTHREAD = -pthread

SDIR   = ./src
OUTDIR  = ./out
IDIR = ./include

SERVER =s
CLIENT =c

all : $(SERVER) $(CLIENT)

$(SERVER):  $(OUTDIR)/server.o $(OUTDIR)/lib.o $(SDIR)/delay.c $(IDIR)/delay.h
	$(CC) $(CFLAGS) -DDELAY=100 $(SDIR)/delay.c $(OUTDIR)/lib.o $(OUTDIR)/server.o -o s $(PTHREAD)

$(CLIENT):  $(IDIR)/common.h $(SDIR)/*.c
	$(CC) $(CFLAGS) $(SDIR)/*.c -o c $(PTHREAD)

clean :
	rm -f $(SERVER) $(CLIENT)
CC = gcc
CFLAGS =  -Wall -Wextra 

SDIR   = ./src
OUTDIR  = ./out

EXEC =xmod

all : $(EXEC)

$(EXEC):  $(OUTDIR)/xmod.o $(OUTDIR)/xmod_aux.o
	$(CC) $(CFLAGS) $(OUTDIR)/xmod.o $(OUTDIR)/xmod_aux.o -o xmod

$(OUTDIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@

clean :
	rm -f $(OUTDIR)/xmod.o $(OUTDIR)/xmod_aux.o xmod

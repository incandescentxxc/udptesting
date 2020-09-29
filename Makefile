# IDIR =../include
CC=gcc
CFLAGS=-I.

ODIR=obj
LDIR =../lib

DEPS = iperf_time.h

_OBJ = udpClient.o udpServer.o iperf_time.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
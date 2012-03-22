CFLAGS = -Wall -I/usr/include/glib-2.0/ -I/usr/lib/glib-2.0/include/
LDFLAGS = -lglib-2.0
EXE = tgio
OBJ = tgio.o

all: $(EXE)

$(EXE): %: %.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(EXE) $(OBJ)

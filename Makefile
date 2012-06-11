CFLAGS = -O0 -Wall -Wextra -g3 -ggdb `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`
EXE = tgio
OBJ = tgio.o

all: $(EXE)

$(EXE): %: %.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(EXE) $(OBJ)

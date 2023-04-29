PROG = file-manager
CFLAGS = -Wall -g -ansi -pedantic
LDFLAGS = -lncurses -lm
OBJMODULES = file_manager.o fm_view.o file.o directory.o list_of_files.o

.PHONY: clean

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm *.o $(PROG)

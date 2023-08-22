PROG = file-manager
CFLAGS = -Wall -g
LDFLAGS = -lncurses -lform -lm
OBJMODULES = file_manager.o fm_view.o file.o directory.o list_of_files.o

.PHONY: clean

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm *.o $(PROG)

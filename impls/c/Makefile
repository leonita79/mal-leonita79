STEPS = step0_repl step1_read_print
FILES = reader.o printer.o types.o linenoise/linenoise.o
HEADERS = reader.h printer.h types.h linenoise/linenoise.h
$(STEPS): %: %.o $(FILES) $(HEADERS)
	$(CC) $@.o $(FILES) -o $@ $(LDFLAGS) 

%.o: %.c $(HEADERS)


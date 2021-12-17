STEPS = step0_repl
FILES = linenoise/linenoise.o
HEADERS = linenoise/linenoise.h
$(STEPS): %: %.o $(FILES)
	$(CC) $@.o $(FILES) -o $@ $(LDFLAGS) 

%.o: %.c $(HEADERS)


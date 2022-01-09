STEPS = step1_read_print step2_eval step3_env
SOURCES = reader.c printer.c types.c env.c linenoise/linenoise.c
OBJS = reader.o printer.o types.o env.o linenoise/linenoise.o
HEADERS = reader.h printer.h types.h env.h linenoise/linenoise.h
CFLAGS = -g

.PHONY: all
all: step0_repl ${STEPS}
step0_repl: step0_repl.o linenoise/linenoise.o
	${CC} step0_repl.o linenoise/linenoise.o -o step0_repl $(LDFLAGS)
$(STEPS): %: %.o $(SOURCES) $(OBJS) $(HEADERS)
	$(CC) $@.o $(OBJS) -o $@ $(LDFLAGS) 

%.o: %.c $(HEADERS)


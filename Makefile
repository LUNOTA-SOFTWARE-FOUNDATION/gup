CFILES = $(shell find . -name "*.c")
DFILES = $(CFILES:.c=.d)
OFILES = $(CFILES:.c=.o)

CFLAGS = -Wall -pedantic -MMD -Iinc/
CC = gcc

.PHONY: all
all: $(OFILES)
	$(CC) $(OFILES) -o gup

-include $(DFILES)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OFILES) $(DFILES)

include mk/defaults.mk

CFILES = $(shell find . -name "*.c" | grep -v "arch")
CFILES += src/arch/$(ARCH).c
DFILES = $(CFILES:.c=.d)
OFILES = $(CFILES:.c=.o)

.PHONY: all
all: $(OFILES)
	$(CC) $(OFILES) -o gup

-include $(DFILES)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OFILES) $(DFILES)

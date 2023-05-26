SRCS=$(wildcard *.c)
TARGETS=$(SRCS:.c=)
CFLAGS= -Wall -fshort-wchar

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(TARGETS)

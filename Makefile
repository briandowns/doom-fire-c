CC     = cc
BINARY = doomfire

$(BINARY):
	$(CC) -std=c99 -Wall -o $(BINARY) main.c

.PHONY: clean
clean:
	rm -f $(BINARY)

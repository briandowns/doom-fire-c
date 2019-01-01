CC     = cc
BINARY = doomfire

$(BINARY):
	$(CC) -o $(BINARY) main.c

.PHONY: clean
clean:
	rm -f $(BINARY)

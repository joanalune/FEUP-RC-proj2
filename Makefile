CC = gcc
CFLAGS = -Wall

SRC = src/
INCLUDE = include/
BIN = bin/

.PHONY: download
download: download.c
	$(CC) $(CFLAGS) -o download $^


.PHONY: clean
clean:
	rm -rf download
# makefile for code examples in the "Inline_Assembly" directory

CC=gcc
WARN=-Wall

all: pow syntaxbug

pow: pow.c
	$(CC) $(WARN) -o pow pow.c

syntaxbug: syntaxbug.c
	$(CC) $(WARN) -o syntaxbug syntaxbug.c

clean:
	rm pow syntaxbug


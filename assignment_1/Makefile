
ASSN = 1
CLASS= cs143
CLASSDIR= ../..
SRC= stack.cl README 
LSRC= Makefile atoi.cl stack.test

CC=gcc
CFLAGS=-g

.c.o:
	${CC} ${CFLAGS} -c $<

compile: stack.s

stack.s: stack.cl atoi.cl
	coolc stack.cl atoi.cl 

test:	compile
	@echo stack.test
	spim -file stack.s < stack.test

clean :
	rm -f *.s core *~

all:	clean	source

source:	source.c
	gcc -g -Wall -o OS source.c -lpthread

clean:
	$(RM) source

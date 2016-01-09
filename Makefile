
all: lyrebird

lyrebird: decrypt.o memwatch.o lyrebird.o
	gcc decrypt.o memwatch.o lyrebird.o -g -Wall -lm -o lyrebird

decrypt.o: decrypt.c
	gcc -c decrypt.c -g -Wall -DMEMWATCH -DMW_STDIO -std=c99 -lm -o decrypt.o

memwatch.o: memwatch.c
	gcc -c memwatch.c -g -Wall -std=c99 -o memwatch.o

lyrebird.o: lyrebird.c
	gcc -c lyrebird.c -g -Wall -DMEMWATCH -DMW_STDIO -std=c99 -o lyrebird.o

clean:
	rm -f *.o *.core lyrebird
	rm -f memwatch.log
	
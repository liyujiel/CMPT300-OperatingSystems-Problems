
all: lyrebird

lyrebird: memwatch.c memwatch.h lyrebird.c
	gcc memwatch.c lyrebird.c -DMEMWATCH -DMW_STDIO -lm -std=gnu99 -o lyrebird

clean:
	rm -f *.o lyrebird
	rm -f memwatch.log
	
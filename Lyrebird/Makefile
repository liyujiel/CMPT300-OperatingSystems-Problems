# Name: 				Junyuan Tan
# Student ID:			301251535
# SFU Username: 		junyuant
# Lecture Session: 		CMPT 300 D100
# Instructor: 			Brian Booth
# TA: 					Scott Kristjanson



all: lyrebird.server lyrebird.client


lyrebird.server: decrypt.o memwatch.o lyrebird.server.o helper.o
	gcc decrypt.o memwatch.o lyrebird.server.o helper.o -g -Wall -lm -o lyrebird.server


lyrebird.client: decrypt.o memwatch.o lyrebird.client.o helper.o
	gcc decrypt.o memwatch.o lyrebird.client.o helper.o -g -Wall -lm -o lyrebird.client


helper.o: helper.c
	gcc -c helper.c -g -Wall -std=c99 -o helper.o


decrypt.o: decrypt.c
	gcc -c decrypt.c -g -Wall -DMEMWATCH -DMW_STDIO -std=c99 -lm -o decrypt.o


memwatch.o: memwatch.c
	gcc -c memwatch.c -g -Wall -std=c99 -o memwatch.o


lyrebird.server.o: lyrebird.server.c
	gcc -c lyrebird.server.c -g -Wall -DMEMWATCH -DMW_STDIO -std=c99 -o lyrebird.server.o

lyrebird.client.o: lyrebird.client.c
	gcc -c lyrebird.client.c -g -Wall -DMEMWATCH -DMW_STDIO -std=c99 -o lyrebird.client.o

clean:
	rm -f *.o *.core lyrebird.server lyrebird.client
	rm -f memwatch.log
	
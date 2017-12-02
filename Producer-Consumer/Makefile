TARGET_NAME = pcmodel
CCARGS = -Wall -Wextra

all:
	gcc -c $(CCARGS) helpers.c
	gcc -c $(CCARGS) queue.c
	gcc -c $(CCARGS) running_status.c
	gcc -c $(CCARGS) workers.c
	gcc -c $(CCARGS) pcmodel.c
	gcc -pthread *.o -o $(TARGET_NAME)
	rm -r *.o

run:
	./$(TARGET_NAME)

clean:
	rm -rf $(TARGET_NAME) *.o

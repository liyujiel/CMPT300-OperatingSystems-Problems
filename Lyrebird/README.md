# Lyrebird

## Description
This program decrypts plain text tweets and saves the output into a file which is specified in console. In the lastest update, you can use the client and server to form a distributed system to perform decryption. The server reads config file to find any text filename that needs to be decrypted and sends the filename to a client. Decryption is done in a client.


## Build
memwatch.c and memwatch.h is needed for compilation. You can use "make" or "make all"  to build the executable file.


## Run
Command for this program is the following:

    ./lyrebird <config filename> <log file>

where the "<config filename>" is the location that contains the configuration file, which contains one input and output file location each line. For more details, refer to the configuration file section.


## Configuration file
The format for one single input tweet and output location need to the following format:

	<input filename> <output filename>

The input file and output file location is seperated by one space. ""NO SPACE AFTER OUTPUT FILENAME"". Otherwise that tweet may not be decrypted successfully due to invalid argument.

	
## Known issues
	
Deadlock may persist when multiple clients are communicating with the server.

## Acknowledgement

1. https://forums.techguy.org/threads/solved-assign-numbers-to-letters.967404/

	I got an idea of how to match an integer to a character and vice versa. Basically, one of the menber in this website suggested that we can use an array to store all the special characters that I want to map in the correct index order that I want to return. When the functions runs, there is a loop that will iterate the array and in each iteration, it checks whether the current character is the correct one. If true, it will return the index, which is the integer that I want to map to.


2. Exponention by Squaring

	For this part, I consulted Wikipedia, and I used an algorithm provided on the "Exponention by Squaring" page of Wikipedia.

3. Use of select() function
	http://jhshi.me/2013/11/02/use-select-to-monitor-multiple-file-descriptors/
	
	This blog briefly describes how to use the select() function to monitor any changes made to a file descriptor. The original post is using select() to monitor a socket. And I got an idea, similarly, how to use it in pipes.

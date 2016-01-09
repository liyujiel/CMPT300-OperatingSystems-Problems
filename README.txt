Name: 				Peter Tan
Student ID:			301251535
SFU Username: 		junyuant
Lecture Session: 	CMPT 300 D100
Instructor: 		Brian Booth
TA: 				Scott Kristjanson


Description:
	The program, lyrebird, decrypts plain text tweets and saves the output into a file you specified in console.


Build:
	memwatch.c and memwatch.h is needed for compilation. You can use "make" or "make all"  to build the executable file.


Run:
	Usage of this program is the following:

					$  ./lyrebird <input filename> <output filename>

	Where the "<input filename>" is the filename that contains the encrypted text and you want the program to decrypt while the "<output filename>" is the filename taht you want the output text to be saved to.


Acknowledgement:

1. https://forums.techguy.org/threads/solved-assign-numbers-to-letters.967404/

	I got an idea of how to match an integer to a character and vice versa. Basically, one of the menber in this website suggested that we can use an array to store all the special characters that I want to map in the correct index order that I want to return. When the functions runs, there is a loop that will iterate the array and in each iteration, it checks whether the current character is the correct one. If true, it will return the index, which is the integer that I want to map to.


2. Exponention by Squaring

	For this part, I consulted Wikipedia, and I used an algorithm provided on the "Exponention by Squaring" page of Wikipedia.
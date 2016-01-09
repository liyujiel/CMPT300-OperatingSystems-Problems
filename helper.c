/* Name: 				Junyuan Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */


#include <time.h>		// For ctime()
#include <stdbool.h>	// Enabling boolean variables in C99
#include <sys/socket.h> // sockaddr type
#include <unistd.h>		// socket read() write()
#include <stdio.h>		// fprintf()
#include <arpa/inet.h>  // inet_ntoa()
#include <string.h>

#include "helper.h"
#include "decrypt.h"

#define DEBUG

int getInOutPath(const char *configLine, char *input, char *output)
{

	bool quote = false;		// Used to detect if double quotes in a filename is used
	bool spaceHit = false;	// This variable toggles when space is hit.
							// If space is hit inside double quotes, then this space is part of filename

	int countSpace = 0;		// Used to detect how many arguments are passed in.
							// Only allow 2 arguments, no more no less.

	int inputInd = 0;		// Counter variables to keep track of what index the new character should append to.
	int outputInd = 0;

	for(int i = 0; i < MAX_CONFIGLINE_SIZE && configLine[i] != '\0' && countSpace <= 1; i++) {
		if(configLine[i] == 34) {	// Check if quote in file location is used
			quote = !quote;	// If quote detected, toggle the boolean variable
			continue;
		}

		if( configLine[i] == 32 && !quote ) { // The space is not inside the quote
			spaceHit = true;
			quote = false;
			countSpace++;
			continue;
		}

		// Recording input and output locations.
		// A space hit (outside double quotes) specifies the end of inpput file path
		if(!spaceHit) {
			input[inputInd] = configLine[i];
			inputInd++;
		} else {
			output[outputInd] = configLine[i];
			outputInd++;
		}
	}

	if(countSpace != 1)
		return -1;
	else if(input[0] == '\0' || output[0] == '\0')
		return -1;

	input[inputInd] = '\0';		// Adds NULL character after the string for safety
	if(output[outputInd - 1] == '\n')
		output[outputInd - 1] = '\0';

	return 0;
}


// This function is to calculate the current time
// PRE: No precondition
// POST: Returns a pointer to a character array
char *getCurrTime()
{
	time_t currTime = time(NULL);
	char *adjustedTime = ctime(&currTime);
	adjustedTime[strlen(adjustedTime) - 1] = '\0';	// Eliminating the newline character
	return adjustedTime;
}

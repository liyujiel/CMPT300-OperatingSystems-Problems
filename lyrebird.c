/* Name: 				Peter Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */


#include <stdio.h>
#include <stdlib.h>		// Used for file I/O  eg. fopen() and fclose()
#include <math.h>		// Used for pow() and abs()
#include <unistd.h>		// Used for fork() and  & getpid()
#include <sys/wait.h>	// For waitpid()
#include <sys/types.h>	// For waitpid()
#include <stdbool.h> 	// Enabling boolean variables in C99
#include <time.h>		// For ctime()
#include <string.h>		// For strlen()

#include "memwatch.h"

int toInt(char input);
char toChar(int input);
unsigned long long expmod(unsigned long long base);
char *getCurrTime();


int main(int argc, char* argv[])
{
	if(argc != 2) { // Checks for argument
		fprintf(stderr, "Invalid Argument: Exactly 1 argument needed\n");
		printf("Usage: lyrebird <config file>\n");
	} else {
		
		FILE *configFile = fopen(argv[1], "r");
		if( configFile == NULL ) { // File does not exist
			fprintf(stderr, "Error: config file does not exist.\n");
			exit(1);
		} else {

			char line[1024];
			int totalChildProcess = 0; // Used to count how many processes to wait

			// Read config file line by line. Exits when reaches EOF.
			while( fgets(line, 1024, configFile) != NULL )
			{
				char input[1024];
				char output[1024];

				bool quote = false;
				bool spaceHit = false;
				int inputInd = 0;
				int outputInd = 0;
				int countSpace = 0;

				for(int i = 0; i < 1024 && line[i] != '\0' && countSpace <= 1; i++) {
					if(line[i] == 34) {	// Check if quote in file location is used
						quote = !quote;	// If quote detected, toggle the boolean variable
						continue;
					}

					if( line[i] == 32 && !quote ) { // The space is not inside the quote
						spaceHit = true;
						quote = false;
						countSpace++;
						continue;
					}

					// Saving both input and output locations
					if(!spaceHit) {
						input[inputInd] = line[i];
						inputInd++;
					} else {
						output[outputInd] = line[i];
						outputInd++;
					}
				}

				if(countSpace != 1) {
					fprintf(stderr, "Invalid Argument: Exactly 2 argument expected for each line in config file.\n");
					continue;
				} else if(input[0] == '\0' || output[0] == '\0') {
					fprintf(stderr, "Invalid Argument: Exactly 2 argument expected for each line in config file.\n");
					continue;
				}

				input[inputInd] = '\0';		// Adds NULL character after the string for safety
				if(output[outputInd - 1] == '\n')
					output[outputInd - 1] = '\0';


				/* By this point, argument checking is done. */


				// Creating child process.
				pid_t pid = fork();
				totalChildProcess++;

				if(pid < 0) {
					fprintf(stderr, "Fork error!\n");
					continue;
				}
				else if(pid == 0) { // Child process

					FILE *inFile = fopen(input, "r");
					if( inFile == NULL ) { // File does not exist
						fprintf(stderr, "[%s] Process #%d | Error: input file does not exist.\n", getCurrTime(), getpid());
						exit(1);
					} else {
						FILE *outFile = fopen(output, "w");
						char inFileLine[166];
						int count = 1;

						while( fgets(line, 166, inFile) != NULL )
						{
							char delExtra[146];
							int delInd = 0;  // Counts how many characters are exactly in delExtra
							
							if(count != 1)  // To insert new line before a sentence except the first one
								fputs("\n", outFile);

							// STEP 1:
							// Create a new char array with the 8th character removed.
							for(int i = 0; line[i] != 0; i++)
								if(i % 8 != 7) {
									delExtra[delInd] = line[i];
									delInd++;
								}
							delExtra[delInd] = 0;

							// STEP 2:
							unsigned long long *numberValue = malloc( sizeof(unsigned long long) * (delInd / 6) );
							// The following performs a mapping to the character table and then calculates
							// the numerical result in base 41 in a group of 41.
							// eg. 6 characters that have numerical value 20 | 24 | 4 | 7 | 1 | 3 
							// will be calculated as 20 * 41^5 + 24 * 41^4 + ...

							for(int i = 0; i < delInd / 6; i++) { // Loop for # of groups of 6 characters
								int power = 5;
								numberValue[i] = 0; // Clean the memory
								for(int j = 0 + 6 * i; j < 6 + 6 * i; j++) {
									numberValue[i] += toInt(delExtra[j]) * pow(41, power); 
									power--;
								}
							}

							// STEP 3:
							for(int i = 0; i < delInd / 6; i++)
								numberValue[i] = expmod(numberValue[i]);

							// STEP 4:
							// Converts to individual base 41 integers
							int *outInt = malloc( sizeof(int) * delInd );
							for(int i = 0; i < delInd / 6; i++)
								for(int j = 5 + 6 * i; j >= 6 * i; j--) {
									outInt[j] = numberValue[i] % 41;
									numberValue[i] /= 41;
								}

							// This translates the integer value back to the character
							for(int i = 0; i < delInd - 1; i++)
								delExtra[i] = toChar(outInt[i]);

							delExtra[delInd - 1] = 0;
							fputs(delExtra, outFile);
						
							free(numberValue);
							free(outInt);
							count++;
						}
						fclose(inFile);
						fclose(outFile);
					}

					printf("[%s] Decryption of %s complete. Process ID #%d Exiting. \n", 
						getCurrTime(), input, getpid());
					
					exit(0);
				}
				else { // Parent Process
					printf("[%s] Child process ID #%d created to decrypt %s.\n", 
						getCurrTime(), pid, input);
				}

			}

			// Waiting Stage
			int childStatus;
			pid_t returnedPID = -1;

			for(int i = 0; i < totalChildProcess; ) {
				int oldPID = returnedPID;
				returnedPID = waitpid(WAIT_ANY, &childStatus, 0);
				
				if(returnedPID != oldPID)
					i++;
				
				if(childStatus != 0)
					fprintf(stderr, "[%s] Child process ID #%d did not terminate successfully. \n", 
						getCurrTime(), returnedPID);
			}

			fclose(configFile);

		}
	}
	return 0;
}


// Given a character, this function will produce the corresponding integer
// based on the table in the last page of the assignment description.
// PRE: Input should be a char
// POST: Returns a corresponding integer according to the table
int toInt(char input)
{
	if(input == 32)
		return 0;
	else if(input >= 97 && input <= 122)
		return input - 96;
	else {
		char charTable[14] = { 35, 46, 44, 39, 33, 63, 40,  // '#', '.', ',', ''', '!', '?', '('
							   41, 45, 58, 36, 47, 38, 92 };// ')', '-', ':', '$', '/', '&', '\'
		for(int i = 0; i < 14; i++)
			if(charTable[i] == input)
				return i + 27;
	}
}


// This function is an inversed one of the toInt(). Given an integer, 
// toChar() will return the corresponding ASCII value of the character.
// PRE: Input should be an integer
// POST: Returns an ASCII code of the character
char toChar(int input)
{
	if(input == 0)
		return 32; // Returning <space>
	else if(input >= 1 && input <= 26)
		return input + 96;
	else {
		char charTable[14] = { 35, 46, 44, 39, 33, 63, 40,
							   41, 45, 58, 36, 47, 38, 92 };
		return charTable[input - 27];
	}
}


// This function calculates the plain-text number accroding to the given
// function in the assignment description.
// PRE: The base number is required and is in "unsigned long long" type
// POST: Returns the decrypted plain-text number
unsigned long long expmod(unsigned long long base)
{
	unsigned long long result = 1;
	unsigned long long exponent = 1921821779;
	unsigned long long mod = 4294434817;
	while(exponent > 0) {
		if(exponent % 2 == 1)
			result = (result * base) % mod;
		base = (base * base) % mod;
		exponent /= 2;
	}
	return result;
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

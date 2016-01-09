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

#define DEBUG

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
							
							assigned = true;
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


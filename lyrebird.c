/* Name: 				Peter Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */


#include <stdio.h>
#include <stdlib.h>		// Used for file I/O  eg. fopen() and fclose()
#include <unistd.h>		// Used for fork() and getpid()
#include <sys/wait.h>	// For waitpid()
#include <sys/types.h>	// For waitpid()
#include <sys/select.h>	// For fd_set and select() function
#include <stdbool.h> 	// Enabling boolean variables in C99
#include <string.h>

#include "decrypt.h"
#include "memwatch.h"

#define DEBUG

int main(int argc, char* argv[])
{
	if(argc != 2) { // Checks for argument
		fprintf(stderr, "Invalid Argument: Exactly 1 argument needed\n");
		printf("Usage: lyrebird <config file>\n");
		exit(1);
	
	} else {
	
  /* ---------------------------- Preparation and environment set-up ----------------------------- */
  /* ------ Open the config file, choose scheduling mode and start creating child processes ------ */
  /* ------ Read pipe and start decryption in child processes ------------------------------------ */
		
		fflush(stdout);
		setbuf(stdout, NULL);

		FILE *configFile = fopen(argv[1], "r");
		if( configFile == NULL ) {
			fprintf(stderr, "Error: config file does not exist.\n");
			exit(1);
		
		} else {

			// Get first string of the config file for evaluating scheduling mode
			char getScheduling[15];
			fgets(getScheduling, 15, configFile);

			// scheduling = 0 means scheduling mode is "round robin"
			// scheduling = 1 means scheduling mode is "fcfs"
			// Values other that these will be treated at scheduling mode not known
			// and the program will exit will this error
			int scheduling;
			if( strcmp(getScheduling, "round robin\n") == 0 )
				scheduling = 0;
			else if( strcmp(getScheduling, "fcfs\n") == 0 )
				scheduling = 1;
			else {
				fprintf(stderr, "Error: Scheduling Mode not known.\n");
				exit(1);
			}


			// Determine how many child processes there will be according to cores of CPU
			int numProcess = sysconf(_SC_NPROCESSORS_ONLN) - 1;


			// Allocating memory for the dynamic fd used for creating pipe
			int **parentToChild = malloc( numProcess * sizeof(int*) );
			int **childToParent = malloc( numProcess * sizeof(int*) );
			
			for(int i = 0; i < numProcess; i++) {
				parentToChild[i] = malloc( 2 * sizeof(int) );
				childToParent[i] = malloc( 2 * sizeof(int) );

				pipe( parentToChild[i] );
				pipe( childToParent[i] );

				pid_t pid = fork();

				if(pid < 0) {
					fprintf(stderr, "Fork error!\n");
					continue;
				} 

				// Child process
				else if(pid == 0) { 

					for(int j = 0; j < numProcess && j != i; j++) {
						free(parentToChild[j]);
						free(childToParent[j]);
					}

					close(parentToChild[i][1]);
					close(childToParent[i][0]);

					write(childToParent[i][1], "Ready", 5);
				#ifdef DEBUG
					printf("[%s] [Child %d] Closed parentToChild[1] in child process...\n", getCurrTime(), i);
					printf("[%s] [Child %d] Closed childToParent[0] in child process...\n", getCurrTime(), i);
					printf("[%s] [Child %d] Sent Ready...\n", getCurrTime(), i);
				#endif

					char input[MAX_CONFIGLINE_SIZE];
					char output[MAX_CONFIGLINE_SIZE];

					// Keep reading input and output path from pipe.
					// When 0 bytes read, there's no more file to decrypt, so break the loop and exit process
					int readSize;
					do {

						readSize = read(parentToChild[i][0], input, 1024);
						readSize = read(parentToChild[i][0], output, 1024);

						// Pipe has been closed
						if(readSize == 0) {
							close( childToParent[i][1] );
							break;
						}

						#ifdef DEBUG
						printf("[%s] [Child %d] Input path: %s\n", getCurrTime(), i, input);
						printf("[%s] [Child %d] Output path: %s\n", getCurrTime(), i, output);
						#endif

						int decryptResult = decrypt(input, output);

						write(childToParent[i][1], "Ready", 5);

						// Decryption exited due to file does not exist
						if(decryptResult == 1)
							continue;
						// Decryption successful
						else if(decryptResult == 0) {
							printf("[%s] Process ID #%d decrypted %s successfully.\n", 
								getCurrTime(), getpid(), input);
							continue;
						}

					} while(1);

					free(parentToChild[i]);
					free(childToParent[i]);
					free(parentToChild);
					free(childToParent);
					
					exit(0);


				} else { // Parent process

					close(parentToChild[i][0]);
					close(childToParent[i][1]);

				#ifdef DEBUG
					printf("[%s] [Parent LOOP %d] Child process ID #%d created successfully.\n", 
						getCurrTime(), i, pid);
					printf("[%s] [Parent LOOP %d] Closed parentToChild[0] in parent process...\n", getCurrTime(), i);
					printf("[%s] [Parent LOOP %d] Closed childToParent[1] in child process...\n", getCurrTime(), i);
				#endif
				}
			}

  /* ------------------------ End of preparation and environment set-up ------------------------ */




  /* ------------------------------- Reading config file process ------------------------------- */
  /* ----------- Getting input and output directory. Argument and open file checking ----------- */

			char line[MAX_CONFIGLINE_SIZE];
			int totalLine = 0;

			// Read config file line by line. Exits when reaches EOF.
			while( fgets(line, MAX_CONFIGLINE_SIZE, configFile) != NULL )
			{
				// Arrays that store the input and output file location
				char input[MAX_CONFIGLINE_SIZE];
				char output[MAX_CONFIGLINE_SIZE];

				bool quote = false;		// Used to detect if double quotes in a filename is used
				bool spaceHit = false;	// This variable toggles when space is hit.
										// If space is hit inside double quotes, then this space is part of filename
				
				int countSpace = 0;		// Used to detect how many arguments are passed in.
										// Only allow 2 arguments, no more no less.
				
				int inputInd = 0;		// Counter variables to keep track of what index the new character should append to.
				int outputInd = 0;

				for(int i = 0; i < MAX_CONFIGLINE_SIZE && line[i] != '\0' && countSpace <= 1; i++) {
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

					// Recording input and output locations. 
					// A space hit (outside double quotes) specifies the end of inpput file path
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

  /* -------------------------- End of reading config file process -------------------------- */




  /* ----------------------------  Running scheduling Algorithms --------------------------- */

				// Round Robin scheduling mode
				if(scheduling == 0) {
					int writeSize;

					// Writing input and output path to child processes
					// "totalLine % numProcess" determines which process to decrypt the file by iterating file descriptor
					writeSize = write(parentToChild[totalLine % numProcess][1], input, 1024);
					#ifdef DEBUG
					printf("[Parent] Wrote input file: %d\n", writeSize);
					#endif

					writeSize = write(parentToChild[totalLine % numProcess][1], output, 1024);
					#ifdef DEBUG
					printf("[Parent] Wrote output file: %d\n", writeSize);
					#endif

				}

				// fcfs scheduling mode
				else if(scheduling == 1) {

					fd_set schedulingFd;
					FD_ZERO(&schedulingFd);
					for(int i = 0; i < numProcess; i++)
						FD_SET(childToParent[i][0], &schedulingFd);
					
					// The number is largest or the last pipe to parent
					select(childToParent[numProcess - 1][0] + 1, &schedulingFd, NULL, NULL, NULL);

					char dump[6]; // Used to keep "Ready" messages sent from children
					bool assigned = false;
					
					// assigned used as: everytime we get a file to decrypt, we scan through each pipe to see
					// if any process is idle and hand in the file path to the process. Once we find such a 
					// child process, we set "assigned" to true to get out of the loop.  
					for(int i = 0; i < numProcess && !assigned; i++) {
						if(FD_ISSET(childToParent[i][0], &schedulingFd)) {
							read(childToParent[i][0], dump, 5);
							int writeSize;

							// Writing input and output path to child processes
							// "totalLine % numProcess" determines which process to decrypt the file by iterating file descriptor
							writeSize = write(parentToChild[i][1], input, 1024);
							#ifdef DEBUG
							printf("[Parent] Wrote input file: %d\n", writeSize);
							#endif

							writeSize = write(parentToChild[i][1], output, 1024);
							#ifdef DEBUG
							printf("[Parent] Wrote output file: %d\n", writeSize);
							#endif
							
							assigned = true;
						}
					}

				}
				
  /* ----------------------------  End of scheduling Algorithms --------------------------- */

				totalLine++; // Used by round robin scheduling mode to determine which child process
							 // to decrypt the file.
			}

			// As all files have been assigned to child processes, the parent need to close all the pipes
			// that send messages to child processes
			for(int i = 0; i < numProcess; i++) {
				close( parentToChild[i][1] );
				free(parentToChild[i]);
			}


			// Waiting Stage
			int childStatus;
			pid_t returnedPID = -1;

			for(int i = 0; i < numProcess; ) {
				int oldPID = returnedPID;
				returnedPID = waitpid(-1, &childStatus, 0);
				
				#ifdef DEBUG
				printf("Returned PID: %d\n", returnedPID);
				#endif
				
				if(returnedPID != oldPID) {
					i++;
					#ifdef DEBUG
					printf("[%s] Child process ID #%d exited. \n", getCurrTime(), returnedPID);
					#endif
				}
				
				if(childStatus != 0)
					fprintf(stderr, "[%s] Child process ID #%d did not terminate successfully. \n", 
						getCurrTime(), returnedPID);
			}

			for(int i = 0; i < numProcess; i++)
				free( childToParent[i] );

			free(parentToChild);
			free(childToParent);

			fclose(configFile);

		}
	}
	return 0;
}


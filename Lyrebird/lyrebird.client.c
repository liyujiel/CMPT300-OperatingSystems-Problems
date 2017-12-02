/* Name: 				Junyuan Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */


#include <stdio.h>
#include <unistd.h>		// Used for fork() and getpid()
#include <sys/types.h>	// For socket
#include <sys/socket.h> // For socket
#include <sys/select.h>	// For fd_set and select() function
#include <stdbool.h> 	// Enabling boolean variables in C99
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "decrypt.h"
#include "helper.h"
#include "memwatch.h"

#define DEBUG

int main(int argc, char* argv[])
{
	if(argc != 3) { // Checks for argument
		fprintf(stderr, "Invalid Argument: Exactly 2 argument needed\n");
		printf("Usage: lyrebird.client <server address> <server port>\n");
		exit(1);

	} else {

  /* ---------------------------- Preparation and environment set-up ----------------------------- */

		char *server_addr = argv[1];
		char *server_port = argv[2];

		struct sockaddr_in sa;
		sa.sin_family = AF_INET;
		inet_pton( AF_INET, server_addr, &(sa.sin_addr) );
		sa.sin_port = htons( atoi(server_port) );

		// Creating socket
		int serverfd = socket(AF_INET, SOCK_STREAM, 0);
		if(serverfd < 0) {
			fprintf(stderr, "Error: opening socket failed.\n");
			exit(1);
		}

		// Connecting to the lyrebird server
		if( connect(serverfd, (struct sockaddr *)&sa, sizeof(struct sockaddr)) == -1 )
			perror("Error");


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
					if(readSize <= 0) {
						close( childToParent[i][1] );
						break;
					}

					#ifdef DEBUG
					printf("[%s] [Child %d] Input path: %s\n", getCurrTime(), i, input);
					printf("[%s] [Child %d] Output path: %s\n", getCurrTime(), i, output);
					#endif

					int decryptResult = decrypt(input, output);
					write(childToParent[i][1], "Ready", 5);

					char formattedStr[1100];
					memset(formattedStr, 0, 1100);
					// Decryption exited due to file does not exist
					if(decryptResult == 1) { // Input file doesn't exist
						sprintf(formattedStr, "has encountered an error: Unable to open file %s in proecss %d.\n",
							input, getpid());
						write(serverfd, formattedStr, 1100);
						continue;
					} else if(decryptResult == 2) { // Output file opened unsuccessfully
						sprintf(formattedStr, "has encountered an error: Output file %s did not open successfully in process %d.\n",
							output, getpid());
						write(serverfd, formattedStr, 1100);
						continue;
					}
					// Decryption successful
					else if(decryptResult == 0) {
						sprintf(formattedStr, "has successfully decrypted %s.\n", input);
						write(serverfd, formattedStr, 1100);
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

		write(serverfd, "Ready", 1100);

		fd_set serverfd_set, schedulingFd;
		do {

			char input[MAX_CONFIGLINE_SIZE];
			char output[MAX_CONFIGLINE_SIZE];

			memset(input, 0, MAX_CONFIGLINE_SIZE);
			memset(output, 0, MAX_CONFIGLINE_SIZE);

			FD_ZERO(&serverfd_set);
			FD_ZERO(&schedulingFd);

			FD_SET(serverfd, &serverfd_set);
			for(int i = 0; i < numProcess; i++)
				FD_SET(childToParent[i][0], &schedulingFd);


			select(serverfd + 1, &serverfd_set, NULL, NULL, NULL);
			if( FD_ISSET(serverfd, &serverfd_set) )
			{
				read(serverfd, input, 1024);

				// Server sends exit command to client
				if( strcmp(input, "Exit") == 0 )
					break;

				read(serverfd, output, 1024);

				// The number is largest or the last pipe to parent
				select(childToParent[numProcess - 1][0] + 1, &schedulingFd, NULL, NULL, NULL);

				char dump[6]; // Used to keep "Ready" messages sent from children
				bool assigned = false;

				// assigned used as: everytime we get a file to decrypt, we scan through each pipe to see
				// if any process is idle and hand in the file path to the process. Once we find such a
				// child process, we set "assigned" to true to get out of the loop.
				for(int i = 0; i < numProcess && !assigned; i++)
					if(FD_ISSET(childToParent[i][0], &schedulingFd)) {
						read(childToParent[i][0], dump, 5);

						write(serverfd, "Ready", 1100);

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
		} while(1);


/* ----------------------------  End of scheduling Algorithms --------------------------- */


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

		close(serverfd);

	}
	return 0;
}

/* Name: 				Junyuan Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */


#include <stdio.h>
#include <stdlib.h>		// Used for file I/O  eg. fopen() and fclose()
#include <unistd.h>		// socket read() write()
#include <sys/select.h>	// For fd_set and select() function
#include <sys/socket.h> // For socket
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <ifaddrs.h>	// ifaddrs struct

#include "decrypt.h"
#include "helper.h"
#include "memwatch.h"

#define DEBUG

int main(int argc, char* argv[])
{
	if(argc != 3) { // Checks for argument
		fprintf(stderr, "Invalid Argument: Exactly 2 argument needed\n");
		printf("Usage: lyrebird.server <config file> <log file>\n");
		exit(1);

	} else {

  /* ---------------------------- Preparation and environment set-up ----------------------------- */
  /* ------ Open the config file, choose scheduling mode and start creating child processes ------ */
  /* ------ Read pipe and start decryption in child processes ------------------------------------ */

		FILE *configFile = fopen(argv[1], "r");
		FILE *logFile = fopen(argv[2], "w");

		if( configFile == NULL ) {
			fprintf(stderr, "Error: config file does not exist.\n");
			exit(1);

		}
		else if( logFile == NULL ) {
			fprintf(stderr, "Error: log file opening failed.\n");
			exit(1);

		} else {

			// Creating socket
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0) {
				fprintf(stderr, "Error: opening socket failed.\n");
				exit(1);
			}

			// Gets the IP address structure of the current computer
			struct sockaddr *s_addr;
			struct ifaddrs *IPConfig, *IPConfig_temp;

			getifaddrs( &IPConfig );
			bool done = false;
			// Loop through IPConfig to find the correct structure then assign to pointer *s_addr
			for( IPConfig_temp = IPConfig; IPConfig_temp && !done; IPConfig_temp = IPConfig_temp->ifa_next ) {

				s_addr = IPConfig_temp->ifa_addr;

				// Find the IP address that belongs to AP_INET and also ignore "127.0.0.1"
				if( strcmp( inet_ntoa( ((struct sockaddr_in *)s_addr)->sin_addr ), "127.0.0.1") != 0 &&
					s_addr->sa_family == AF_INET )

						done = true;

			}
			freeifaddrs( IPConfig );

			// Bind the socket to address of current computer
			if( bind(sockfd, s_addr, sizeof(struct sockaddr)) == -1 )
				printf("Bind error!\n");

			// Start listening from the port
			if( listen(sockfd, LISTEN_BACKLOG) == -1 )
				printf("listen error!\n");

			// Updates the sockaddr structure to find out which port the computer is listening to
			struct sockaddr_in  s_in_addr;
			socklen_t len = sizeof(s_in_addr);
			getsockname( sockfd, (struct sockaddr *)&s_in_addr, &len );

			// Prints out info like address and port of the current lyrebird.server
			printf( "[%s] lyrebird.server: PID %d on host %s, port %d\n",
				getCurrTime(), getpid(), inet_ntoa(s_in_addr.sin_addr), ntohs(s_in_addr.sin_port) );


  /* ------------------------------- Reading config file process ------------------------------- */

			fd_set sockfd_set;
			// Setting select timeout for 10 ms.
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 200;

			int clientfd[100];
			char *client_addr[100];
			int clientIndex = 0;

			char line[MAX_CONFIGLINE_SIZE];

			// Read config file line by line. Exits when reaches EOF.
			while( fgets(line, MAX_CONFIGLINE_SIZE, configFile) != NULL )
			{
				// Arrays that store the input and output file location
				char input[MAX_CONFIGLINE_SIZE];
				char output[MAX_CONFIGLINE_SIZE];

				// Processing input and output file to prepare for writing to client.
				// If there is error in the config file, we skip the current iteration.
				if( getInOutPath(line, input, output) == -1 ) {
					fprintf(stderr, "Invalid Argument: Exactly 2 argument expected for each line in config file.\n");
					continue;
				}

				do {
					FD_ZERO(&sockfd_set);
					FD_SET(sockfd, &sockfd_set);
					select(sockfd + 1, &sockfd_set, NULL, NULL, &timeout);
					if( FD_ISSET(sockfd, &sockfd_set) ) {
						// Accepting connections from the Internet
						clientfd[clientIndex] = accept(sockfd, (struct sockaddr *)&s_in_addr, &len);

						if(clientfd[clientIndex] >= 0) {
							client_addr[clientIndex] = inet_ntoa(s_in_addr.sin_addr);
							// Print out client information
							fprintf(logFile, "[%s] Successfully connected to lyrebird client %s\n",
								getCurrTime(), client_addr[clientIndex] );
							clientIndex++;
						}
					}
				} while(clientIndex == 0);


				fd_set clientfd_set;
				FD_ZERO(&clientfd_set);
				for(int i = 0; i < clientIndex; i++)
					FD_SET(clientfd[i], &clientfd_set);


				select(clientfd[clientIndex - 1] + 1, &clientfd_set, NULL, NULL, &timeout);

				// check if there is a socket that is readable
				char client_message[1100];
				for(int i = 0; i < clientIndex; i++) {
					memset(client_message, 0, 1100);
					if( FD_ISSET(clientfd[i], &clientfd_set) ) {
						read(clientfd[i], client_message, 1100);
						if( strcmp(client_message, "Ready") != 0)
							fprintf(logFile, "[%s] The lyrebird client %s %s",
								getCurrTime(), client_addr[i], client_message);
					}
				}

				// check if there is a socket that is writable
				bool assigned = false;
				while(!assigned) {
					FD_ZERO(&clientfd_set);
					for(int i = 0; i < clientIndex; i++)
						FD_SET(clientfd[i], &clientfd_set);

					select(clientfd[clientIndex - 1] + 1, NULL, &clientfd_set, NULL, &timeout);

					for(int i = 0; i < clientIndex && !assigned ; i++)
						if( FD_ISSET(clientfd[i], &clientfd_set) ) {

							write(clientfd[i], input, 1024);
							write(clientfd[i], output, 1024);
							fprintf(logFile, "[%s] The lyrebird client %s has been given the task of decrypting %s.\n",
								getCurrTime(), client_addr[i], input);

							assigned = true;
						}
				}
			}
			printf("End of while loop.\n");
   /* ----------------------------  End of scheduling Algorithms --------------------------- */

			// Writes "Exit" command to child and once receiving "Exited" status that means a child has exited
			for(int i = 0; i < clientIndex ; i++)
				write(clientfd[i], "Exit", 4);

			// Read messages from client until EOF. When EOF, this indicates the client has closed
			// the socket and exited successfully.
			char client_message[1100];
			int remainClient = clientIndex;
			while(remainClient > 0)
				for(int i = 0; i < clientIndex; i++) {
					memset(client_message, 0, 1100);
					int readSize = read(clientfd[i], client_message, 1100);
					if( readSize == 0 )
						remainClient--;
					else if( strcmp(client_message, "Ready") != 0)
						fprintf(logFile, "[%s] The lyrebird client %s %s",
							getCurrTime(), client_addr[i], client_message);
				}

			for(int i = 0; i < clientIndex; i++)
				close(clientfd[i]);

			printf("[%s] lyrebird.server: PID %d completed its tasks and is exiting successfully.\n",
				getCurrTime(), getpid() );
		}
	}
	return 0;
}

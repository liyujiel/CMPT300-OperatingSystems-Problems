/* Name: 				Junyuan Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */

#include <sys/socket.h>   // sockaddr type

#ifndef HELPER_H
#define HELPER_H

#define		LISTEN_BACKLOG			100		// Defines the maximum connection that the lyrebird.server can wait for


int 	getInOutPath(const char *configFile, char *input, char *output);
char*	getCurrTime();

#endif

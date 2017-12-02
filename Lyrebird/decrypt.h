/* Name: 				Junyuan Tan
 * Student ID:			301251535
 * SFU Username: 		junyuant
 * Lecture Session: 	CMPT 300 D100
 * Instructor: 			Brian Booth
 * TA: 					Scott Kristjanson
 */


#ifndef DECRYPT_H
#define DECRYPT_H

#define		MAX_CONFIGLINE_SIZE		1024	// Maximum character of a line in config file
#define		MAX_TWEETLINE_SIZE		166		// Maximum character of a line in tweets file
#define		MAX_NOEXTRA_SIZE		146		// Maximum character of a line in tweets file after eliminating 8th characters

int					decrypt( char *input, char *output );
int 				toInt( char input );
char 				toChar( int input );
unsigned long long 	expmod( unsigned long long base );

#endif
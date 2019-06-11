/* Mathew McDade
 * CS344.400
 * Spring 2019
 * otp_enc: one time pad encryption client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "otp_utils.h"

int main(int argc, char *argv[])
{
	int socketFD;
	struct sockaddr_in serverAddress;
	char plaintext[MAX_MESSAGE_LENGTH];
	char key[MAX_MESSAGE_LENGTH];
	char ciphertext[MAX_MESSAGE_LENGTH];
    
	if (argc != 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); fflush(stderr); exit(EXIT_FAILURE); } // Check usage & args

	//Load plaintext and key files.
	loadfile(argv[1],plaintext);
	loadfile(argv[2],key);

	//Check file and key length match.
	if(strlen(plaintext)>strlen(key)) error("KEY not long enough");
	//Check file and key for invalid characters.
	validate(plaintext);
	validate(key);

	// Set up the server address struct.
	buildClientServerAddress(&serverAddress,argv[3]);
	// Set up the socket
	buildClientSocket(&socketFD);	
	// Connect to server
	connectoServer(&serverAddress,&socketFD);

	//Send client name to server.
	sendmessage("otp_enc\n",socketFD);
	//Receive ok to send from server.
	char confirmation[10];
	receivemessage(confirmation,socketFD);
	if(strstr(confirmation,"accept")==NULL) {
		fprintf(stderr,"Cannot connect to otp_dec_d from otp_enc.\n");
		fflush(stderr);
		exit(2);
	}

	//Send plaintext to server.
	sendmessage(plaintext,socketFD);
	//Send key to server.
	sendmessage(key,socketFD);
	//Receive ciphertext from server.
	receivemessage(ciphertext,socketFD);

	//Print ciphertext.
	printf("%s\n",ciphertext);
	fflush(stdout);
	
	close(socketFD); 
	return 0;
}

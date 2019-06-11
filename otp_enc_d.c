/* Mathew McDade
 * CS344.400
 * Spring 2019
 * otp_enc_d: One time pad encryption daemon.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include "otp_utils.h"

/* One time pad encryption from plaintext to ciphertext. */
// Function defined here as opposed to the utilities file as
// otp_enc_d is the only process which should be accessing otpEncrypt.
void otpEncrypt(char* message, char* key) {
	//change ' ' to '[' for ease of encryption.
	char* pos;
	while((pos = strchr(message,' '))) {
		*pos = '[';
	}
	while((pos = strchr(key,' '))) {
		*pos = '[';
	}
	for(int i=0;i<strlen(message);i++) {
		message[i] = (char)((((message[i]-'A')+(key[i]-'A'))%27)+'A');
	}
	//change '[' back to ' '.
	while((pos = strchr(message,'['))) {
		*pos = ' ';
	}
	//Don't have to worry about changing key back as it's not reused.
	//append a newline to the encrypted message.
	message[strlen(message)] = '\n';
}

int main(int argc, char *argv[])
{	
	int listenSocketFD, establishedConnectionFD;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); fflush(stderr); exit(EXIT_FAILURE); } // Check usage & args

	buildServerAddress(&serverAddress,argv[1]);
	buildListenSocket(&serverAddress,&listenSocketFD);
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//Begin server listening loop.
	int activeConnectionCount = 0;
	while(1) {
		//burn zombie forks to free active connection space.
		burnZombies(&activeConnectionCount);

		if(activeConnectionCount<6) {
			// Accept a connection, blocking if one is not available until one connects
			acceptConnection(&clientAddress,&listenSocketFD,&establishedConnectionFD);
			activeConnectionCount++;

			pid_t childPid = fork();
			switch(childPid) {
				case(-1): {
							error("Hull Breach!\n");
							break;
						}
				//in the child process: check for the correct client, get the plaintext, get the key.
				//encrypt plaintext to ciphertext, send back ciphertext to client.
				case(0): {
						    char client[256];
						    char plaintext[MAX_MESSAGE_LENGTH];
						    char key[MAX_MESSAGE_LENGTH];

						    //receive client name.
						    receivemessage(client,establishedConnectionFD);
						    //verify correct client.
						    verifyClient(client,"otp_enc",establishedConnectionFD);
						    //receive plaintext.
						    receivemessage(plaintext,establishedConnectionFD);
						    //receive key.
						    receivemessage(key,establishedConnectionFD);
						    //encrypt plaintext to ciphertext.
						    otpEncrypt(plaintext,key);
						    //send back encrypted text.
						    sendmessage(plaintext,establishedConnectionFD);
						    
						    close(establishedConnectionFD); // Close the existing socket which is connected to the client.
						    exit(0);
					    }
				default: {
						    close(establishedConnectionFD); // Close the existing socket which is connected to the client, not shared w child.
					    }
			}
		}
	}
	close(listenSocketFD); // Close the listening socket, not reached.
	return 0; 
}

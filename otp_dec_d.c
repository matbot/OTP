/* Mathew McDade
 * CS344.400
 * Spring 2019
 * otp_dec_d: One time pad decryption daemon.
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

/* One time pad decryption from ciphertext to plaintext. */
void otpDecrypt(char* cipher, char* key) {
	//change ' ' to '[' for encryption.
	char* pos;
	while((pos = strchr(cipher,' '))) {
		*pos = '[';
	}
	while((pos = strchr(key,' '))) {
		*pos = '[';
	}
	for(int i=0;i<strlen(cipher);i++) {
		int letter = cipher[i]-key[i];
		if(letter<0) {
			letter += 27;
		}
		cipher[i] = (char)(letter+'A');
	}
	//change '[' back to ' '.
	while((pos = strchr(cipher,'['))) {
		*pos = ' ';
	}
	cipher[strlen(cipher)] = '\n';
}

int main(int argc, char *argv[])
{	
	int listenSocketFD, establishedConnectionFD;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc != 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); fflush(stderr); exit(EXIT_FAILURE); } // Check usage & args

	// Set up the address struct for this process (the server).
	buildServerAddress(&serverAddress,argv[1]);
	// Set up the socket and enable the socket to begin listening.
	buildListenSocket(&serverAddress,&listenSocketFD);
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//Begin server listening loop.
	int activeConnectionCount = 0;
	while(1) {
		//burn zombie forks to free active connection space.
		burnZombies(&activeConnectionCount);

		if(activeConnectionCount<5) {
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
						    char key[MAX_MESSAGE_LENGTH];
						    char ciphertext[MAX_MESSAGE_LENGTH];

						    //receive client name.
						    receivemessage(client,establishedConnectionFD);
						    //verify correct client.
							verifyClient(client,"otp_dec",establishedConnectionFD);	
						    //receive plaintext.
						    receivemessage(ciphertext,establishedConnectionFD);
						    //receive key.
						    receivemessage(key,establishedConnectionFD);
						    //encrypt plaintext to ciphertext.
						    otpDecrypt(ciphertext,key);
						    //send back encrypted text.
						    sendmessage(ciphertext,establishedConnectionFD);
						    
						    close(establishedConnectionFD); // Close the existing socket which is connected to the client
						    exit(0);
					    }
				default: {
						    close(establishedConnectionFD); // Close the existing socket which is connected to the client.
					    }
			}
		}
	}
	close(listenSocketFD); // Close the listening socket, not reached.
	return 0; 
}

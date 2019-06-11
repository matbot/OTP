/* Mathew McDade
 * CS344.400
 * Spring 2019
 * otp_utils: a variety of helper functions that abstract away the more technical details
 * of the socket implementation from the client and server files.
 */
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

#include "otp_utils.h"

void error(const char *msg) { perror(msg); fflush(stderr); exit(1); } // Error function used for reporting issues

//Kill the zombies to free up room to accept new connections.
void burnZombies(int* activeConnectionCount) {
	int status;
	while(waitpid(-1,&status,WNOHANG)>0) {
		(*activeConnectionCount)--;
	}
}

//Set up the server address for the server using the command line argument.
void buildServerAddress(struct sockaddr_in* serverAddress,char* portArg) {
	int portNumber;
	memset((char*)serverAddress,'\0',sizeof(*serverAddress));
	portNumber = atoi(portArg);
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_port = htons(portNumber);
	serverAddress->sin_addr.s_addr = INADDR_ANY;
}

//Set up the server address for the client using the command line argument.
void buildClientServerAddress(struct sockaddr_in* serverAddress,char* portArg) {
	memset((char*)serverAddress,'\0',sizeof(*serverAddress));
	int portNumber = atoi(portArg);
	serverAddress->sin_port = htons(portNumber);
	serverAddress->sin_family = AF_INET;
	struct hostent* serverHostInfo = gethostbyname("localhost");
	if (serverHostInfo==NULL) { fprintf(stderr,"CLIENT: ERROR: no such host\n"); fflush(stderr); exit(2); }
	memcpy((char*)&serverAddress->sin_addr.s_addr,(char*)serverHostInfo->h_addr,serverHostInfo->h_length);
}

//Initialize and bind the server's listening socket.
void buildListenSocket(struct sockaddr_in* serverAddress,int* listenSocket) {
	*listenSocket = socket(AF_INET,SOCK_STREAM,0);
	if (*listenSocket < 0) error("ERROR opening socket");
	if (bind(*listenSocket,(struct sockaddr*)serverAddress,sizeof(*serverAddress)) < 0) error("ERROR on binding");
}

//Set up the socket for the client. Note use of SO_REUSEADDR to make port reuse easier.
void buildClientSocket(int* clientSocket) {
	*clientSocket = socket(AF_INET,SOCK_STREAM,0);
	if (*clientSocket < 0) error("CLIENT: ERROR opening socket");
	int optval = 1;
	setsockopt(*clientSocket,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
}

//Connect the client socket to the server.
void connectoServer(struct sockaddr_in* serverAddress,int* clientSocket) {
	if (connect(*clientSocket,(struct sockaddr*)serverAddress,sizeof(*serverAddress)) < 0)
		error("CLIENT: ERROR connecting to server");
}

//Accept the client connection and spin off a new socket file descriptor.
void acceptConnection(struct sockaddr_in* clientAddress,int* listenSocket,int* connectionFD) {
	socklen_t sizeofClientInfo = sizeof(*clientAddress);
	*connectionFD = accept(*listenSocket,(struct sockaddr*)clientAddress,&sizeofClientInfo);
	if (*connectionFD < 0) error("ERROR on accept");
}

//Make sure the correct client is connecting to the correct server.
void verifyClient(char* clientName,const char* serverName,int connectionFD) {
	if(strcmp(clientName,serverName)!=0) {
		sendmessage("deny\n",connectionFD);
		close(connectionFD);
		exit(2);
	}
	else {
		sendmessage("accept\n",connectionFD);
	}
}

//Send a message over the socket, works client->server and server->client.
void sendmessage(char* message, int connectionFD) {
	int charsWrit = 0;
	charsWrit = send(connectionFD, message, strlen(message), 0);
	if (charsWrit < 0) error("ERROR writing to socket");
	//Wait for a confirmation of receipt with a blocking recv. Prevents the next send from being stuffed into the recv buffer.
	char receipt[2];
	charsWrit = recv(connectionFD,receipt,2,0);
}

//Receive a message over the socket, works client->server and server->client.
void receivemessage(char* message, int connectionFD) {
	int charsRead = 0;
	char buffer[NETWORK_BUFFER_SIZE];
	memset(buffer,'\0',NETWORK_BUFFER_SIZE);
	do {
		charsRead = recv(connectionFD, buffer, NETWORK_BUFFER_SIZE-1, 0);
		if (charsRead < 0) error("ERROR reading from socket");
		strcat(message,buffer);
		memset(buffer,'\0',NETWORK_BUFFER_SIZE);
	} while(strstr(message,"\n")==NULL);	//loop until the newline special character shows up in the message.
	char* at = strchr(message,'\n');
	*at = '\0';
	//Send back a confirmation to acknowledge receipt of the message.
	char* confirm = "1\0";
	charsRead = send(connectionFD,confirm,2,0);
}

//Load up a file's contents into a variable.
void loadfile(char* file, char* destination) {
	char buffer[100000];
	memset(buffer,'\0',sizeof(buffer));
	FILE* ifile = fopen(file,"r");
	if(!ifile) {
		error("Error opening argument files.");
	}
	//Lock the file from being written while I'm reading it, nonexclusive.
	flock(fileno(ifile),LOCK_SH);
	//Only have to worry about getting one null terminated line.
	fgets(buffer,sizeof(buffer),ifile);
	sprintf(destination,"%s",buffer);
	flock(fileno(ifile),LOCK_UN);
	fclose(ifile);
}

//Make sure all characters in a string variable are valid for the program. 
void validate(char* string) {
	for(int i=0;i<strlen(string)-1;i++) {
		if((string[i]<'A' && string[i]!=' ') || string[i]>'Z') {
			error ("Invalid character in file");
		}
	}
}

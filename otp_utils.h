/* Mathew McDade
 * CS344.400
 * Spring 2019
 * otp_utils header file declares utility functions for use by the otp server deamons and the clients.
 */

#ifndef UTILS_H
#define UTILS_H

#define NETWORK_BUFFER_SIZE 256
#define MAX_MESSAGE_LENGTH 100000
#define h_addr h_addr_list[0]

void error(const char*);

void burnZombies(int*);

void buildServerAddress(struct sockaddr_in*,char*);

void buildClientServerAddress(struct sockaddr_in*,char*);

void buildListenSocket(struct sockaddr_in*,int*);

void buildClientSocket(int*);

void connectoServer(struct sockaddr_in*,int*);

void acceptConnection(struct sockaddr_in*,int*,int*);

void verifyClient(char*,const char*,int);

void sendmessage(char*,int);

void receivemessage(char*,int);

void loadfile(char*,char*);

void validate(char*);

#endif //UTILS_H

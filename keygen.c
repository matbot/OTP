/* Mathew McDade
 * CS344.400
 * Spring 2019
 * OTP: keygen
 * Generates a key of user defined length consisting of random characters from the 'lingua' variable defined below.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int main(int argc, char* argv[]) {
	if(argc != 2) {
		fprintf(stderr,"syntax: keygen length\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	char lingua[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int keyLength = atoi(argv[1]);
	char key[keyLength+1];
	key[keyLength] = '\0';

	srand(time(0));
	
	//Could also be done with simple char math, but would still require a loop to remove '[', so potatoes potatoes.
	for(int i=0;i<keyLength;i++) {
		int j = rand() % 27;
		key[i] = lingua[j];
	}

	printf("%s\n",key);

	return 0;
}

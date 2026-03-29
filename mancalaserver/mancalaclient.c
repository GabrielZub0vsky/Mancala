/* mancalaclient.c
 *
 * Connect to the server socket whose IP address is on the command line.
 * Write a message and exit.
 *
 * @author: Paul Haskell
 * @copyright: Paul Haskell. 2024.
 */


////
//// Include files
////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>


////
//// Constants and globals.
////
const int PORT = 12345;
int serverFd = -1;


////
//// Method definitions
////
void doConnect(const char* serverAddr) {
	// Create socket
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) { perror("socket() fail"); exit(1); }

	// Connect to remote server
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT); // TCP
	addr.sin_addr.s_addr = inet_addr(serverAddr);
	const socklen_t addrlen = sizeof(addr);
	int retval = connect(serverFd, (struct sockaddr*) &addr, addrlen);
	if (retval < 0) { perror("connect() fail."); exit(1); }
}


void doWrite(char* msg) {
	const int MsgLen = strlen(msg) + 1; // +1 for the '\0';
	const int retval = write(serverFd, msg, MsgLen);
	if (retval != MsgLen) { perror("write() fail."); exit(1); }
}


////
//// Main module
////
int main(int argc, char** argv) {
	// Verify cmdline args.
	if (argc < 2) { // if no command-line argument...
		fprintf(stderr, "usage: client ServerIPAddress");
		return 1;
	}

	// Connect to remote server.
	doConnect(argv[1]);

	// Write message to server.
	doWrite("Hi it's PUT_YOUR_NAME_HERE");

	// Clean up.
	close(serverFd);
	return 0;
}

/* server.c
 *
 * Listen for client socket connections,
 * accept them, write messages.
 *
 * @author: phaskell
 * @copyright: Paul Haskell. 2024.
 */

////
//// Include files
////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

////
//// Constants and globals
////
const int PORT = 12345;

////
//// Methods
////
int doListen(const char* serverAddr) {
	// Create socket
	const int listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd < 0) {
		perror("socket() fail");
		exit(1);
	}

	// Bind socket to this server's address
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(serverAddr);
	socklen_t len = sizeof(addr);

	// Bind address to listenFd.
	int retval = bind(listenFd,(struct sockaddr*) &addr, len);
	if (retval < 0) {
		printf("bind() fail with errno %d\n", errno);
		exit(1);
	}

	// Listen for a client, i.e. ready to accept.
	retval = listen(listenFd, 100);
	if (retval < 0) {
		perror("listen() failed.");
		exit(1);
	}

	// Return the listener socket fd.
	return listenFd;
}

int doAccept(int listenFd) {
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);

	// Accept connection to client. Can accept multiple connections.
	const int remoteFd = accept(listenFd, (struct sockaddr*) &addr, &len);
	if (remoteFd < 0) {
		perror("accept() fail");
		exit(1);
	}

	return remoteFd;
}


void doClose(int fd) {
	close(fd);
}

/**
 *
 */
void doRead(int fd, char* buf) {
	//printf("reading from %d\n", fd);

	char cmd_length[1];
	int read_len = read(fd, cmd_length, 1);
	if (read_len > 0) {
		int count = cmd_length[0];
		//printf("input has %d bytes\n", count);

		int read_cmd = read(fd, buf, count);
		if (read_cmd > 0) {
			buf[read_cmd] = '\0';

			printf("Server received: '%s' (%d bytes)\n", buf, strlen(buf));
		}
		else {
			printf("read command failed: %d (%d:%s)\n", read_cmd, errno, strerror(errno));
		}
	}
	else {
		printf("read byte count failed: %d (%d:%s)\n", read_len, errno, strerror(errno));
	}
}

/**
 *
 */
void doWrite(char* msg, int clientFd) {
	//printf("message %s has %d bytes\n", msg, strlen(msg));

	char msg2[257];
	msg2[0] = strlen(msg) + 1;
	strncpy(msg2+1, msg, 256);
	//printf("writing %s (%d bytes)\n", msg2, strlen(msg2));

	int bytes_to_write = strlen(msg2)+1;
	const int bytes_written = write(clientFd, msg2, bytes_to_write);
	if (bytes_written == bytes_to_write) {
		printf("Server wrote %d%s (%d bytes)\n", msg2[0], msg2+1, bytes_to_write);
	}
	else {
		printf("write failed: %d\n", bytes_written);
	}
}

////
//// Main module
////
int main(int argc, char** argv) {
	// Set up listening socket.
	const int listenFd = doListen("127.0.0.1");
	printf("Listening on port %d: %d\n", PORT, listenFd);

	while(1){
		// Accept a remote connection, read it, and close down.
		const int acceptFd = doAccept(listenFd);
		printf("Accepted a remote connection: %d\n", acceptFd);
																//   1  	 6							6		5				 3				  6				   2				6	   6				6				  6				  5					6	    3				6		6				  3				  4		  6		   3				2
		char* commands[80] = {"LOGIN", "NEWGAME", "OPP:2", "OPP:3", "PLAY", "PLAY", "OPP:1", "OPP:4", "PLAY", "PLAY", "OPP:5", "PLAY", "OPP:6", "PLAY", "OPP:1", "PLAY", "OPP:2", "PLAY", "PLAY", "OPP:3", "PLAY", "OPP:4", "PLAY", "OPP:1", "PLAY", "OPP:5", "PLAY", "PLAY", "OPP:6", "PLAY", "PLAY", "OPP:1", "PLAY", "OPP:3", "PLAY", "PLAY", "PLAY", "OPP:4", "PLAY", "OPP:5", "NEWGAME", "OPP:2", "OPP:3", "PLAY", "PLAY", "OPP:1", "OPP:4", "PLAY", "PLAY", "OPP:5", "PLAY", "OPP:6", "PLAY", "OPP:1", "PLAY", "OPP:2", "PLAY", "PLAY", "OPP:3", "PLAY", "OPP:4", "PLAY", "OPP:1", "PLAY", "OPP:5", "PLAY", "PLAY", "OPP:6", "PLAY", "PLAY", "OPP:1", "PLAY", "OPP:3", "PLAY", "PLAY", "PLAY", "OPP:4", "PLAY", "OPP:5", "DONE"};
		for (int i = 0; i < 80; i++) {
			doWrite(commands[i], acceptFd);
			char buf[1024];
			buf[0] = 0;
			if(strcmp(commands[i], "LOGIN") == 0 || strcmp(commands[i], "PLAY") == 0){
				doRead(acceptFd, buf);
			}
		}
		doClose(acceptFd);
		printf("Disconnected from %d\n", acceptFd);
	}

	// Clean up
	doClose(listenFd);
	return 0;
}

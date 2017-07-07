#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>

#define LENGTH 10


void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	/* Variable Definition */
	int sockfd; 
	int nsockfd;
	char revbuf[LENGTH]; 
	struct sockaddr_in remote_addr;

	/* Get the Socket file descriptor */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
		exit(1);
	}

	/* Fill the socket address struct */
	remote_addr.sin_family = AF_INET; 
	remote_addr.sin_port = htons(atoi(argv[4]));
	inet_pton(AF_INET, argv[3], &remote_addr.sin_addr); 
	bzero(&(remote_addr.sin_zero), 8);

	/* Try to connect the remote */
	if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
		exit(1);
	}
	else 
		printf("[Client] Connected to server at port %s...ok!\n", argv[3]);


	/*Send output filename to server  */
	char filenamebuffer[LENGTH];
	bzero(filenamebuffer, LENGTH);
	strcpy(filenamebuffer,argv[2]);
	if(send(sockfd, filenamebuffer, LENGTH, 0) < 0)
	    {
		        fprintf(stderr, "ERROR: Failed to send output filename %s. (errno = %d)\n", argv[2], errno);
		        return;
	    }
		else
		{
			printf("Output filename %s sent to server",argv[2]);
		}

	/* Send File to Server */
		char sdbuf[LENGTH]; 
		printf("[Client] Sending %s to the Server... ", argv[1]);
		FILE *fs = fopen(argv[1], "r");
		if(fs == NULL)
		{
			printf("ERROR: File %s not found.\n", argv[1]);
			exit(1);
		}

		bzero(sdbuf, LENGTH); 
		int fs_block_sz;
		while(!feof(fs))
		{ 
		while((fs_block_sz = fread(sdbuf, sizeof(char), 10, fs)) > 0)
		{   printf("%s",sdbuf);
		    if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
		    {
		        fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", argv[1], errno);
		        break;
		    }
		    bzero(sdbuf, LENGTH);
		}
		}
		printf("Ok File %s from Client was Sent!\n", argv[1]);
	


	close (sockfd);
	printf("[Client] Connection lost.\n");
	return (0);
}

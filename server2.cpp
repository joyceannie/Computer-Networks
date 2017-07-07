#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <iomanip>
using namespace std;

int checksum(string msgrcvd)	{

	int sum = 0;
	string chk, msg = msgrcvd;
	char xyz[4];
	chk = msgrcvd.substr(1, 4);
	msg.erase(0, 5);
	for(int i = 0; i < 11; i++)	{
		sum += msg[i];
	}

	sprintf(xyz, "%04d", sum);
	string chsum = string(xyz);
	if(strcmp(&chsum[0], &chk[0]) == 0)	
		return 0;
	else
		return 1;
}

int main (int argc, char **argv)
{
	int udpSocket, read_size = 0, fsize = 1, j, i, buf_size, count;	
	struct sockaddr_in server, client; 
	string send_buffer;
	char fname[25], order;
	char *file_size;
	socklen_t addr_size;
	struct sockaddr_storage serverStorage;
	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	unsigned int sock_len;
	ofstream fout;

	//Initializing a Socket
	cout << "Initializing a Socket\n";
	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Socket: ");
		close(udpSocket);
		exit(-1);
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(7000);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 0);

	sock_len = sizeof(client);

	//Binding a Socket
	cout << "Binding a Socket\n";
	if((bind(udpSocket, (struct sockaddr *)&server, sock_len)) == -1)
	{
		perror("Bind: ");
		close(udpSocket);
		exit(-1);
	}

	addr_size = sizeof serverStorage;
	
	//To accept connection and get data from server
	//Opening file.
	
	fout.open("output.txt", ios::out | ios::binary);
	if(!fout)	{
		perror("Fout : ");
		close(udpSocket);
		exit(1);
	}

	cout << "Waiting for message...\n";
	count = 0;
	while(1)
	{		
		//checking inactivity on server
		if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(struct timeval)) < 0)	{
			close(udpSocket);
			exit(1);
		}
		memset(fname, 0, sizeof(fname));
		//Getting data from client
		if ((recvfrom(udpSocket , fname, sizeof(fname), 0, (struct sockaddr *)&serverStorage, &addr_size)) == -1)	{
			cout << "Server too idle... Closing server\n";
			fout.close();
			close(udpSocket);
			exit(1);
		}
		else	{

			send_buffer = string(fname);
			send_buffer.erase(0, 5);
			order = fname[0];
			if(checksum(fname) == 0)	{

				
				fout.write(send_buffer.c_str(), send_buffer.length());
				cout << "Received Message from client: " << send_buffer << endl;
				cout << "Writing & Sending ACK\n";
				//Sending ACK.
				if ((sendto(udpSocket, &order, 1, 0, (struct sockaddr *)&serverStorage, addr_size)) == -1)	{
					fout.close();
					close(udpSocket);
					exit(1);
				}
				count++;
			}
		}				
	}
	fout.close();		//Closing file
	close(udpSocket);	//Closing server instance
	return 0;
}

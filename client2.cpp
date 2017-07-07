#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <netdb.h>
using namespace std;
string chk(string buf_msg, int order, int ran_chksm);
string chk(string buf_msg, int order, int ran_chksm)	{

	char xyz[4];
	int sum = 0;
	string chsum, temp;
	
	if (ran_chksm%15 == 0)	{
		sum = 0;
	}
	else	{
		for(int i = 0; i < 11; i++)	{
			sum += buf_msg[i];
		}	
	}
	sprintf(xyz, "%04d", sum);
	chsum = string(xyz);
	temp = to_string(order) + chsum + buf_msg;
	return temp;
	
}
int main (int argc, char *argv[])
{
	int sockfd, count, min, ran_chksm, success = 0, fail = 0, fsize;
	float eff;

	string buf_msg, checksum;
	char ack_msg[2], buf[15];
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	ifstream fin;

	// Checking input arguments
	if(argc != 3)	{
		perror("Credentials Incomplete... Try again");
		exit(1);
	}
	
	//Initiating the client socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)		{
		cout << "Socket Failure." << endl;
		exit(1);
	}

	//Assigning port to socket descriptor
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&serverAddr.sin_zero, 0);

	addr_size = sizeof serverAddr;


	//Opening input file
	cout << "Opening: input.txt\n";
	fin.open ("input.txt", ios::in | ios::app | ios::binary);
	if(!fin)	{
		cout << "Error opening input file\n";
		close(sockfd);
		exit(1);
	}
	fin.seekg(0, fin.end);
	fsize = fin.tellg();
	fin.seekg(0, fin.beg);
	count = 0;

	//Reading contents of file
	while(fin)	{

		memset(buf, 0, 10);
		if(fsize > 10)
			min = 10;
		else
			min = fsize;
		fin.read(buf, min);
		buf[min] = '\0';
		buf_msg = string(buf);
		
		ran_chksm = rand()%15;
		checksum = chk(buf_msg, count%2, ran_chksm);
		
		cout << "Original Data with checksum: " << checksum << endl;
		if ((sendto(sockfd, checksum.data(), checksum.length(), 0, (struct sockaddr *)&serverAddr, addr_size)) == -1)	{
			fin.close();
			perror("Error sending\n");
			exit(1);
		}
		//Receive ACK from server
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(struct timeval)) < 0)	{
			cout << "No ACK revcd... Resending\n";
		}
		memset(ack_msg, 0, 2);
		if (recvfrom(sockfd, ack_msg, 2, 0, NULL, NULL) == -1)	{
			if(fin.eof())
				fin.seekg(-(min+1), ios::end);
			else
				fin.seekg(-(min), ios::cur);
			fail++;
			cout << "No ACK rvcd... Resending\n";
		}
		else	{
			if((count % 2) != (atoi(ack_msg) % 2))	{
				if(fin.eof())
					fin.seekg(-(min+1), ios::end);
				else
					fin.seekg(-(min), ios::cur);
				cout << "ACK does not match... Resending\n";
			}
			else	{
			cout << "Sent successfully, sending next\n";
			success++;
			}
		}
		count++;
	}

	
	cout << "Successful Transfers: " << success << " Failed Transfers: " << fail << endl;

	cout << "Closing Client\n";
	fin.close();	//Closing file
	close(sockfd);	//Closing connection
	return 0;
}

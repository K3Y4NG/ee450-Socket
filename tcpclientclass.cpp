/*
tapclientclass.cpp
University of Southern California
EE-450 Final Project
NAN JIANG
http://www.slyar.com/
2014.11.8
*/

#ifndef __HEADER_H_DEFINE__
#define __HEADER_H_DEFINE__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#endif

using namespace std;

#define PORT 21309

class tcp_client
{
private:
	int clientSocket;
	int portNum;
	struct hostent *hostInfo;
	struct sockaddr_in server_address;
	void showErr(const char *err);
	int bufferSize;
public:
	tcp_client(string, string, int);
	int connToServer();
	void closeClientSocket();
	int sendData(string);
	int recvData();
	string recvString;
	string patientName;
};

tcp_client::tcp_client(string n, string h, int p)
{
	this->patientName = n;
	this->portNum = p;
	this->clientSocket = -1;
	this->bufferSize = 128;

	// create socket for tcp client
	this->clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	// get information about the server
	this->hostInfo = gethostbyname(h.c_str());

	memset((char *) &this->server_address, 0, sizeof(this->server_address));

	this->server_address.sin_family = AF_INET;
	memcpy(&this->server_address.sin_addr, this->hostInfo->h_addr, this->hostInfo->h_length);
	this->server_address.sin_port = htons(this->portNum);
}

int tcp_client::connToServer()
{
	if (connect(this->clientSocket, (struct sockaddr *) &this->server_address, sizeof(this->server_address)) < 0)
	{
		return -1;
	}
	// get IP address and port number of client itself
	struct sockaddr_in myInfo;
	socklen_t myInfoLength = sizeof(myInfo);
	getsockname(this->clientSocket, (struct sockaddr*) &myInfo, &myInfoLength);

	// OUTPUT
	printf("Phase 1: %s has TCP port number %d and IP address %s.\n", this->patientName.c_str(), ntohs (myInfo.sin_port), inet_ntoa (myInfo.sin_addr));
	return 1;
}

int tcp_client::recvData()
{
	int n;
	int recvLength = 0;
	if (read(this->clientSocket, &recvLength, sizeof(recvLength)) < 0)
	{
		showErr("Read Error");
		return -1;
	}
	recvLength = ntohl(recvLength);

	char buffer[this->bufferSize];
	memset(buffer, 0, this->bufferSize);
	if ((read(this->clientSocket, buffer, recvLength) < 0))
	{
		showErr("Read Error");
		return -1;
	}
	this->recvString = buffer;
	return 1;
}

int tcp_client::sendData(string str)
{
	int sendLength = htonl(str.size());

	if ((write(this->clientSocket, &sendLength, sizeof(sendLength))) < 0)
	{
		this->showErr("Send Error");
		return -1;
	}

	//cout << "String length is " << str.size() << endl;
	//cout << "sendLength is " << sendLength << endl;

	// char buffer[this->bufferSize];
	// memset(buffer, 0, this->bufferSize);
	// strncpy(buffer, str.c_str(), sizeof(str));
	//buffer[sizeof(buffer) - 1] = 0;

	int n =0;
	if ((n = write(this->clientSocket, str.c_str(), str.size())) < 0)
	{
		this->showErr("Send Error");
		return -1;
	}
	//cout << "send to buffer: " << n << endl;
	return 1;
}

void tcp_client::showErr(const char *err)
{
	perror(err);
	exit(1);
}

void tcp_client::closeClientSocket()
{
	close(this->clientSocket);
}

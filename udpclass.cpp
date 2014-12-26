/*
udpclass.cpp
University of Southern California
EE-450 Final Project
NAN JIANG
http://www.slyar.com/
2014.11.9
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
#include <string>
#include <vector>

#endif

using namespace std;

// define the hostname
const string HOSTNAME = "localhost";

class udp_socket_class
{
private:
	void showErr(const char *);
	int bufferSize;
	struct sockaddr_in peer_address;
	struct sockaddr_in local_address;
public:
	int udpSocket;
	char localIP[32];
	socklen_t localPort;
	char peerIP[32];
	socklen_t peerPort;
	string recvString;
	string clientName;
	udp_socket_class(string, int, string, int);
	void closeUdpSocket();
	int start();
	int sendData(string);
	int recvData();
};

int udp_socket_class::recvData()
{
	ssize_t n;
	socklen_t lengthOfAddress = sizeof(struct sockaddr);
	
	// receive the length of data
	int recvLength = 0;
	if (recvfrom(this->udpSocket, &recvLength, sizeof(recvLength), 0, (struct sockaddr *)&this->peer_address, &lengthOfAddress) < 0)
	{
		this->showErr("Receive Error");
		return -1;
	}
	recvLength = ntohl(recvLength);

	// get peer IP and port
	strcpy(this->peerIP, inet_ntoa(this->peer_address.sin_addr));
	this->peerPort = ntohs(this->peer_address.sin_port);

	// receive data
	char buffer[this->bufferSize];
	memset(buffer, 0, this->bufferSize);
	if ((recvfrom(this->udpSocket, buffer, recvLength, 0, (struct sockaddr *)&this->peer_address, &lengthOfAddress)) < 0)
	{
		this->showErr("Receive Error");
		return -1;
	}
	this->recvString = buffer;

	return 1;
}

int udp_socket_class::sendData(string str)
{
	ssize_t n;
	int sendLength = htonl(str.size());
	socklen_t lengthOfAddress = sizeof(struct sockaddr);

	// send the length of data
	if ((sendto(this->udpSocket, &sendLength, sizeof(sendLength), 0, (struct sockaddr *)&this->peer_address, lengthOfAddress)) < 0)
	{
		this->showErr("Send Error");
		return -1;
	}

	// send data
	// char buffer[this->bufferSize];
	// memset(buffer, 0, this->bufferSize);
	// strncpy(buffer, str.c_str(), sizeof(str));
	// buffer[sizeof(buffer) - 1] = 0;

	if ((sendto(this->udpSocket, str.c_str(), str.size(), 0, (struct sockaddr *)&this->peer_address, lengthOfAddress)) < 0)
	{
		this->showErr("Send Error");
		return -1;
	}
	return 1;
}

int udp_socket_class::start()
{
	this->udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket < 0)
	{
		showErr("Socket cannot create");
	}

	if (bind(this->udpSocket, (struct sockaddr *)&this->local_address, sizeof(struct sockaddr)) < 0)
	{
		showErr("Bind Error");
		return -1;
	}

	// update port number after udp socket been created
	struct sockaddr_in udpTemp;
	socklen_t udpTempLen = sizeof(udpTemp);
	getsockname(this->udpSocket, (struct sockaddr *)&udpTemp, &udpTempLen);
	strcpy(this->localIP, inet_ntoa(udpTemp.sin_addr));
	this->localPort = ntohs(udpTemp.sin_port);

	return 0;
}

udp_socket_class::udp_socket_class(string srcHost, int srcPort, string dstHost, int dstPort)
{
	this->bufferSize = 128;
	this->localPort = srcPort;
	this->peerPort = dstPort;

	// get UDP local IP address
	struct hostent *local;
	local = gethostbyname(srcHost.c_str());
	strcpy(this->localIP, inet_ntoa(*((struct in_addr *)local->h_addr)));

	memset((char *) &this->local_address, 0, sizeof(this->local_address));
	this->local_address.sin_family = AF_INET;
	this->local_address.sin_port = htons(srcPort);
	this->local_address.sin_addr = *((struct in_addr *)local->h_addr);

	// get UDP peer IP address
	struct hostent *peer;
	peer = gethostbyname(dstHost.c_str());
	strcpy(this->peerIP, inet_ntoa(*((struct in_addr *)peer->h_addr)));

	memset((char *) &this->peer_address, 0, sizeof(this->peer_address));
	this->peer_address.sin_family = AF_INET;
	this->peer_address.sin_port = htons(dstPort);
	this->peer_address.sin_addr = *((struct in_addr *)peer->h_addr);
}

void udp_socket_class::closeUdpSocket()
{
	close(this->udpSocket);
}

void udp_socket_class::showErr(const char *err)
{
	perror(err);
	exit(1);
}

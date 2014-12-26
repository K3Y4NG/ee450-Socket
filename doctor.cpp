/*
doctor.cpp
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
#include "udpclass.cpp"

#endif

using namespace std;

#define PORT1 41309
#define PORT2 42309

void doctorProcess(string, int);
void input();

struct insurance
{
	string docName;
	string insuranceName;
	string cost;
};

vector<insurance> insuranceCost;

/////////////////////
/// Main Function ///
/////////////////////

int main()
{
	//signal(SIGCHLD, SIG_IGN);
	
	input();

	// create two child processes
	pid_t p1 = fork();
	if (p1 == 0)
	{
		doctorProcess("Doctor 1", PORT1);
		return 0;
	}

	pid_t p2 = fork();
	if (p2 == 0)
	{
		doctorProcess("Doctor 2", PORT2);
		return 0;
	}

	// wait child processes exit
	waitpid(p1, NULL, 0);
	waitpid(p2, NULL, 0);

	return 0;
}

////////////////////////////////////
/// Implement of other functions ///
////////////////////////////////////

void doctorProcess(string docName, int port)
{
	int n;
	string recvInsurance;
	udp_socket_class server(HOSTNAME, port, HOSTNAME, INADDR_ANY);
	server.start();

	// OUTPUT
	printf("Phase 3: %s has a static UDP port %d and IP address %s.\n", docName.c_str(), server.localPort, server.localIP);

	// in case of two patients connect to the same doctor
	while(1)
	{
		// receive patient name
		server.recvData();
		server.clientName = server.recvString;

		// receive patient submited insurance plan
		server.recvData();
		recvInsurance = server.recvString;

		// OUTPUT
		printf("Phase 3: %s receives the request from the patient with port number %d and the insurance plan %s.\n", docName.c_str(), server.peerPort, recvInsurance.c_str());
		string cost;
		// search the insurance plan submited
		for (vector<insurance>::iterator it = insuranceCost.begin(); it != insuranceCost.end(); it++)
		{
			if ((it->docName == docName) && (it->insuranceName == recvInsurance))
			{
				cost = it->cost;
				break;
			}
		}
		// send cost and dotor name
		server.sendData(cost);
		//server.sendData(docName);

		// OUTPUT
		printf("Phase 3: %s has sent estimated price %s$ to patient with port number %d.\n", docName.c_str(), cost.c_str(), server.peerPort);
	}

	server.closeUdpSocket();

	return;
}

void input()
{
	ifstream indoc1;
	ifstream indoc2;
	indoc1.open("doc1.txt");
	indoc2.open("doc2.txt");

	insurance t;
	for (int i = 0; i < 3; ++i)
	{
		t.docName = "Doctor 1";
		indoc1 >> t.insuranceName;
		indoc1 >> t.cost;
		insuranceCost.push_back(t);

		t.docName = "Doctor 2";
		indoc2 >> t.insuranceName;
		indoc2 >> t.cost;
		insuranceCost.push_back(t);
	}
	
	indoc1.close();
	indoc2.close();
}

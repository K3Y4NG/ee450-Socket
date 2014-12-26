/*
patient1.cpp
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
#include <vector>
#include <string>
#include "udpclass.cpp"
#include "tcpclientclass.cpp"

#endif

using namespace std;

struct user
{
	string username;
	string password;
	string insurance;
};

user p;

struct availTimeFormat
{
	string index;
	string week;
	string time;
	string doctor;
	string doctorPort;
	bool vacant;
};

void input();
bool indexLegalCheck(vector<availTimeFormat> &, string);

/////////////////////
/// Main Function ///
/////////////////////

int main()
{
	string udpPatientName;
	int udpHealthCenterPort;
	string udpDoctorName;
	input();

	/////////////////////////////////////////////////////////
	// connect to the server
	tcp_client client("Patient 1", HOSTNAME, PORT);
	client.connToServer();
	
	// send username and password
	client.sendData("authenticate");
	client.sendData(p.username);
	client.sendData(p.password);

	printf("Phase 1: Authentication request from %s with username %s and password %s has been sent to the Health Center Server.\n", client.patientName.c_str(), p.username.c_str(), p.password.c_str());
	
	//if authed then send available, otherwise exit
	string authResult;
	client.recvData();
	authResult = client.recvString;
	if (authResult == "success")
	{
		client.sendData("available");
		// OUTPUT
		printf("Phase 1: %s authentication result: success.\n", client.patientName.c_str());
	}
	else if (authResult == "failure")
	{
		// OUTPUT
		printf("Phase 1: %s authentication result: failure.\n", client.patientName.c_str());
		client.closeClientSocket();
		exit(0);
	}
	// OUTPUT
	printf("Phase 1: End of Phase 1 for %s.\n", client.patientName.c_str());

	// receive available time table
	vector<availTimeFormat> availableTime;
	int n;
	availTimeFormat t;
	string indexChoice;

	// OUTPUT
	printf("Phase 2: The following appointments are available for %s:\n", client.patientName.c_str());
	while(1)
	{
		n = client.recvData();
		t.index = client.recvString;
		if (n <= 0 || (t.index == "END"))
		{
			break;
		}
		cout << t.index << " ";

		n = client.recvData();
		t.week = client.recvString;
		if (n <= 0)
		{
			break;
		}
		cout << t.week << " ";

		n = client.recvData();
		t.time = client.recvString;
		if (n <= 0)
		{
			break;
		}
		cout << t.time << " " << endl;
		availableTime.push_back(t);
	}

	// Input the index of available time
	while (1)
	{
		// OUTPUT
		printf("Please enter the preferred appointment index and press enter: ");
		cin >> indexChoice;
		// if the index is not vaild, re-enter
		if (indexLegalCheck(availableTime, indexChoice))
		{
			break;
		}
	}

	// send index to server
	client.sendData("selection");
	client.sendData(indexChoice);

	// receive the result of reserve
	string reserveResult;
	client.recvData();
	reserveResult = client.recvString;
	if (reserveResult == "notavailable")
	{
		// close the TCP connection and stop the process immediately.
		printf("Phase 2: The requested appointment from %s is not available. Exiting...\n", client.patientName.c_str());
		client.closeClientSocket();
		return 0;
	}
	else
	{
		// save the doctor name
		udpDoctorName = reserveResult;
		// receive the port number, and the TCP connection must be closed.
		client.recvData();
		string udpSocketPort = client.recvString;
		udpHealthCenterPort = atoi(udpSocketPort.c_str()); 
		udpPatientName = client.patientName;
		printf("Phase 2: The requested appointment is available and reserved to %s. The assigned doctor port number is %s.\n", client.patientName.c_str(), udpSocketPort.c_str());
		client.closeClientSocket();
	}

	// create UDP socket
	udp_socket_class udpClient(HOSTNAME, INADDR_ANY, HOSTNAME, udpHealthCenterPort);
	udpClient.start();
	printf("Phase 3: %s has a dynamic UDP port number %d and IP address %s.\n", udpPatientName.c_str(), udpClient.localPort, udpClient.localIP);

	// send UDP message to server
	udpClient.sendData(udpPatientName);
	udpClient.sendData(p.insurance);
	printf("Phase 3: The cost estimation request from %s with insurance plan %s has been sent to the doctor with port number %d and IP address %s.\n", udpPatientName.c_str(), p.insurance.c_str(), udpClient.peerPort, udpClient.peerIP);

	udpClient.recvData();
	string insuranceCost = udpClient.recvString;
	//udpClient.recvData();
	//string doctorName = udpClient.recvString;
	printf("Phase 3: %s receives %s$ estimation cost from doctor with port number %d and name %s.\n", udpPatientName.c_str(), insuranceCost.c_str(), udpClient.peerPort, udpDoctorName.c_str());

	printf("Phase 3: End of Phase 3 for %s.\n", udpPatientName.c_str());

	udpClient.closeUdpSocket();
	return 0;
}

////////////////////////////////////
/// Implement of other functions ///
////////////////////////////////////

void input()
{
	ifstream userFile;
	ifstream insuranceFile;
	userFile.open("patient1.txt");
	insuranceFile.open("patient1insurance.txt");

	userFile >> p.username >> p.password;
	insuranceFile >> p.insurance;
	
	userFile.close();
	insuranceFile.close();
}

// check if the input index legal
bool indexLegalCheck(vector<availTimeFormat> & t, string index)
{
	for (vector<availTimeFormat>::iterator it = t.begin(); it != t.end(); it++)
	{
		if (index == it->index)
		{
			//d = it->doctor;
			return true;
		}
	}
	return false;
}

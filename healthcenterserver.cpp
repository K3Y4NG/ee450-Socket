/*
healthcenterserver.cpp
University of Southern California
EE-450 Final Project
NAN JIANG
http://www.slyar.com/
2014.11.8
*/

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

using namespace std;

#define PORT 21309

// define the hostname
const string HOSTNAME = "localhost";

void input();
bool Auth(string, string);
void showErr(const char *);

struct user
{
	string username;
	string password;
};

user patient1, patient2;

struct availTimeFormat
{
	string index;
	string week;
	string time;
	string doctor;
	string doctorPort;
	bool vacant;
};

// save all available time
vector<availTimeFormat> availableTime;

// class for tcp data transfer
class tcp_data
{
private:
	int bufferSize;
public:
	int dataSocket;
	string recvString;
	tcp_data(int);
	int recvData();
	int sendData(string);
	void closeDataSocket();
	void showErr(const char *);
	char peerSocketIP[32];
	int peerSocketPort;
	char peerUserName[32];
};

// class for tcp server listen
class tcp_server
{
private:
	string hostname;
	int portNum;
	struct sockaddr_in server_address, client_address;
public:
	int listenSocket;
	tcp_server(string, int);
	void closeListenSocket();
	int acceptClient();
	int startListen();
	void showErr(const char *);
};

/////////////////////
/// Main Function ///
/////////////////////

int main()
{
	input();
	string tmp;
	tcp_server server(HOSTNAME, PORT);
	server.startListen();

	fd_set all_fd;
	fd_set read_fd;
	int fdmax;

	FD_ZERO(&all_fd);
	FD_ZERO(&read_fd);

	// add listen socket into all_fd
	FD_SET(server.listenSocket, &all_fd);
	fdmax = server.listenSocket;

	int socketReady;

	int socketAccept;
	int recvResult;
	string cmd;
	string userRecv, pswRecv;
	bool auth;
	string indexChoice;

	// save all the data socket
	vector<tcp_data> savedDataSocket;
	vector<tcp_data>::iterator cuerrentSocket;

	while(1)
	{
		// make read_fd as all_fd then select avtive socket
		read_fd = all_fd;
		socketReady = select(fdmax+1, &read_fd, NULL, NULL, NULL);

		if (socketReady == -1)
		{
			showErr("select error");
		}

		// go through all sockets less than fdmax+1
		for (int i = 0; i <= fdmax; ++i)
		{
			// find a active socket
			if (FD_ISSET(i, &read_fd))
			{
				// accept a new connection if the listen socket avtive
				if (i == server.listenSocket)
				{
					socketAccept = server.acceptClient();

					if (socketAccept > fdmax)
					{
						fdmax = socketAccept;
					}

					// add new connection socket to all_fd
					FD_SET(socketAccept, &all_fd);


					// add new socket to vector
					tcp_data temp(socketAccept);

					// get the peer socket username, IP and port
					struct sockaddr_in peerInfo;
					socklen_t peerInfoLength = sizeof(peerInfo);
					getpeername(socketAccept, (struct sockaddr*) &peerInfo, &peerInfoLength);
					strcpy(temp.peerSocketIP, inet_ntoa(peerInfo.sin_addr));
					temp.peerSocketPort = ntohs(peerInfo.sin_port);

					savedDataSocket.push_back(temp);
				}
				else
				{
					// find the saved socket, save the iterator to the savedDataSocket
					for (cuerrentSocket = savedDataSocket.begin(); cuerrentSocket != savedDataSocket.end(); cuerrentSocket++)
					{
						if (i == cuerrentSocket->dataSocket)
						{
							break;
						}
					}

					int recvResult = cuerrentSocket->recvData();

					// the client close the connection
					if (recvResult <= 0)
					{
						// peer close the connection
						cuerrentSocket->closeDataSocket();
						FD_CLR(i, &all_fd);

						// delete the socket from the vector
						savedDataSocket.erase(cuerrentSocket);
					}
					else
					{
						// get the command that client sent
						cmd = cuerrentSocket->recvString;
						//cout << "cmd: " << cmd << endl;

						// authentation patient
						if (cmd == "authenticate")
						{
							recvResult = cuerrentSocket->recvData();
							userRecv = cuerrentSocket->recvString;
							//cout << "user: " << userRecv << endl;

							recvResult = cuerrentSocket->recvData();
							pswRecv = cuerrentSocket->recvString;
							//cout << "psw: " << pswRecv << endl;

							// OUTPUT
							strcpy(cuerrentSocket->peerUserName, userRecv.c_str());
							printf("Phase 1: The Health Center Server has received request from a patient with username %s and password %s.\n", cuerrentSocket->peerUserName, pswRecv.c_str());

							// check the username and password of a patient
							if(Auth(userRecv, pswRecv))
							{
								cuerrentSocket->sendData("success");
								auth = true;
								// OUTPUT
								printf("Phase 1: The Health Center Server sends the response success to patient with username %s.\n", cuerrentSocket->peerUserName);
							}
							else
							{
								cuerrentSocket->sendData("failure");
								auth = false;
								// OUTPUT
								printf("Phase 1: The Health Center Server sends the response failure to patient with username %s.\n", cuerrentSocket->peerUserName);
							}

						}
						else if (cmd == "available")
						{
							// OUTPUT
							printf("Phase 1: The Health Center Server, receives a request for available time slots from patients with port number %d and IP address %s.\n", cuerrentSocket->peerSocketPort, cuerrentSocket->peerSocketIP);
							
							// send available time table to client
							for (vector<availTimeFormat>::iterator it = availableTime.begin(); it != availableTime.end(); it++)
							{
								if (it->vacant)
								{
									cuerrentSocket->sendData(it->index);
									cuerrentSocket->sendData(it->week);
									cuerrentSocket->sendData(it->time);
								}
							}

							// send END as a EOF
							cuerrentSocket->sendData("END");

							// OUTPUT
							printf("Phase 1: The Health Center Server sends available time slot to patient with username %s.\n", cuerrentSocket->peerUserName);
						}
						else if (cmd == "selection")
						{
							recvResult = cuerrentSocket->recvData();
							indexChoice = cuerrentSocket->recvString;

							// OUTPUT
							printf("Phase 2: The Health Center Server receives a request for appointment %s from patient with port number %d and username %s.\n", indexChoice.c_str(), cuerrentSocket->peerSocketPort, cuerrentSocket->peerUserName);

							// check the time is available or not
							bool isFree = false;
							vector<availTimeFormat>:: iterator it;
							for (it = availableTime.begin(); it != availableTime.end(); it++)
							{
								if ((indexChoice == it->index) && (it->vacant))
								{
									isFree = true;
									break;
								}
							}
							// if free, mark the time has been reserved, otherwise retuen notavailable
							if (isFree)
							{
								// OUTPUT
								printf("Phase 2: The Health Center Server confirms the following appointment %s to patient with username %s.\n", indexChoice.c_str(), cuerrentSocket->peerUserName);
								it->vacant = false;
								cuerrentSocket->sendData(it->doctor);
								cuerrentSocket->sendData(it->doctorPort);
							}
							else
							{
								// OUTPUT
								printf("Phase 2: The Health Center Server rejects the following appointment %s to patient with username %s.\n", indexChoice.c_str(), cuerrentSocket->peerUserName);
								cuerrentSocket->sendData("notavailable");
							}
						}
					}
				}
			}
		}
	}
	server.closeListenSocket();

	return 0;
}

///////////////////////////////////
/// Implement of tcp data class ///
///////////////////////////////////

tcp_data::tcp_data(int sock)
{
	this->bufferSize = 128;
	this->dataSocket = sock;
}

int tcp_data::recvData()
{
	int n;
	int recvLength = 0;
	n = read(this->dataSocket, &recvLength, sizeof(recvLength));
	recvLength = ntohl(recvLength);

	//cout << "recvLength is " << recvLength << endl;

	char buffer[this->bufferSize];
	memset(buffer, 0, this->bufferSize);
	n = read(this->dataSocket, buffer, recvLength);
	//cout << "receive length from buffer: " << n << endl;
	recvString = buffer;
	return n;
}

int tcp_data::sendData(string str)
{
	int sendLength = htonl(str.size());


	if ((write(this->dataSocket, &sendLength, sizeof(sendLength))) < 0)
	{
		this->showErr("Send Error");
		return -1;
	}

	char buffer[this->bufferSize];
	memset(buffer, 0, this->bufferSize);
	strncpy(buffer, str.c_str(), sizeof(str));
	buffer[sizeof(buffer) - 1] = 0;
	
	if ((write(this->dataSocket, str.c_str(), str.size())) < 0)
	{
		this->showErr("Send Error");
		return -1;
	}
	
	return 1;
}

void tcp_data::closeDataSocket()
{
	close(this->dataSocket);
}

void tcp_data::showErr(const char *err)
{
	perror(err);
	exit(1);
}

/////////////////////////////////////
/// Implement of tcp server class ///
/////////////////////////////////////

tcp_server::tcp_server(string s, int p)
{
	this->listenSocket = -1;
	this->hostname = s;
	this->portNum = p;
}

int tcp_server::startListen()
{
	struct hostent *hostInfo;
	hostInfo = gethostbyname(this->hostname.c_str());

	this->listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	memset((char*) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	memcpy(&server_address.sin_addr, hostInfo->h_addr, hostInfo->h_length);
	//server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(this->portNum);
	if (bind(this->listenSocket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
	{
		this->showErr("Bind Error");
		return -1;
	}
	
	if (listen(this->listenSocket, 5) < 0)
	{
		this->showErr("Listen Error");
		return -1;
	}

	// OUTPUT
	printf("Phase 1: The Health Center Server has port number %d and IP address %s.\n", this->portNum, inet_ntoa(server_address.sin_addr));
	
	return 1;
}

void tcp_server::closeListenSocket()
{
	close(this->listenSocket);
}

int tcp_server::acceptClient()
{
	int r;
	socklen_t client_addr_length = sizeof(client_address);
	r = accept(listenSocket, (struct sockaddr *) &client_address, &client_addr_length);
	return r;
}

void tcp_server::showErr(const char *err)
{
	perror(err);
	exit(1);
}

////////////////////////////////////
/// Implement of other functions ///
////////////////////////////////////

void showErr(const char *err)
{
	perror(err);
	exit(1);
}

void input()
{
	ifstream userFile;
	ifstream timeFile;
	userFile.open("users.txt");
	timeFile.open("availabilities.txt");

	userFile >> patient1.username >> patient1.password;
	userFile >> patient2.username >> patient2.password;

	availTimeFormat t;
	for (int i = 0; i < 6; ++i)
	{
		timeFile >> t.index >> t.week >> t.time >> t.doctor >> t.doctorPort;
		t.vacant = true;
		availableTime.push_back(t);
	}
	
	userFile.close();
	timeFile.close();
}

// compare the username and password
bool Auth(string u, string p)
{
	if (
		((u == patient1.username) && (p == patient1.password))
		||
		((u == patient2.username) && (p == patient2.password))
		)
	{
		return true;
	}
	return false;
}

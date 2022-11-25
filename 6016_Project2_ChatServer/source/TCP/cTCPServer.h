#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "cTCP_Client.h"

#pragma comment(lib, "Ws2_32.lib")

class cTCPServer
{
public:
	cTCPServer();
	~cTCPServer();

	struct ClientInformation {
		SOCKET socket;
		bool connected;
	};

	enum cmdType
	{
		CREATEACCOUNT,
		LOGIN
	};

	int TCP_init(PCSTR port);
	int TCP_Run();
	void CloseSocket();
	void poll();
	int ReadFromClient(ClientInformation& client);
	void responseToChatClient(int resultFromAuth,std::string& s, std::string date,cTCP_Client::returnStatus status);

	cmdType action;
	std::string email_;
	std::string password_;

	WSADATA wsaData;
	struct addrinfo* info;
	struct addrinfo hints;
	SOCKET ListenSocket;
	SOCKET ClientSocket;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
	std::vector<ClientInformation> clients;

private:
	int WinsockInit(PCSTR port);
	int SocketCreate();
	int BindSocket();
	int ListenConnection();
	int SelectConnection();
	int AcceptConnection();

};


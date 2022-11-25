#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

class cTCPServer
{
public:
	cTCPServer();
	~cTCPServer();


	int TCP_init(PCSTR port);
	int TCP_Run();
	void CloseSocket();
	int ReadFromClient();
	void decode(char* buf);

	struct ClientInformation {
		SOCKET socket;
		bool connected;
	};

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


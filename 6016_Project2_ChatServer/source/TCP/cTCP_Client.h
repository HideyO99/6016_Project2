#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

enum ClientCMD
{
	SetName = 0,
	JoinRoom,
	LeaveRoom,
	SendMSG = 4
};

class cTCP_Client
{
public:
	cTCP_Client();
	~cTCP_Client();

	enum cmdType
	{
		CREATEACCOUNT,
		LOGIN
	};

	int initialize(PCSTR ip, PCSTR port);
	void CloseSocket();
	int SendToServer(cmdType cmd, std::string email, std::string passwd);
	int ReceiveFromServer();
	int Chat(std::string user);

	WSADATA wsaData;
	struct addrinfo* info;
	struct addrinfo* ptr;
	struct addrinfo hints;
	SOCKET ConnectSocket;
	SOCKET ClientSocket;

	enum returnStatus
	{
		SUCCESS,
		INVALID_CREDENTIAL,
		ACCOUNT_EXISTS,
		INVALID_PASS,
		INTERNAL_SERVER_ERROR
	};
	returnStatus status;
	std::string createDate;





private:
	int WinsockInit(PCSTR ip, PCSTR port);
	int SocketCreate();
	int MakeConnect();
	int IOCntrolSocket();
	int CloseConnection();
	void ShutdownWinsock();
	std::map<int, cmdType> mCMDHistory;
};


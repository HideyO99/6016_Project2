#include "cTCP_Client.h"
#include <iostream>
#include <map>
#include "../protobuf/AuthProtocol.pb.h"

//std::map<std::string, int> room{ {"Network",1},{"Physic",2},{"Deploy",3} };
//RoomStatus curRoom;
uint16_t roomFlag = 0;

cTCP_Client::cTCP_Client()
{
	ZeroMemory(&wsaData, sizeof(wsaData));
	info = nullptr;
	ptr = nullptr;
	ZeroMemory(&hints, sizeof(hints));
	ConnectSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	mCMDHistory.clear();
}

cTCP_Client::~cTCP_Client()
{
}

int cTCP_Client::initialize(PCSTR ip, PCSTR port)
{
	int result = 0;

	//initialize winsock
	result = WinsockInit(ip,port);
	if (result != 0)
	{
		return result;
	}

	//socket creation
	result = SocketCreate();
	if (result != 0)
	{
		return result;
	}

	//connect
	result = MakeConnect();
	if (result != 0)
	{
		return result;
	}
	//input output socket control
	result = IOCntrolSocket();
	if (result != 0)
	{
		return result;
	}

	return result;
}

void cTCP_Client::CloseSocket()
{
	CloseConnection();
	ShutdownWinsock();
}

int cTCP_Client::SendToServer(cmdType cmd, std::string email, std::string passwd)
{
	int result = 0;
	AuthProtocol::Request sendRequest2AuthenServer;
	

	if (cmd==CREATEACCOUNT)
	{
		AuthProtocol::CreateAccountWeb* regist = sendRequest2AuthenServer.add_createacc();

		int reqID = mCMDHistory.size() + 1;
		regist->set_requestid(reqID);
		regist->set_email(email);
		regist->set_passwd(passwd);
		mCMDHistory.emplace(reqID, CREATEACCOUNT);
	}
	if (cmd == LOGIN)
	{
		AuthProtocol::AuthenticateWeb* login = sendRequest2AuthenServer.add_authen();

		int reqID = mCMDHistory.size() + 1;
		login->set_requestid(reqID);
		login->set_email(email);
		login->set_passwd(passwd);
		mCMDHistory.emplace(reqID, CREATEACCOUNT);
	}

	std::string serializedString;
	sendRequest2AuthenServer.SerializeToString(&serializedString);

	result = send(ConnectSocket, serializedString.c_str(), serializedString.length(), 0);

	return result;
}

int cTCP_Client::ReceiveFromServer()
{
	//int result = 0;

	const int rcvBuffLen = 16348;
	char rcvBuff[rcvBuffLen];

	int recvResult = recv(ConnectSocket, rcvBuff, rcvBuffLen, 0);

	AuthProtocol::Response responseFromAuthenServer;
	bool success = responseFromAuthenServer.ParseFromString(rcvBuff);
	if (!success) {
		std::cout << "Failed to parse" << std::endl;
	}

	//const AuthProtocol::AuthenticateWebSuccess& authSuccess = responseFromAuthenServer.authsuccess(0);
	//const AuthProtocol::AuthenticateWebFailure& authFail = responseFromAuthenServer.authfail(0);
	//const AuthProtocol::CreateAccountWebSuccess& createAccSuccess = responseFromAuthenServer.createdetail(0);
	//const AuthProtocol::CreateAccountWebFailure& createAccFail = responseFromAuthenServer.createfail(0);
	if (responseFromAuthenServer.authsuccess_size() != 0)//(authSuccess.has_requestid())
	{
		const AuthProtocol::AuthenticateWebSuccess& authSuccess = responseFromAuthenServer.authsuccess(0);
		createDate = authSuccess.creationdate().c_str();
		status = SUCCESS;
		return 1;
	}
	if (responseFromAuthenServer.authfail_size() != 0)//(authFail.has_requestid())
	{
		const AuthProtocol::AuthenticateWebFailure& authFail = responseFromAuthenServer.authfail(0);
		switch (authFail.fail_reason())
		{
		case AuthProtocol::AuthenticateWebFailure_reason_INVALID_CREDENTIALS:
			status = INVALID_CREDENTIAL;
			break;
		case AuthProtocol::AuthenticateWebFailure_reason_INTERNAL_SERVER_ERROR:
			status = INTERNAL_SERVER_ERROR;
			break;
		}
		return 2;
	}
	if (responseFromAuthenServer.createdetail_size() != 0)//(createAccSuccess.has_requestid())
	{
		const AuthProtocol::CreateAccountWebSuccess& createAccSuccess = responseFromAuthenServer.createdetail(0);
		status = SUCCESS;
		return 3;
	}
	if (responseFromAuthenServer.createfail_size()!=0)//(createAccFail.has_requestid())
	{
		const AuthProtocol::CreateAccountWebFailure& createAccFail = responseFromAuthenServer.createfail(0);
		switch (createAccFail.fail_reason())
		{
		case AuthProtocol::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS:
			status = ACCOUNT_EXISTS;
			break;
		case AuthProtocol::CreateAccountWebFailure_reason_INVALID_PASSWORD:
			status = INVALID_PASS;
			break;
		case AuthProtocol::CreateAccountWebFailure_reason_INTERNAL_SERVER_ERROR:
			status = INTERNAL_SERVER_ERROR;
			break;
		}
		return 4;
	}

	//return result;
}


int cTCP_Client::WinsockInit(PCSTR ip, PCSTR port)
{
	int result = 0;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("- error WSAStartup %#x\n", result);
		return 1;
	}


	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	result = getaddrinfo(ip, port, &hints, &info);
	if (result != 0)
	{
		printf("- error getaddrinfo %#x\n", result);
		WSACleanup();
		return 1;
	}


	return result;
}

int cTCP_Client::SocketCreate()
{
	int result = 0;

	ConnectSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("- error create socket %#x\n", WSAGetLastError());
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}

	return result;
}

int cTCP_Client::MakeConnect()
{
	int result = 0;

	result = connect(ConnectSocket, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("- error connecting %d\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	return result;
}

int cTCP_Client::IOCntrolSocket()
{
	int result = 0;
	DWORD NonBlock = 1;
	result = ioctlsocket(ConnectSocket, FIONBIO, &NonBlock);
	if (result == SOCKET_ERROR) {
		printf("ioctlsocket to failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	return result;
}

int cTCP_Client::CloseConnection()
{
	int result = 0;

	
	result = shutdown(ConnectSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		printf("- error shutdown %#x\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	return result;
}

void cTCP_Client::ShutdownWinsock()
{
	closesocket(ConnectSocket);
	WSACleanup();
}

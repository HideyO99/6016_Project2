#include "cTCPServer.h"
#include "../protobuf/AuthProtocol.pb.h"


cTCPServer::cTCPServer()
{
}

cTCPServer::~cTCPServer()
{
}

int cTCPServer::TCP_init(PCSTR port)
{
	int result = 0;

	//initialize winsock
	result = WinsockInit(port);
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

	//bind
	result = BindSocket();
	if (result != 0)
	{
		return result;
	}
	//listen
	result = ListenConnection();
	if (result != 0)
	{
		return result;
	}

	return result;
}


void cTCPServer::CloseSocket()
{
	freeaddrinfo(info);
	closesocket(ListenSocket);
	closesocket(ClientSocket);

	WSACleanup();
}

void cTCPServer::poll()
{
	SelectConnection();
	AcceptConnection();
}

int cTCPServer::ReadFromClient(ClientInformation& client)
{
	
	const int buflen = 16348;
	char buf[buflen];

	int recvResult = recv(client.socket, buf, buflen, 0);

	if (recvResult == 0) {
		printf("Client disconnected!\n");
		client.connected = false;
		return 0;
	}
	AuthProtocol::Request reqMSG;
	//AuthProtocol::Response replyMSG;
	bool result = reqMSG.ParseFromString(buf);
	if (!result)
	{
		std::cout << "Failed to parse message" << std::endl;
		return 1;
	}

	const AuthProtocol::AuthenticateWeb& Authen_req = reqMSG.authen(0);
	const AuthProtocol::CreateAccountWeb& CreateAcc_req = reqMSG.createacc(0);

	if (Authen_req.has_requestid())
	{
		action = LOGIN;
		email_ = Authen_req.email().c_str();
		password_ = Authen_req.passwd().c_str();

	}
	if (CreateAcc_req.has_requestid())
	{
		action = CREATEACCOUNT;
		email_ = CreateAcc_req.email().c_str();
		password_ = CreateAcc_req.passwd().c_str();
	}

	return 0;
}

void cTCPServer::responseToChatClient(int resultFromAuth, std::string& s, std::string date, cTCP_Client::returnStatus status)
{
	AuthProtocol::Response replyMSG;
	switch (resultFromAuth)
	{
	case 1:
		{
			AuthProtocol::AuthenticateWebSuccess *authSuccess = replyMSG.add_authsuccess();
			authSuccess->set_requestid(0);
			authSuccess->set_userid(0);
			authSuccess->set_creationdate(date);
		}
		break;
	case 2:
		{
			AuthProtocol::AuthenticateWebFailure *authFail = replyMSG.add_authfail();
			authFail->set_requestid(0);
			if(status == cTCP_Client::INVALID_CREDENTIAL)
				authFail->set_fail_reason(AuthProtocol::AuthenticateWebFailure_reason_INVALID_CREDENTIALS);
			if(status == cTCP_Client::INTERNAL_SERVER_ERROR)
				authFail->set_fail_reason(AuthProtocol::AuthenticateWebFailure_reason_INTERNAL_SERVER_ERROR);
		}
		break;
	case 3:
		{
			AuthProtocol::CreateAccountWebSuccess* createSuccess = replyMSG.add_createdetail();
			createSuccess->set_requestid(0);
			createSuccess->set_userid(0);
		}
		break;
	case 4:
		{
			AuthProtocol::CreateAccountWebFailure* createFail = replyMSG.add_createfail();
			createFail->set_requestid(0);
			if (status == cTCP_Client::ACCOUNT_EXISTS)
				createFail->set_fail_reason(AuthProtocol::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS);
			if (status == cTCP_Client::INVALID_PASS)
				createFail->set_fail_reason(AuthProtocol::CreateAccountWebFailure_reason_INVALID_PASSWORD);
			if (status == cTCP_Client::INTERNAL_SERVER_ERROR)
				createFail->set_fail_reason(AuthProtocol::CreateAccountWebFailure_reason_INTERNAL_SERVER_ERROR);
		}
		break;
	default:
		break;
	}
	replyMSG.SerializeToString(&s);
}



int cTCPServer::WinsockInit(PCSTR port)
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
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, port, &hints, &info);
	if (result != 0)
	{
		printf("- error getaddrinfo %#x\n", result);
		WSACleanup();
		return 1;
	}

	return result;
}

int cTCPServer::SocketCreate()
{
	int result = 0;

	ListenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("- error create socket %#x\n", WSAGetLastError());
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}

	return result;
}

int cTCPServer::BindSocket()
{
	int result = 0;

	result = bind(ListenSocket, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("- bind error %#x\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	return result;
}

int cTCPServer::ListenConnection()
{
	int result = 0;

	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("- listen error %#x\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	return result;
}

int cTCPServer::SelectConnection()
{
	int result = 0;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;
	
	FD_ZERO(&socketsReadyForReading);
	FD_SET(ListenSocket, &socketsReadyForReading);

	for (int i = 0; i < clients.size(); i++)
	{
		ClientInformation& client = clients[i];
		if (client.connected)
		{
			FD_SET(client.socket, &socketsReadyForReading);
		}
	}

	result = select(0, &socketsReadyForReading, NULL, NULL, &tv);
	if (result == SOCKET_ERROR)
	{
		printf("select() failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	return result;
}

int cTCPServer::AcceptConnection()
{
	int result = 0;
	
	if (FD_ISSET(ListenSocket, &socketsReadyForReading))
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("- accept error %#x\n", WSAGetLastError());
			freeaddrinfo(info);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		ClientInformation newClient;
		newClient.socket = ClientSocket;
		newClient.connected = true;
		clients.push_back(newClient);
	}
	return result;
}

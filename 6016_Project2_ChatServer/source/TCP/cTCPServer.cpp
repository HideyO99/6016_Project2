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

//int cTCPServer::TCP_Run()
//{
//
//	while (1)
//	{
//		SelectConnection();
//		AcceptConnection();
//
//		for (int i = clients.size() - 1; i >= 0; i--)
//		{
//			ClientInformation& client = clients[i];
//			if (!client.connected)
//			{
//				continue;
//			}
//			
//			if (FD_ISSET(client.socket,&socketsReadyForReading))
//			{
//			
//				const int buflen = 128;
//				char buf[buflen];
//
//				int recvResult = recv(client.socket, buf, buflen, 0);
//
//				if (recvResult == 0) {
//					printf("Client disconnected!\n");
//					client.connected = false;
//					continue;
//				}
//				AuthProtocol::Request reqMSG;
//				AuthProtocol::Response replyMSG;
//				bool result = reqMSG.ParseFromString(buf);
//				if (!result)
//				{
//					std::cout << "Failed to parse message" << std::endl;
//					return 1;
//				}
//
//				//cMySQL sql;
//
//				const AuthProtocol::AuthenticateWeb& Authen_req = reqMSG.authen(0);
//				const AuthProtocol::CreateAccountWeb& CreateAcc_req = reqMSG.createacc(0);
//
//				if (Authen_req.has_requestid())
//				{
//					//sql.Connect();
//					result = sql.userAuthen(Authen_req.email().c_str(), Authen_req.passwd().c_str());
//					sql.Disconnect();
//
//					switch (sql.status)
//					{
//					case cMySQL::SUCCESS:
//						{
//							AuthProtocol::AuthenticateWebSuccess* AuthenSuccess = replyMSG.add_authsuccess();
//							AuthenSuccess->set_requestid(Authen_req.requestid());
//							AuthenSuccess->set_userid(std::stoi(sql.uID));
//							AuthenSuccess->set_creationdate(sql.createDate);
//						}
//						break;
//					case cMySQL::INVALID_CREDENTIAL:
//						{
//							AuthProtocol::AuthenticateWebFailure* AuthenFail = replyMSG.add_authfail();
//							AuthenFail->set_requestid(Authen_req.requestid());
//							AuthenFail->set_fail_reason(AuthProtocol::AuthenticateWebFailure::INVALID_CREDENTIALS);
//						}
//						break;
//					default:
//						{
//							AuthProtocol::AuthenticateWebFailure* AuthenFail = replyMSG.add_authfail();
//							AuthenFail->set_requestid(Authen_req.requestid());
//							AuthenFail->set_fail_reason(AuthProtocol::AuthenticateWebFailure::INTERNAL_SERVER_ERROR);
//						}
//						break;
//					}
//				}
//				if (CreateAcc_req.has_requestid())
//				{
//					sql.Connect();
//					result = sql.createNewAccount(CreateAcc_req.email().c_str(), CreateAcc_req.passwd().c_str());
//					sql.Disconnect();
//
//					switch (sql.status)
//					{
//					case cMySQL::SUCCESS:
//						{
//							AuthProtocol::CreateAccountWebSuccess* CreateAccSuccess = replyMSG.add_createdetail();
//							CreateAccSuccess->set_requestid(Authen_req.requestid());
//							CreateAccSuccess->set_userid(std::stoi(sql.uID));
//						}
//						break;
//					case cMySQL::ACCOUNT_EXISTS:
//						{
//							AuthProtocol::CreateAccountWebFailure* CreateAccFail = replyMSG.add_createfail();
//							CreateAccFail->set_requestid(Authen_req.requestid());
//							CreateAccFail->set_fail_reason(AuthProtocol::CreateAccountWebFailure::ACCOUNT_ALREADY_EXISTS);
//						}
//						break;
//					case cMySQL::INVALID_PASS:
//						{
//							AuthProtocol::CreateAccountWebFailure* CreateAccFail = replyMSG.add_createfail();
//							CreateAccFail->set_requestid(Authen_req.requestid());
//							CreateAccFail->set_fail_reason(AuthProtocol::CreateAccountWebFailure::INVALID_PASSWORD);
//						}
//						break;
//					default:
//						{
//							AuthProtocol::CreateAccountWebFailure* CreateAccFail = replyMSG.add_createfail();
//							CreateAccFail->set_requestid(Authen_req.requestid());
//							CreateAccFail->set_fail_reason(AuthProtocol::CreateAccountWebFailure::INTERNAL_SERVER_ERROR);
//						}
//						break;
//					}
//				}
//				
//				std::string serializedString;
//				replyMSG.SerializeToString(&serializedString);
//
//				int sendResult = send(client.socket, serializedString.c_str(), recvResult, 0);
//			}
//
//		}
//
//	}
//	return 0;
//}

void cTCPServer::CloseSocket()
{
	freeaddrinfo(info);
	closesocket(ListenSocket);
	closesocket(ClientSocket);

	WSACleanup();
}

int cTCPServer::ReadFromClient()
{
	return 0;
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

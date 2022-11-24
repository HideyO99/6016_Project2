#include "cTCPServer.h"
#include "../protobuf/AuthProtocol.pb.h"
#include "../MySQL/cMySQL.h"

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

int cTCPServer::TCP_Run()
{

	while (1)
	{
		SelectConnection();
		AcceptConnection();

		for (int i = clients.size() - 1; i >= 0; i--)
		{
			ClientInformation& client = clients[i];
			if (!client.connected)
			{
				continue;
			}
			
			if (FD_ISSET(client.socket,&socketsReadyForReading))
			{
				//todo
				const int buflen = 128;
				char buf[buflen];

				int recvResult = recv(client.socket, buf, buflen, 0);

				if (recvResult == 0) {
					printf("Client disconnected!\n");
					client.connected = false;
					continue;
				}
				AuthProtocol::Request reqMSG;
				AuthProtocol::Response replyMSG;
				bool result = reqMSG.ParseFromString(buf);
				cMySQL sql;
				if (!result)
				{
					std::cout << "Failed to parse message" << std::endl;
					return 1;
				}

				switch (reqMSG.cmd())
				{
				case AuthProtocol::Request::CMD_TYPE::Request_CMD_TYPE_AUTHENTICATE:
					result = sql.userAuthen(reqMSG.authen().email().c_str(), reqMSG.authen().passwd().c_str());
					break;
				case AuthProtocol::Request::CMD_TYPE::Request_CMD_TYPE_CREATE_ACCOUNT:
					result = sql.createNewAccount(reqMSG.createacc().email().c_str(), reqMSG.createacc().passwd().c_str());
					break;
				}


				replyMSG.set_requestid(reqMSG.requestid());
				if (result)
				{
					replyMSG.set_resp_msg_type(AuthProtocol::Response_Res_TYPE::Response_Res_TYPE_SUCCESS);
				}
				else
				{
					switch (sql.status)
					{
					case cMySQL::cmdStatus::ACCOUNT_EXISTS:
						replyMSG.set_resp_msg_type(AuthProtocol::Response_Res_TYPE::Response_Res_TYPE_ACCOUNT_EXISTS);
						break;
					case cMySQL::cmdStatus::INVALID_CREDENTIAL:
						replyMSG.set_resp_msg_type(AuthProtocol::Response_Res_TYPE::Response_Res_TYPE_INVALID_CREDENTIALS);
						break;
					case cMySQL::cmdStatus::INVALID_PASS:
						replyMSG.set_resp_msg_type(AuthProtocol::Response_Res_TYPE::Response_Res_TYPE_INVALID_PASS);
						break;
					case cMySQL::cmdStatus::INTERNAL_SERVER_ERROR:
						replyMSG.set_resp_msg_type(AuthProtocol::Response_Res_TYPE::Response_Res_TYPE_INTERNAL_SERVER_ERROR);
						break;
					}

				}

				std::string serializedString;
				replyMSG.SerializeToString(&serializedString);

				int sendResult = send(client.socket, serializedString.c_str(), recvResult, 0);
			}

		}

	}
	return 0;
}

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

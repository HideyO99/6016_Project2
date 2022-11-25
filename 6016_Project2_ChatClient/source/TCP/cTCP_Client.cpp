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

	const int rcvBuffLen = 512;
	char rcvBuff[rcvBuffLen];

	int recvResult = recv(ConnectSocket, rcvBuff, rcvBuffLen, 0);

	AuthProtocol::Response responseFromAuthenServer;
	bool success = responseFromAuthenServer.ParseFromString(rcvBuff);
	if (!success) {
		std::cout << "Failed to parse" << std::endl;
	}

	const AuthProtocol::AuthenticateWebSuccess& authSuccess = responseFromAuthenServer.authsuccess(0);
	const AuthProtocol::AuthenticateWebFailure& authFail = responseFromAuthenServer.authfail(0);
	const AuthProtocol::CreateAccountWebSuccess& createAccSuccess = responseFromAuthenServer.createdetail(0);
	const AuthProtocol::CreateAccountWebFailure& createAccFail = responseFromAuthenServer.createfail(0);
	if (authSuccess.has_requestid())
	{
		createDate = authSuccess.creationdate().c_str();
		status = SUCCESS;
		return 1;
	}
	if (authFail.has_requestid())
	{
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
	if (createAccSuccess.has_requestid())
	{
		status = SUCCESS;
		return 3;
	}
	if (createAccFail.has_requestid())
	{
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

//int cTCP_Client::Chat(std::string user)
//{
//	int result = 0;
//	bool terminate = false;
//	std::string input;
//	std::string cmd;
//	
//	//uint16_t opcode = 0xff;
//	//std::string para;
//	
//	std::cout << "Type /J [room] to join a room" << std::endl;
//	std::cout << "Type /L [room] to Leave a room" << std::endl;
//	std::cout << "Type /S [message] to send message to a room" << std::endl;
//	std::cout << "Type /X  to exit program" << std::endl;
//	std::cout << "Room Available" << std::endl;
//	for (auto it = room.begin();
//		it != room.end(); it++)
//	{
//		std::cout <<"< " << it->first << "  >\t";
//	}
//	std::cout << std::endl;
//	std::getline(std::cin, input);
//	uint16_t opcode = 0xff;
//	int i = 0;
//	while (!terminate)
//	{
//		std::cout << i++ << " ";
//		std::cout << user << "> ";
//		std::getline(std::cin, input);
//		size_t delimiterPos = input.find(' ');
//		if (delimiterPos != std::string::npos)
//		{
//			cmd = input.substr(0, delimiterPos);
//			input.erase(0, delimiterPos + 1);
//			while (input.front() == ' ')
//			{
//				delimiterPos = input.find(' ');
//				input.erase(0, delimiterPos + 1);
//			}
//			//join room
//			if ((cmd.compare("/J") == 0) || (cmd.compare("/j") == 0))
//			{
//				auto found = room.find(input);
//				if (found != room.end())
//				{
//					opcode = (uint16_t)(found->second);
//					result = SendToServer(JoinRoom, opcode, user);
//					roomFlag = opcode;
//				}
//				else
//				{
//					std::cout << "incorrect room" << std::endl;
//				}
//			}
//			//leave room
//			if ((cmd.compare("/L") == 0) || (cmd.compare("/l") == 0))
//			{
//				auto found = room.find(input);
//				if (found != room.end())
//				{
//					opcode = (uint16_t)(found->second);
//					if (curRoom.getActiveRoom(opcode))
//					{
//						result = SendToServer(LeaveRoom, opcode, user);
//						roomFlag = opcode;
//					}
//					else {
//						std::cout << "you are not in the room" << std::endl;
//					}
//				}
//				else
//				{
//					std::cout << "incorrect room" << std::endl;
//				}
//			}
//			//send message
//			if ((cmd.compare("/S") == 0) || (cmd.compare("/s") == 0))
//			{
//				std::string toRoom;
//				delimiterPos = input.find(' ');
//				toRoom = (input.substr(0, delimiterPos));
//				input.erase(0, delimiterPos + 1);
//				auto found = room.find(toRoom);
//				if (found != room.end())
//				{
//					opcode = (uint16_t)(found->second);
//					result = SendToServer(SendMSG, opcode, input);
//				}
//				else
//				{
//					std::cout << "incorrect room" << std::endl;
//				}
//				
//			}
//			result = ReceiveFromServer();
//			//system("Pause");
//		}
//		if ((input.compare("/X") == 0) || (input.compare("/x") == 0))
//		{
//			terminate = true;
//			break;
//		}
//	}
//	
//
//	return result;
//}

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

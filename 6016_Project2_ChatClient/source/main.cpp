#include <iostream>
#include "TCP/cTCP_Client.h"


#define CHAT_SERVERIP "127.0.0.1"
#define CHAT_SERVER_PORT "1112"

int main(int argc, char** argv)
{
	//srand((unsigned)time(NULL));

	cTCP_Client ChatClient;
	ChatClient.initialize(CHAT_SERVERIP, CHAT_SERVER_PORT);

	//chatServer.TCP_Run();
	std::string opt;
	std::cout << "please select cmd \n 1: create new account\n or\ 2:Login \n:>";
	std::cin >> opt;
	int cmd = std::stoi(opt);
	std::string email;
	std::string pass;
	if (cmd == 1)
	{
		bool result = false;
		int receiveResult = 0;
		//while (!result)
		{
			std::cout << " Please enter email and password to create new account" << std::endl << "email:>";
			std::cin >> email;
			std::cout << "password:>";
			std::cin >> pass;

			ChatClient.SendToServer(cTCP_Client::CREATEACCOUNT, email, pass);
			receiveResult = ChatClient.ReceiveFromServer();
			if (receiveResult == 3)
			{
				std::cout << "account created" << std::endl;
				result = true;
				//break;
			}
			switch (ChatClient.status)
			{
			case cTCP_Client::ACCOUNT_EXISTS:
				std::cout << "Error ACCOUNT_ALREADY_EXISTS" << std::endl;
				break;
			case cTCP_Client::INVALID_PASS:
				std::cout << "Invalid password" << std::endl;
				break;
			case cTCP_Client::INTERNAL_SERVER_ERROR:
				std::cout << "Server Error" << std::endl;
				break;
			default:
				break;
			}
		}
	}
	if (cmd == 2)
	{
		bool result = false;
		int receiveResult = 0;
		//while (!result)
		{
			std::cout << " Please enter email and password to login" << std::endl << "email:>";
			std::cin >> email;
			std::cout << "password:>";
			std::cin >> pass;

			ChatClient.SendToServer(cTCP_Client::LOGIN, email, pass);
			receiveResult = ChatClient.ReceiveFromServer();
			if (receiveResult == 1)
			{
				std::cout << "login completed" << std::endl;
				result = true;
				//break;
			}
			switch (ChatClient.status)
			{
			case cTCP_Client::INVALID_CREDENTIAL:
				std::cout << "Invalid Credential" << std::endl;
				break;
			case cTCP_Client::INTERNAL_SERVER_ERROR:
				std::cout << "Server Error" << std::endl;
				break;
			default:
				break;
			}
		}
	}


	ChatClient.CloseSocket();


	return 0;
}
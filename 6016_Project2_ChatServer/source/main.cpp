#include <iostream>
#include "TCP/cTCPServer.h"
#include "TCP/cTCP_Client.h"

#define AUTHEN_PORT "1112"
#define AUTHEN_SERVERIP "127.0.0.1"
#define CHAT_SERVER_PORT "1150"

int main(int argc, char** argv)
{
	//srand((unsigned)time(NULL));

	cTCPServer chatServer;
	cTCP_Client AuthenClient;

	chatServer.TCP_init(AUTHEN_PORT);
	AuthenClient.initialize(AUTHEN_SERVERIP, AUTHEN_PORT);

	//chatServer.TCP_Run();
	while (1)
	{
		chatServer.poll();
		for (int i = chatServer.clients.size() - 1; i >= 0; i--)
		{
			cTCPServer::ClientInformation& client = chatServer.clients[i];
			if (!client.connected)
			{
				continue;
			}

			if (FD_ISSET(client.socket, &chatServer.socketsReadyForReading))
			{
				chatServer.ReadFromClient(client);
				std::string serializedString;
				if (chatServer.action == cTCPServer::CREATEACCOUNT)
				{
					int resultFromAuthServer;
					AuthenClient.SendToServer(cTCP_Client::CREATEACCOUNT, chatServer.email_, chatServer.password_);
					resultFromAuthServer = AuthenClient.ReceiveFromServer();
					
					chatServer.responseToChatClient(resultFromAuthServer, serializedString, AuthenClient.createDate,AuthenClient.status);
					int sendResult = send(client.socket, serializedString.c_str(), serializedString.length(), 0);
				}
				if (chatServer.action == cTCPServer::LOGIN)
				{
					int resultFromAuthServer;
					AuthenClient.SendToServer(cTCP_Client::LOGIN, chatServer.email_, chatServer.password_);
					resultFromAuthServer = AuthenClient.ReceiveFromServer();
					chatServer.responseToChatClient(resultFromAuthServer, serializedString, AuthenClient.createDate, AuthenClient.status);
					int sendResult = send(client.socket, serializedString.c_str(), serializedString.length(), 0);
				}
				serializedString.clear();
			}
		}
		//chatServer.ReadFromClient();


	}
	chatServer.CloseSocket();
	AuthenClient.CloseSocket();


	return 0;
}
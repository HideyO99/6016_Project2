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

	}
	chatServer.CloseSocket();
	AuthenClient.CloseSocket();


	return 0;
}
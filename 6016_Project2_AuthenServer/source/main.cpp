#include <iostream>
#include "MySQL/cMySQL.h"
#include "TCP/cTCPServer.h"


int main(int argc, char** argv)
{
	srand((unsigned)time(NULL));
	
	cTCPServer server;
	server.TCP_init("1150");
	server.TCP_Run();
	server.CloseSocket();


	return 0;
}
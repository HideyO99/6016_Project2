#include "cMySQL.h"
#include <iostream>

cMySQL::cMySQL()
{
	pDriver = nullptr;
	pConnection = nullptr;
	pResultSet = nullptr;
	pStatement = nullptr;
	pInsertStatement = nullptr;
}

cMySQL::~cMySQL()
{
}

bool cMySQL::Connect()
{
	try
	{
		pDriver = sql::mysql::get_driver_instance();
	}
	catch (sql::SQLException e)
	{
		std::cerr << "Unable to get driver instance: " << e.what() << std::endl;
		return false;
	}
	std::cout << "Get SQL driver is done" << std::endl;

	try
	{
		sql::SQLString hostName(HOST);
		sql::SQLString userDB(USERNAME);
		sql::SQLString passwdDB(PASSWD);
		pConnection = pDriver->connect(HOST, USERNAME, PASSWD);
		pConnection->setSchema(SCHEMA);
	}
	catch (sql::SQLException e)
	{
		std::cerr << "Unable to connect DB: " << e.what() << std::endl;
		return false;
	}
	std::cout << "Database connected " << std::endl;

	return true;
}

void cMySQL::Disconnect()
{
	try
	{
		pConnection->close();
	}
	catch (sql::SQLException e)
	{
		std::cerr << "Unable to disconnect: " << e.what() << std::endl;
		return;
	}
	std::cout << "Database disconnected " << std::endl;

	delete pStatement;
	delete pResultSet;
	delete pInsertStatement;

}

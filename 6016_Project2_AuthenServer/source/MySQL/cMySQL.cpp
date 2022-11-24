#include "cMySQL.h"
#include <iostream>
#include "../protobuf/AuthProtocol.pb.h"

cMySQL::cMySQL()
{
	Salt_.clear();
	Hash_.clear();
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

void cMySQL::genSalt(int length)
{
	std::string Salt;
	for (size_t i = 0; i < length; i++)
	{
		char c = (rand() % 2 == 1) ? 'a' : 'A';
		c += rand() % 26;
		Salt += c;
	}
	Salt_ = Salt;
}

void cMySQL::calHash(std::string input)
{
	SHA256 sha256;
	Hash_ = sha256(Salt_ + input).c_str();
}

bool cMySQL::createNewAccount(std::string email,std::string passwd)
{
	//Connect();
	genSalt(passwd.length());
	calHash(passwd.c_str());
	try
	{
		pInsertStatement = pConnection->prepareStatement("INSERT INTO web_auth (email, salt, hashed_password) VALUES (?,?,?);");

	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot create new account: " << e.what() << std::endl;
		return false;
	}

	pInsertStatement->setString(1, email);
	pInsertStatement->setString(2, Salt_);
	pInsertStatement->setString(3, Hash_);
	
	try
	{
		pInsertStatement->execute();
	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot execute create new account: " << e.what() << std::endl;
	}

	try
	{
		pStatement = pConnection->createStatement();
		pStatement->execute("INSERT INTO user (creation_date) VALUES (now()); ");
	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot insert creation date: " << e.what() << std::endl;
	}

	Salt_.clear();
	Hash_.clear();
	//Disconnect();

	return true;
}

bool cMySQL::userAuthen(std::string email, std::string passwd)
{
	//Connect();
	try
	{
		std::string q = "SELECT salt, hashed_password, userID FROM authserver WHERE email = " + email;
		pStatement = pConnection->createStatement();
		pResultSet = pStatement->executeQuery(q);
		Salt_.clear();
		Salt_ = pResultSet->getString("salt");
		calHash(passwd.c_str());
		if (Hash_.compare(pResultSet->getString("hashed_password")) != 0)
		{
			std::cout << "incorrect password" << std::endl;
		}
		std::string u = "UPDATE user SET last_login = now() WHERE id = " + pResultSet->getString("userID");
		pStatement = pConnection->createStatement();
		pStatement->executeUpdate(u);
	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot login : " << e.what() << std::endl;
		return false;
	}
	return true;
}

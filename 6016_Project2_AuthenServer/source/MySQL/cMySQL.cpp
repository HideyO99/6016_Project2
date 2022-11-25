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
		status = INTERNAL_SERVER_ERROR;
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
		status = INTERNAL_SERVER_ERROR;
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
		status = INTERNAL_SERVER_ERROR;
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

bool cMySQL::createNewAccount(int reqID, std::string email,std::string passwd)
{
	//Connect();
	genSalt(passwd.length());
	calHash(passwd.c_str());
	try
	{
		//std::string q = "SELECT * FROM authserver WHERE email = " + email;
		//pStatement = pConnection->createStatement();
		//pResultSet = pStatement->executeQuery(q);
		//if (pResultSet->getRow() != 0)
		//	return false;

		pInsertStatement = pConnection->prepareStatement("INSERT INTO web_auth (email, salt, hashed_password,userId) VALUES (?,?,?,?);");

	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot create new account: " << e.what() << std::endl;
		status = INTERNAL_SERVER_ERROR;
		return false;
	}

	pInsertStatement->setString(1, email);
	pInsertStatement->setString(2, Salt_);
	pInsertStatement->setString(3, Hash_);
	pInsertStatement->setInt64(4, reqID);
	uID = reqID;
	
	try
	{
		pInsertStatement->execute();
		pInsertStatement->clearAttributes();
		pInsertStatement = nullptr;
		//pInsertStatement->clearParameters();
	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot execute create new account: " << e.what() << std::endl;
		status = ACCOUNT_EXISTS;
	}

	try
	{
		pInsertStatement = pConnection->prepareStatement("INSERT INTO user (userId, creation_date) VALUES (?,now());");
		pInsertStatement->setInt64(1, reqID);
		pInsertStatement->execute();
		//pStatement = pConnection->createStatement();
		//pStatement->execute("INSERT INTO user (userId,creation_date) VALUES ("+std::to_string(reqID)+",now()); ");
		//pResultSet = pStatement->executeQuery("SELECT id FROM web_auth WHERE email = "+email);
		//uID = pResultSet->getInt64("id");
		//pStatement->executeUpdate("UPDATE web_auth SET userID = "+std::to_string(uID)+" WHERE email = " + email);
	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot insert creation date: " << e.what() << std::endl;
	}
	status = SUCCESS;

	Salt_.clear();
	Hash_.clear();
	//Disconnect();

	return true;
}

bool cMySQL::userAuthen(int reqID, std::string email, std::string passwd)
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
			status = INVALID_CREDENTIAL;
			return false;
		}
		uID = pResultSet->getInt64("userID");
		std::string u = "UPDATE user SET last_login = now() WHERE id = " + uID;
		pStatement = pConnection->createStatement();
		pStatement->executeUpdate(u);

		q = "SELECT creation_date FROM user WHERE id = " + uID;
		pStatement = pConnection->createStatement();
		pResultSet = pStatement->executeQuery(q);
		createDate= pResultSet->getString("creation_date");
	}
	catch (sql::SQLException e)
	{
		std::cerr << "cannot login : " << e.what() << std::endl;
		status = INTERNAL_SERVER_ERROR;
		return false;
	}
	status = SUCCESS;
	return true;
}

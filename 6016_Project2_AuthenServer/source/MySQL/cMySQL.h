#pragma once

#include "jdbc/mysql_driver.h"
#include "jdbc/mysql_connection.h"
#include "jdbc/mysql_error.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"
#include "jdbc/cppconn/resultset.h"
#include "../hash-library/sha256.h"

#define HOST		"127.0.0.1:3306"
#define USERNAME	"yo"
#define PASSWD		"1234"
#define SCHEMA		"authserver"

class cMySQL
{
public:
	cMySQL();
	~cMySQL();

	bool Connect();
	void Disconnect();
	
	void genSalt(int length);
	void calHash(std::string input);
	bool createNewAccount(int reqID, std::string email, std::string passwd);
	bool userAuthen(int reqID, std::string email, std::string passwd);
	enum cmdStatus
	{
		SUCCESS,
		INVALID_CREDENTIAL,
		ACCOUNT_EXISTS,
		INVALID_PASS,
		INTERNAL_SERVER_ERROR
	};
	cmdStatus status;
	int uID;
	std::string createDate;

private:
	std::string Salt_;
	std::string Hash_;
	sql::Driver* pDriver;
	sql::Connection* pConnection;
	sql::ResultSet* pResultSet;
	sql::Statement* pStatement;
	sql::PreparedStatement* pInsertStatement;
};


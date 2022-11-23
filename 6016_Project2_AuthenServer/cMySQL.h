#pragma once

#include "jdbc/mysql_driver.h"
#include "jdbc/mysql_connection.h"
#include "jdbc/mysql_error.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"
#include "jdbc/cppconn/resultset.h"
#include <bcrypt.h>

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
	

private:
	sql::Driver* pDriver;
	sql::Connection* pConnection;
	sql::ResultSet* pResultSet;
	sql::Statement* pStatement;
	sql::PreparedStatement* pInsertStatement;
};


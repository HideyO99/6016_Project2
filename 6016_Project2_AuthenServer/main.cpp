#include <iostream>
#include "cMySQL.h"
#include "sCredential.h"

int main(int argc, char** argv)
{
	cMySQL db;

	if (!db.Connect())
	{
		return -1;
	}


	db.Disconnect();

	return 0;
}
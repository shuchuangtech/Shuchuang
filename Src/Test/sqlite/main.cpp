#include <stdio.h>
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include "Poco/Types.h"
#include "Common/PrintLog.h"
#include "Poco/Thread.h"
using namespace Poco::Data::Keywords;
struct User
{
	int id;
	std::string username;
	std::string password;
	int authority;
	Poco::Int64 timeOfValidity;
	int	remainOpen;
	std::string token;
	Poco::Int64 lastVerify;
	Poco::Int64 lastLogin;
};

int main(int argc, char** argv)
{
	Poco::Data::SQLite::Connector::registerConnector();
	Poco::Data::Session* session = new Poco::Data::Session("SQLite", "/home/hj/Dev_Env/Shuchuang/test.db");
	/*
	int t = 0;
	for(int i = 0; i < 10; i++)
	{
		t = session.getLoginTimeout();
		printf("login timeout: %d\n", t);
		t = session.getConnectionTimeout();
		printf("Connection timeout: %d\n", t);
		Poco::Thread::sleep(5000);
	}
	session.close();
	*/
	/*
	try
	{
		Poco::Data::Statement create(*session);
		create << "CREATE TABLE IF NOT EXISTS `User` ("
			<< "`Id` INTEGER PRIMARY KEY AUTOINCREMENT,"
			<< "`Username` VARCHAR(30) NOT NULL,"
			<< "`Password` VARCHAR(64) NOT NULL,"
			<< "`Authority` TINYINT,"
			<< "`TimeOfValidity` BIGINT,"
			<< "`RemainOpen` INTEGER,"
			<< "`Token` VARCHAR(64),"
			<< "`LastVerify` BIGINT,"
			<< "`LastLogin` BIGINT);";
		create.execute();
	}
	catch(Poco::Exception& e)
	{
		printf("%s\n", e.message().c_str());
	}*/
	tracepoint();
	struct User user1;
	user1.username = "huangjian";
	user1.password = "huangjian";
	
	Poco::Data::Statement insert(*session);
	insert << "INSERT INTO `User` (`Username`, `Password`) VALUES(?, ?)",
		   use(user1.username),
		   use(user1.password);
	tracepoint();
	printf("sql string:%s\n", insert.toString().c_str());
	/*
	try
	{
		insert.execute();
	}
	catch(Poco::Exception& e)
	{
		printf("%s\n", e.message().c_str());
	}
	tracepoint();
	Poco::Data::Statement select(session);
	tracepoint();
	select << "SELECT `Id`, `Username`, `Password` FROM `User`",
		   into(user1.id),
		   into(user1.username),
		   into(user1.password),
		   range(0, 1);
	tracepoint();
	while(!select.done())
	{
		select.execute();
		printf("User Id: %d, Username: %s, Password: %s\n", user1.id, user1.username.c_str(), user1.password.c_str());
	}
*/
	return 0;
}


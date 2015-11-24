#include <stdio.h>
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include "Poco/Types.h"
#include "Common/PrintLog.h"
#include "Poco/Thread.h"
#include "Poco/SHA1Engine.h"
using namespace Poco::Data::Keywords;
struct User
{
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
	Poco::Data::Statement create(*session);
	create << "CREATE TABLE IF NOT EXISTS `User` ("
			"`Id` INTEGER PRIMARY KEY AUTOINCREMENT,"
			"`Username` VARCHAR(30) NOT NULL UNIQUE,"
			"`Password` VARCHAR(64) NOT NULL,"
			"`Authority` TINYINT,"
			"`TimeOfValidity` BIGINT,"
			"`RemainOpen` INTEGER,"
			"`Token` VARCHAR(64),"
			"`LastVerify` BIGINT,"
			"`LastLogin` BIGINT)", now;
	struct User user;
	user.username = "admin";
	Poco::SHA1Engine sha1;
	std::string password = "admin@shuchuang";
	sha1.update(password);
	const Poco::DigestEngine::Digest& digest = sha1.digest();
	std::string sha1pass(Poco::DigestEngine::digestToHex(digest));
	user.password = sha1pass;
	user.authority = 9;
	Poco::DateTime tov(2050, 1, 1);
	user.timeOfValidity = tov.timestamp().epochMicroseconds();
	user.remainOpen = -1;
	user.token = "";
	user.lastVerify = 0;
	user.lastLogin = 0;
	
	Poco::Data::Statement insert(*session);
	insert << "INSERT INTO `User` (`Username`, `Password`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin`) VALUES(?, ?, ?, ?, ?, ?, ?, ?)",
		   use(user.username),
		   use(user.password),
		   use(user.authority),
		   use(user.timeOfValidity),
		   use(user.remainOpen),
		   use(user.token),
		   use(user.lastVerify),
		   use(user.lastLogin);
	try
	{
		insert.execute();
	}
	catch(Poco::Exception& e)
	{
		printf("%s\n", e.message().c_str());
	}
	session->close();
	delete session;
	return 0;
}


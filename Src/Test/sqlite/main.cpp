#include <stdio.h>
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include "Poco/Types.h"
#include "Common/PrintLog.h"
#include "Poco/Thread.h"
#include "Poco/SHA1Engine.h"
#include "Poco/Random.h"
#include <fstream>
using namespace Poco::Data::Keywords;
char character[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
struct User
{
	std::string username;
	std::string password;
	int authority;
	Poco::Int64 timeOfValidity;
	std::string binduser;
	int	remainOpen;
	std::string token;
	Poco::Int64 lastVerify;
	Poco::Int64 lastLogin;
};

std::string generatePassword(int num)
{
	char* pass = new char[num + 1];
	Poco::Random rand;
	rand.seed();
	for(int i = 0; i < num; i++)
	{
		Poco::UInt32 t = rand.next(62);
		pass[i] = character[t];
	}
	pass[num] = 0;
	std::string ret(pass);
	delete[] pass;
	return ret;
}

void generateUser(const std::string& uuid)
{
	std::string path = "/home/huang_jian/samba/";
	path += uuid;
	path += "/user.db";
	Poco::Data::Session* session = new Poco::Data::Session("SQLite", path.c_str());
	Poco::Data::Statement create(*session);
	create << "CREATE TABLE IF NOT EXISTS `User` ("
			"`Id` INTEGER PRIMARY KEY AUTOINCREMENT,"
			"`Username` VARCHAR(64) NOT NULL UNIQUE,"
			"`Password` VARCHAR(64) NOT NULL,"
			"`BindUser` VARCHAR(64),"
			"`Authority` TINYINT,"
			"`TimeOfValidity` BIGINT,"
			"`RemainOpen` INTEGER,"
			"`Token` VARCHAR(64),"
			"`LastVerify` BIGINT,"
			"`LastLogin` BIGINT)", now;

	struct User user;
	user.username = "admin";
	Poco::SHA1Engine sha1;
	std::string password = "";
	password = generatePassword(10);

	std::ofstream fos("/home/huang_jian/Dev_Env/Shuchuang/user_pass", std::ios::out|std::ios::app);
	fos << uuid << "\t" << "admin" << "\t" << password << std::endl;
	std::cout << uuid << "\t" << "admin" << "\t" << password << std::endl;

	sha1.update(password);
	const Poco::DigestEngine::Digest& digest = sha1.digest();
	std::string sha1pass(Poco::DigestEngine::digestToHex(digest));
	user.password = sha1pass;
	user.authority = 9;
	Poco::DateTime tov(2050, 1, 1);
	user.timeOfValidity = tov.timestamp().epochMicroseconds();
	user.remainOpen = -1;
	user.token = "";
	user.binduser = "";
	user.lastVerify = 0;
	user.lastLogin = 0;
	
	Poco::Data::Statement insert(*session);
	insert << "INSERT INTO `User` (`Username`, `Password`, `BindUser`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin`) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)",
		   use(user.username),
		   use(user.password),
		   use(user.binduser),
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
}

void generateBackup(const std::string uuid)
{
	std::string dirPath = "/home/huang_jian/samba/";
	dirPath += uuid;
	std::string oriPath = dirPath + "/user.db";
	std::string backPath = dirPath + "user_bak.db";
	std::ifstream ifs("/home/huang_jian/samba/");
}

int main(int argc, char** argv)
{
	Poco::Data::SQLite::Connector::registerConnector();
	generateUser("SC0000000011");
	/*
	for(int i = 1; i <= 500; i++)
	{
		char uuid[13] = {0, };
		snprintf(uuid, 13, "SC%010d", i);
	//	generateUser(uuid);
		generateBackup(uuid);
	}
	*/
	return 0;
}


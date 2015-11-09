#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/SessionFactory.h"
#include "Common/PrintLog.h"
using namespace Poco::Data::Keywords;
struct User
{
	int id;
	std::string username;
	std::string password;
	std::string mobile;
};
int main(int argc, char** argv)
{
	Poco::Data::MySQL::Connector::registerConnector();
	Poco::SharedPtr<Poco::Data::Session> pSession = 0;
	std::string dbConnString = "user=root;password=drmfSlxd12;db=test;compress=true;auto-reconnect=true";
	try
	{
		pSession = new Poco::Data::Session(Poco::Data::SessionFactory::instance().create(Poco::Data::MySQL::Connector::KEY, dbConnString));
	}
	catch(Poco::Data::MySQL::ConnectionException& e)
	{
		warnf("%s, %d: connect fail, %s.", __FILE__, __LINE__, e.message().c_str());
	}

	if(pSession && pSession->isConnected())
	{
		tracef("%s, %d: Connect to db test.", __FILE__, __LINE__);
	}
	User user={
		0,
		"wangshuai",
		"ws123456",
		"18712345678"
	};
	/*
	Poco::Data::Statement insert(*pSession);
	try
	{
		insert << "INSERT INTO User(id, name, password, mobile) VALUES(?, ?, ?, ?)",
		   use(user.id),
		   use(user.username),
		   use(user.password),
		   use(user.mobile);
		insert.execute();
	}
	catch(Poco::Data::MySQL::MySQLException& e)
	{
		warnf("%s, %d: %s.", __FILE__, __LINE__, e.message().c_str());
	}
	*/
	Poco::Data::Statement select(*pSession);
	select << "SELECT id, name, password, mobile FROM User where name='huangjian'", 
		   into(user.id),
		   into(user.username),
		   into(user.password),
		   into(user.mobile);
	while(!select.done())
	{
		select.execute();
		tracef("id: %d, username:%s, password:%s, mobile:%s.", user.id, user.username.c_str(), user.password.c_str(), user.mobile.c_str());
	}
	return 0;
}


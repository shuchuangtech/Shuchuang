#include "Poco/Thread.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Struct.h"
#include "Common/PrintLog.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/Context.h"
#include "Poco/Timespan.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/MD5Engine.h"
#include "Common/ConfigManager.h"
using namespace Poco;
using namespace Poco::Net;
char myGetchar()
{
	char last = 0;
	char c = 0;
	c = getchar();
	while(c != '\n')
	{
		last = c;
		c = getchar();
	}
	return last;
}

void sslTest1()
{
	tracef("ssl test 1 begin.");
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	for(int i = 0; i < 3; i++)
	{
		tracef("%d seconds...", i + 1);
		Thread::sleep(1000);
	}
	sss.close();
	tracef("ssl test 1 finished.\n");
}

void sslTest2()
{
	tracef("ssl test 2 begin: ssl send data.");
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.token";
	Timestamp t;
	UInt64 tms = t.epochMicroseconds();
	char tms_str[32];
	snprintf(tms_str, 31, "%llu", tms);
	std::string key = "alpha2015";
	key += tms_str;
	MD5Engine md5;
	md5.update(key);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key = DigestEngine::digestToHex(digest);
	DynamicStruct param;
	param["key"] = md5key;
	param["timestamp"] = tms_str;
	param["dev_name"] = "lock1";
	param["dev_type"] = "sc-01";
	param["uuid"] = "SC00000001";
	ds["param"] = param;

	tracef("data send: %s.", ds.toString().c_str());
	sss.sendBytes(ds.toString().c_str(), ds.toString().length());
	sss.close();
	tracef("socket closed.");
	tracef("ssl test 2 finished.\n");
}

void sslTest3()
{	
	tracef("ssl test 3 begin: ssl receive data.");
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.token";
	Timestamp t;
	UInt64 tms = t.epochMicroseconds();
	char tms_str[32];
	snprintf(tms_str, 31, "%llu", tms);
	std::string key = "alpha2015";
	key += tms_str;
	MD5Engine md5;
	md5.update(key);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key = DigestEngine::digestToHex(digest);
	DynamicStruct param;
	param["key"] = md5key;
	param["timestamp"] = tms_str;
	param["dev_name"] = "lock1";
	param["dev_type"] = "sc-01";
	param["uuid"] = "SC00000001";
	ds["param"] = param;

	tracef("data send: %s.", ds.toString().c_str());
	sss.sendBytes(ds.toString().c_str(), ds.toString().length());

	char buf[1024] = {0, };
	sss.receiveBytes(buf, 1024);
	tracef("data receive: %s.", buf);
	tracef("ssl test 3 finished.\n");
}

void sslTest4()
{	
	tracef("ssl test 4 begin: ssl send 2 times.");
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.token";
	Timestamp t;
	UInt64 tms = t.epochMicroseconds();
	char tms_str[32];
	snprintf(tms_str, 31, "%llu", tms);
	std::string key = "alpha2015";
	key += tms_str;
	MD5Engine md5;
	md5.update(key);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key = DigestEngine::digestToHex(digest);
	DynamicStruct param;
	param["key"] = md5key;
	param["timestamp"] = tms_str;
	param["dev_name"] = "lock1";
	param["dev_type"] = "sc-01";
	param["uuid"] = "SC00000001";
	ds["param"] = param;

	tracef("data send: %s.", ds.toString().c_str());
	sss.sendBytes(ds.toString().c_str(), ds.toString().length());

	char buf[1024] = {0, };
	sss.receiveBytes(buf, 1024);
	tracef("data receive: %s.", buf);
	sss.close();
	tracef(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	Thread::sleep(2000);
	tracef("data send again: %s.", ds.toString().c_str());
	SecureStreamSocket sss2(pContext);
	sss2.connect(sa, Timespan(3, 0));
	sss2.sendBytes(ds.toString().c_str(), ds.toString().length());
	memset(buf, 0, 1024);
	sss2.receiveBytes(buf, 1024);
	tracef("data receive: %s.", buf);
	sss2.close();
	tracef("ssl test 4 finished.\n");
}

void sslTest5()
{
	tracef("ssl test 5 begin: ssl connect timeout, now sleep.");
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	for(int i = 0; i < 25; i++)
	{
		tracef("%d seconds...", i + 1);
		Thread::sleep(1000);
	}
	sss.close();
	tracef("ssl test 5 finished.\n");
}

void regTest1()
{
	tracef("reg test 1 begin: reg connect timeout.");
	StreamSocket ss;
	SocketAddress sa("127.0.0.1", 13333);
	ss.connect(sa, Timespan(3, 0));
	for(int i = 0; i < 3; i++)
	{
		tracef("%d seconds...", i + 1);
		Thread::sleep(1000);
	}
	ss.close();
	tracef("register test 1 finished.\n");
}

void regTest2()
{
	tracef("reg test 2 begin: reg send data.");
	StreamSocket ss;
	SocketAddress sa("127.0.0.1", 13333);
	ss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.register";
	DynamicStruct param;
	param["token"] = "1234567890";
	param["uuid"] = "SC000000001";
	ds["param"] = param;

	ss.sendBytes(ds.toString().c_str(), ds.toString().length());
	tracef("reg data send: %s.", ds.toString().c_str());
	ss.close();
	tracef("socket closed.");
	tracef("register test 2 finished.\n");
}

void regTest3()
{
	tracef("reg test 3 begin: reg send and receive data, register fail");
	StreamSocket ss;
	SocketAddress sa("127.0.0.1", 13333);
	ss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.register";
	DynamicStruct param;
	param["token"] = "1234567890";
	param["uuid"] = "SC000000001";
	ds["param"] = param;

	ss.sendBytes(ds.toString().c_str(), ds.toString().length());
	tracef("reg data send: %s.", ds.toString().c_str());
	char buf[1024] = {0, };
	ss.receiveBytes(buf, 1024);
	tracef("receive bytes: %s.", buf);
	tracef("socket closed.");
	tracef("register test 3 finished.\n");
}

void regTest4()
{
	tracef("reg test 4 begin: reg send and receive, register successfully.");
//ssl
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.token";
	Timestamp t;
	UInt64 tms = t.epochMicroseconds();
	char tms_str[32];
	snprintf(tms_str, 31, "%llu", tms);
	std::string key = "alpha2015";
	key += tms_str;
	MD5Engine md5;
	md5.update(key);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key = DigestEngine::digestToHex(digest);
	DynamicStruct param;
	param["key"] = md5key;
	param["timestamp"] = tms_str;
	param["dev_name"] = "lock2";
	param["dev_type"] = "sc-01";
	param["uuid"] = "SC00000002";
	ds["param"] = param;

	tracef("data send: %s.", ds.toString().c_str());
	sss.sendBytes(ds.toString().c_str(), ds.toString().length());

	char buf[1024] = {0, };
	sss.receiveBytes(buf, 1024);

	JSON::Parser parser;
	Dynamic::Var var = parser.parse(buf);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	DynamicStruct ds_recv = *pObj;
	std::string token = ds_recv["param"]["token"];

	DynamicStruct ds_reg;
	ds_reg["type"] = "request";
	ds_reg["action"] = "server.register";
	DynamicStruct param_reg;
	param_reg["token"] = token;
	param_reg["uuid"] = "SC00000002";
	ds_reg["param"] = param_reg;

	StreamSocket ss;
	SocketAddress sa2("127.0.0.1", 13333);
	ss.connect(sa2, Timespan(3, 0));

	ss.sendBytes(ds_reg.toString().c_str(), ds_reg.toString().length());
	tracef("reg data send: %s.", ds_reg.toString().c_str());
	memset(buf, 0, 1024);
	ss.receiveBytes(buf, 1024);
	tracef("receive bytes: %s.", buf);
	tracef("socket closed.");
	tracef("register test 3 finished.");
}

void regTest5()
{
	tracef("reg test 5 begin:reg twice");
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket sss(pContext);
	SocketAddress sa("127.0.0.1", 12222);
	sss.connect(sa, Timespan(3, 0));
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.token";
	Timestamp t;
	UInt64 tms = t.epochMicroseconds();
	char tms_str[32];
	snprintf(tms_str, 31, "%llu", tms);
	std::string key = "alpha2015";
	key += tms_str;
	MD5Engine md5;
	md5.update(key);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5key = DigestEngine::digestToHex(digest);
	DynamicStruct param;
	param["key"] = md5key;
	param["timestamp"] = tms_str;
	param["dev_name"] = "lock3";
	param["dev_type"] = "sc-01";
	param["uuid"] = "SC00000003";
	ds["param"] = param;

	tracef("data send: %s.", ds.toString().c_str());
	sss.sendBytes(ds.toString().c_str(), ds.toString().length());

	char buf[1024] = {0, };
	sss.receiveBytes(buf, 1024);

	JSON::Parser parser;
	Dynamic::Var var = parser.parse(buf);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	DynamicStruct ds_recv = *pObj;
	std::string token = ds_recv["param"]["token"];

	DynamicStruct ds_reg;
	ds_reg["type"] = "request";
	ds_reg["action"] = "server.register";
	DynamicStruct param_reg;
	param_reg["token"] = token;
	param_reg["uuid"] = "SC00000003";
	ds_reg["param"] = param_reg;

	StreamSocket ss;
	SocketAddress sa2("127.0.0.1", 13333);
	ss.connect(sa2, Timespan(3, 0));

	ss.sendBytes(ds_reg.toString().c_str(), ds_reg.toString().length());
	tracef("reg data send: %s.", ds_reg.toString().c_str());
	memset(buf, 0, 1024);
	ss.receiveBytes(buf, 1024);
	tracef("receive bytes: %s.", buf);
	ss.close();
	Thread::sleep(2000);

	StreamSocket ss2;
	ss2.connect(sa2, Timespan(3, 0));
	tracef("reg data send again: %s.", ds_reg.toString().c_str());
	ss2.sendBytes(ds_reg.toString().c_str(), ds_reg.toString().length());
	memset(buf, 0, 1024);
	ss2.receiveBytes(buf, 1024);
	tracef("recv data: %s.", buf);
	ss2.close();
	tracef("reg test 5 finished.");
}

void regTest6()
{
	tracef("reg test 6 begin: reg connect timeout.");
	StreamSocket ss;
	SocketAddress sa("127.0.0.1", 13333);
	ss.connect(sa, Timespan(3, 0));
	for(int i = 0; i < 25; i++)
	{
		tracef("%d seconds...", i + 1);
		Thread::sleep(1000);
	}
	ss.close();
	tracef("register test 6 finished.");
}
int main()
{
	initPrintLogger();
	//CConfigManager* config = CConfigManager::instance();
	//config->init("./Config");
	//CRegServer* reg = CRegServer::instance();
	//CDeviceManager* dev = CDeviceManager::instance();
	//dev->start();
	//reg->start();
	char c = 0;
	while(c != 'q')
	{
		printf("what do you want to do?\n");
		printf("1 ssl test.\n2 reg test.\nq quit.\n");
		c = myGetchar();
		if(c == '1')
		{
			while(c != '0')
			{
				printf("1 ssl connect, disconnect after 3 seconds.\n");
				printf("2 ssl connect, send data and disconnect immediately.\n");
				printf("3 ssl connect, send data, and receive data.\n");
				printf("4 ssl connect, send data and receive data twice.\n");
				printf("5 ssl connect timeout.\n");
				printf("0 return.\n");
				c = myGetchar();
				switch(c)
				{
					case '1':
						sslTest1();
						break;
					case '2':
						sslTest2();
						break;
					case '3':
						sslTest3();
						break;
					case '4':
							sslTest4();
						break;
					case '5':
						sslTest5();
						break;
					default:
						break;
				}
			}
		}
		else if(c == '2')
		{

			while(c != '0')
			{
				printf("1 reg connect, disconnect after 3 seconds.\n");
				printf("2 reg conncet, send data and disconnect immediately.\n");
				printf("3 reg connect, send data and receive data, register fail.\n");
				printf("4 reg register successfully.\n");
				printf("5 reg connect, send data and receive data twice.\n");
				printf("6 reg connect timeout.\n");
				printf("0 return.\n");
				c = myGetchar();
				switch(c)
				{
					case '1':
						regTest1();
						break;
					case '2':
						regTest2();
						break;
					case '3':
						regTest3();
						break;
					case '4':
						regTest4();
						break;
					case '5':
						regTest5();
						break;
					case '6':
						regTest6();
						break;
					default:
						break;
				}
			}
		}
	}
	/*
	sslTest1();
	tracef("ssl test 1 finish, press any key to continue...");
	getchar();

	sslTest2();
	tracef("ssl test 2 finish, press any key to continue...");
	getchar();

	sslTest3();
	tracef("ssl test 3 finish, press any key to continue...");
	getchar();

	sslTest4();
	tracef("ssl test 4 finish, press any key to continue...");
	getchar();

	sslTest5();
	tracef("ssl test 5 finish, press any key to continue...");
	getchar();

	regTest1();
	tracef("reg test 1 finish, press any key to continue...");
	getchar();

	regTest2();
	tracef("reg test 2 finish, press any key to continue...");
	getchar();

	regTest3();
	tracef("reg test 3 finish, press any key to continue...");
	getchar();

	
	serverCheck("hjhjhjhj");
	Thread::sleep(30 * 1000);
	serverCheck("hjhjhjhjhj");
	Thread::sleep(1000);
	userLogin("hjhjhjhj");
	Thread::sleep(1000);
	userLogin("hjhjhjhjhj");
	Thread::sleep(1000);
	userLogin("hjhjhjhjhj");
	Thread::sleep(120 * 1000);
	*/
	//reg->stop();
	return 0;
}


#include <iostream>
#include "Device/RegProxy.h"
#include "Device/RPCServer.h"
#include "Device/Component/UserManager.h"
#include "Poco/Types.h"
#include "Poco/Thread.h"
#include "Common/PrintLog.h"
#include "Poco/Semaphore.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	if(argc < 4)
	{
		std::cout << "argc < 4" << std::endl;
		return -1;
	}
	std::string host = argv[1];
	UInt16 ssl_port = atoi(argv[2]);
	UInt16 port = atoi(argv[3]);
	CUserManager* user = CUserManager::instance();
	user->init();
	CRegProxy* proxy = CRegProxy::instance();
	proxy->setSecureServerInfo(host, ssl_port);
	proxy->setServerInfo(host, port);
	proxy->start();
	CRPCServer* rpc = CRPCServer::instance();
	rpc->start();
	Poco::Semaphore sem(0, 1);
	sem.wait();
	proxy->stop();
	rpc->stop();
	return 0;
}

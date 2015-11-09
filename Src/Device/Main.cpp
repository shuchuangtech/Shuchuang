#include <iostream>
#include "Device/RegProxy.h"
#include "Device/RPCServer.h"
#include "Device/Component/User/UserManager.h"
#include "Poco/Types.h"
#include "Poco/Thread.h"
#include "Common/ConfigManager.h"
#include "Device/SystemManager.h"
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
	initPrintLogger();
	CConfigManager* config = CConfigManager::instance();
	config->init("./global.conf");
	//初始化用户中心
	CUserManager* user = CUserManager::instance();
	user->init();
	//注册到网络服务器
	CRegProxy* proxy = CRegProxy::instance();
	proxy->start();
	//开启rpc server
	CRPCServer* rpc = CRPCServer::instance();
	rpc->start();
	Poco::Semaphore sem(0, 1);
	sem.wait();
	proxy->stop();
	rpc->stop();
	return 0;
}

#include <iostream>
#include "Device/RegProxy.h"
#include "Device/RPCServer.h"
#include "Device/Component/User/UserManager.h"
#include "Device/Component/Task/TaskManager.h"
#include "Device/Component/DeviceController.h"
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
	std::string configPath = "";
	if(argc == 2)
	{
		configPath = argv[1];
	}
	else
	{
		configPath = "./config";
	}
	initPrintLogger();
	//init gpio
	CDeviceController* device = CDeviceController::instance();
	device->openDevice();
	//init config manager
	CConfigManager* config = CConfigManager::instance();
	config->init(configPath.c_str());
	//初始化用户中心
	CUserManager* user = CUserManager::instance();
	user->init();
	//注册到网络服务器
	CRegProxy* proxy = CRegProxy::instance();
	proxy->start();
	//开启rpc server
	CRPCServer* rpc = CRPCServer::instance();
	rpc->start();
	//hold here
	Poco::Semaphore sem(0, 1);
	sem.wait();
	//stop
	proxy->stop();
	rpc->stop();
	CTaskManager* task = CTaskManager::instance();
	task->stopAllTasks();
	return 0;
}


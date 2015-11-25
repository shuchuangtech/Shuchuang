#include <iostream>
#include "Device/RegProxy.h"
#include "Device/RPCServer.h"
#include "Device/Component/User/UserManager.h"
#include "Device/Component/Task/TaskManager.h"
#include "Device/Component/Record/OperationManager.h"
#include "Device/Component/DeviceController.h"
#include "Poco/Types.h"
#include "Poco/Thread.h"
#include "Common/ConfigManager.h"
#include "Device/Network/NetworkManager.h"
#include "Common/PrintLog.h"
#include "Poco/Semaphore.h"
using namespace Poco;
using namespace Poco::Net;
extern const char* getMKTIME();
extern const char* getGITSHA1();
extern const char* getGITDIRTY();
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
	//Pint log < infof on board
#ifdef __SC_ARM__
	setPrintLogLevel(LEVEL_INFO);
#endif
	infof("%s, git version: sha1(%s) dirty(%s)", getMKTIME(), getGITSHA1(), getGITDIRTY());
	//init config manager
	CConfigManager* config = CConfigManager::instance();
	config->init(configPath.c_str());
	CTaskManager* task = CTaskManager::instance();
	//OpManager should before DeviceController
	COpManager* op = COpManager::instance();
#ifdef __SC_ARM__
	op->init("/mnt/nand1-1/Application/oprecord.db");
#else
	op->init("/home/hj/Dev_Env/Shuchuang/oprecord.db");
#endif
	op->start();
	//init gpio
	CDeviceController* device = CDeviceController::instance();
	device->openDevice();
	CNetworkManager* network = CNetworkManager::instance();
	network->startDhcp("eth0");
	//初始化用户中心
	CUserManager* user = CUserManager::instance();
#ifdef __SC_ARM__
	user->init("/mnt/nand1-1/Application/user.db");
#else
	user->init("/home/hj/Dev_Env/Shuchuang/user.db");
#endif
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
	task->stopAllTasks();
	network->stopDhcp("eth0");
	op->stop();
	return 0;
}


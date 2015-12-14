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
extern void registerSignalHandler();
int main(int argc, char** argv)
{
	registerSignalHandler();
	std::string configPath = "";
	std::string logPath = "";
	if(argc >=3)
	{
		configPath = argv[1];
		logPath = argv[2];
	}
	else
	{
#ifdef __SC_ARM__
		configPath = "/mnt/nand1-1/Application/config";
		logPath = "/mnt/nand1-1/Application/log/logfile";
#else
		printf("argc < 3.");
		return 0;
#endif
	}
	initPrintLogger(logPath);
	//Pint log < infof on board
#ifdef __SC_ARM__
	//setPrintLogLevel(LEVEL_INFO);
#endif
	infof("Shuchuang application build at %s, git version: sha1(%s) dirty(%s)", getMKTIME(), getGITSHA1(), getGITDIRTY());
	//init config manager
	CConfigManager* config = CConfigManager::instance();
	config->init(configPath.c_str());
	CTaskManager* task = CTaskManager::instance();
	//OpManager should before DeviceController
	COpManager* op = COpManager::instance();
	JSON::Object::Ptr pDataConfig = NULL;
	config->getConfig("DataPath", pDataConfig);
	std::string userdata = "";
	std::string opdata = "";
	if(!pDataConfig.isNull() && pDataConfig->has("User") && pDataConfig->has("Operation"))
	{
		userdata = pDataConfig->getValue<std::string>("User");
		opdata = pDataConfig->getValue<std::string>("Operation");
	}
	else
	{
#ifdef __SC_ARM__
		userdata = "/mnt/nand1-1/Application/oprecord.db";
		opdata = "/mnt/nand1-1/Application/user.db";
#else
		userdata = "/home/hj/Dev_Env/Shuchuang/user.db";
		opdata = "/home/hj/Dev_Env/Shuchuang/oprecord.db";
#endif
	}
	//初始化用户中心
	CUserManager* user = CUserManager::instance();
	op->init(opdata);
	user->init(userdata);
	op->start();
	pDataConfig = NULL;
	//init gpio
	CDeviceController* device = CDeviceController::instance();
	device->openDevice();
	CNetworkManager* network = CNetworkManager::instance();
	//设置Mac地址
	network->initIf("eth0");
	network->startDhcp("eth0");
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


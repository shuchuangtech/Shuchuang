#include "TransmitServer/RegServer/RegServer.h"
#include "TransmitServer/DeviceManager.h"
#include "TransmitServer/HTTPServer/HTTPServer.h"
#include "TransmitServer/HTTPSAcceptor/HTTPSAcceptor.h"
#include "Common/PrintLog.h"
#include "Poco/Semaphore.h"
#include "Poco/Types.h"
#include "Common/ConfigManager.h"
using namespace Poco;
extern const char* getMKTIME();
extern const char* getGITSHA1();
extern const char* getGITDIRTY();
extern void registerSignalHandler();
int main(int argc, char** argv)
{
	registerSignalHandler();
	//setPrintLogLevel(LEVEL_INFO);
	initPrintLogger("log/serverlog");
	infof("\n\n=================================================================");
	infof("%s, git version: sha1(%s) dirty(%s)", getMKTIME(), getGITSHA1(), getGITDIRTY());
	infof("TransmitServer started now.");
	CConfigManager* config = CConfigManager::instance();
	config->init("./Config");
	CDeviceManager* device_manager = CDeviceManager::instance();
	device_manager->start();
	CRegServer* reg_server = CRegServer::instance();
	reg_server->start();
	CHTTPServer* http_server = CHTTPServer::instance();
	http_server->start();
	CHTTPSAcceptor* https_acceptor = CHTTPSAcceptor::instance();
	https_acceptor->start();
	try{
		Poco::Semaphore sem(0, 1);
		sem.wait();
	}
	catch(Exception& e)
	{
		tracef("%s", e.message().c_str());
	}
	https_acceptor->stop();
	http_server->stop();
	reg_server->stop();
	device_manager->stop();
	return 0;
}


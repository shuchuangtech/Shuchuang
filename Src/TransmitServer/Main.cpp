#include "TransmitServer/RegServer.h"
#include "TransmitServer/DeviceManager.h"
#include "TransmitServer/HTTPServer.h"
#include "TransmitServer/HTTPSAcceptor.h"
#include "Common/PrintLog.h"
#include "Poco/Semaphore.h"
#include "Poco/Types.h"
using namespace Poco;
int main(int argc, char** argv)
{
	if(argc < 5)
	{
		errorf("argc < 5.\n"
				"ssl_port "
				"reg_port "
				"http_port "
				"https_port\n");
		return 0;
	}
	//setPrintLogLevel(LEVEL_INFO);	
	UInt16 ssl_port = atoi(argv[1]);
	UInt16 reg_port = atoi(argv[2]);
	UInt16 http_port = atoi(argv[3]);
	UInt16 https_port = atoi(argv[4]);
	CDeviceManager* device_manager = CDeviceManager::instance();
	device_manager->start();
	CRegServer* reg_server = CRegServer::instance();
	reg_server->setServerPort(ssl_port, reg_port);
	reg_server->start();
	CHTTPServer* http_server = CHTTPServer::instance();
	http_server->setPort(http_port);
	http_server->start();
	CHTTPSAcceptor* https_acceptor = CHTTPSAcceptor::instance();
	https_acceptor->setPort(https_port, http_port);
	https_acceptor->start();
	try{
		Poco::Semaphore sem(0, 1);
		sem.wait();
	}
	catch(Exception& e)
	{
		tracef("%s\n", e.message().c_str());
	}
	https_acceptor->stop();
	http_server->stop();
	reg_server->stop();
	device_manager->stop();
	return 0;
}


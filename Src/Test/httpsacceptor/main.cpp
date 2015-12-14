#include "TransmitServer/HTTPSAcceptor/HTTPSAcceptor.h"
#include "TransmitServer/HTTPServer/HTTPServer.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Common/PrintLog.h"
#include "Poco/Types.h"
#include "Poco/Semaphore.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	initPrintLogger("log/test.log");
	CHTTPSAcceptor* https_acceptor = CHTTPSAcceptor::instance();
	https_acceptor->start();
	CHTTPServer* http_server = CHTTPServer::instance();
	http_server->start();
	Semaphore sem(0, 1);
	sem.wait();
	http_server->stop();
	https_acceptor->stop();
	return 0;
}


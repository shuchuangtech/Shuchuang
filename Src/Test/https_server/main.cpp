#include "TransmitServer/HTTPServer/HTTPServer.h"
#include "Common/PrintLog.h"
#include "Poco/Semaphore.h"
int main()
{
	initPrintLogger("");
	CHTTPServer * http = CHTTPServer::instance();
	http->start();
	Poco::Semaphore sem(0, 1);
	sem.wait();
	return 0;
}


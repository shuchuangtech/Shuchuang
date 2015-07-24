#include "TransmitServer/HTTPSAcceptor.h"
#include "Poco/Types.h"
using namespace Poco;
int main(int argc, char** argv)
{
	UInt16 port = atoi(argv[1]);
	CHTTPSAcceptor* https_acceptor = CHTTPSAcceptor::instance();
	https_acceptor->setPort(port, 8888);
	https_acceptor->start();
	Thread::sleep(60 * 1000);
	return 0;
}


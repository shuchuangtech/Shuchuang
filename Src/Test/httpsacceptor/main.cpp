#include "TransmitServer/HTTPSAcceptor.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Common/ConfigManager.h"
#include "Poco/Types.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	CConfigManager* config = CConfigManager::instance();
	config->init("./Config");
	CHTTPSAcceptor* https_acceptor = CHTTPSAcceptor::instance();
	https_acceptor->start();
	Thread::sleep(3 * 1000);
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SocketAddress sa("127.0.0.1", 9999);
	SecureStreamSocket ss(sa, pContext);
	Thread::sleep(35 * 1000);
	https_acceptor->stop();
	return 0;
}


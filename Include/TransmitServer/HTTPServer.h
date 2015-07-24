#ifndef __SERVER_HTTP_SERVER_H__
#define __SERVER_HTTP_SERVER_H__
#include "Poco/Net/HTTPServer.h"
#include "Poco/Types.h"
#include "Poco/SingletonHolder.h"
using namespace Poco;
using namespace Poco::Net;
class CHTTPServer
{
public:
	CHTTPServer();
	~CHTTPServer();
	static CHTTPServer* instance()
	{
		static SingletonHolder<CHTTPServer> sh;
		return sh.get();
	}
	bool start();
	bool stop();
	bool setPort(UInt16 port);
private:
	bool			m_started;
	HTTPServer*		m_svr;
	UInt16			m_port;
};
#endif


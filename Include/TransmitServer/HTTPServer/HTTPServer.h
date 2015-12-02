#ifndef __SERVER_HTTP_SERVER_H__
#define __SERVER_HTTP_SERVER_H__
#include "Poco/Net/HTTPServer.h"
#include "Poco/Types.h"
#include "Poco/SingletonHolder.h"
class CHTTPServer
{
public:
	CHTTPServer();
	~CHTTPServer();
	static CHTTPServer* instance()
	{
		static Poco::SingletonHolder<CHTTPServer> sh;
		return sh.get();
	}
	bool start();
	bool stop();
	bool setPort(Poco::UInt16 port);
private:
	bool						m_started;
	Poco::Net::HTTPServer*		m_svr;
	Poco::UInt16				m_port;
};
#endif


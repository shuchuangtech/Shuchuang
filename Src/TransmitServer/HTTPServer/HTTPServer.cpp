#include "TransmitServer/HTTPServer.h"
#include "TransmitServer/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Common/PrintLog.h"
CHTTPServer::CHTTPServer()
{
	m_started = false;
	m_svr = NULL;
	m_port = 0;
}

CHTTPServer::~CHTTPServer()
{
	if(m_svr != NULL)
		delete m_svr;
}

bool CHTTPServer::setPort(UInt16 port)
{
	m_port = port;
	return true;
}

bool CHTTPServer::start()
{
	if(m_started || m_svr != NULL || m_port > 65535)
	{
		warnf("%s, %d: HTTP server start failed.\n", __FILE__, __LINE__);
		return false;
	}
	HTTPServerParams* pParams = new HTTPServerParams;
	pParams->setMaxQueued(100);
	pParams->setMaxThreads(16);
	ServerSocket svs(m_port);
	m_svr = new HTTPServer(new CHTTPRequestHandlerFactory(), svs, pParams);
	m_svr->start();
	m_started = true;
	infof("%s, %d: HTTP server start successfully.\n", __FILE__, __LINE__);
	return true;
}

bool CHTTPServer::stop()
{
	if(!m_started || m_svr == NULL)
	{
		warnf("%s, %d: HTTP server stop failed.\n", __FILE__, __LINE__);
		return false;
	}
	m_svr->stop();
	m_started = false;
	infof("%s, %d: HTTP server stop successfully.\n", __FILE__, __LINE__);
	return true;
}


#include "TransmitServer/HTTPServer/HTTPServer.h"
#include "TransmitServer/HTTPServer/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Common/PrintLog.h"
#include "Common/ConfigManager.h"
using namespace Poco;
using namespace Poco::Net;
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
		warnf("%s, %d: HTTP server start failed.", __FILE__, __LINE__);
		return false;
	}
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pConfig;
	config->getConfig("HTTPServer", pConfig);
	if(pConfig.isNull())
	{
		m_port = 8888;
		pConfig = new JSON::Object;
		pConfig->set("port", 8888);
		config->setConfig("HTTPServer", pConfig);
	}
	else
	{
		if(pConfig->has("port"))
		{
			m_port = pConfig->getValue<UInt16>("port");
		}
		else
		{
			m_port = 8888;
			pConfig = NULL;
			pConfig = new JSON::Object;
			pConfig->set("port", 8888);
			config->setConfig("HTTPServer", pConfig);
		}
	}
	HTTPServerParams* pParams = new HTTPServerParams;
	pParams->setMaxQueued(100);
	pParams->setMaxThreads(16);
	ServerSocket svs(m_port);
	m_svr = new HTTPServer(new CHTTPRequestHandlerFactory(), svs, pParams);
	m_svr->start();
	m_started = true;
	infof("%s, %d: HTTP server start successfully.", __FILE__, __LINE__);
	infof("%s, %d: HTTP server port:%d", __FILE__, __LINE__, m_port);
	return true;
}

bool CHTTPServer::stop()
{
	if(!m_started || m_svr == NULL)
	{
		warnf("%s, %d: HTTP server stop failed.", __FILE__, __LINE__);
		return false;
	}
	m_svr->stop();
	m_started = false;
	infof("%s, %d: HTTP server stop successfully.", __FILE__, __LINE__);
	return true;
}


#include "TransmitServer/HTTPServer/HTTPServer.h"
#include "TransmitServer/HTTPServer/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Common/PrintLog.h"
#include "Common/ConfigManager.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureServerSocket.h"
using namespace Poco;
using namespace Poco::Net;
CHTTPServer::CHTTPServer()
{
	m_started = false;
	m_svr = NULL;
}

CHTTPServer::~CHTTPServer()
{
	if(m_svr != NULL)
		delete m_svr;
}

bool CHTTPServer::start()
{
	if(m_started || m_svr != NULL)
	{
		warnf("%s, %d: HTTP server start failed.", __FILE__, __LINE__);
		return false;
	}
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pConfig;
	config->getConfig("HTTPServer", pConfig);
	UInt16 port = 0;
	std::string keyPath = "";
	std::string certPath = "";
	if(pConfig.isNull())
	{
		errorf("%s, %d: HTTPServer config does not exists.", __FILE__, __LINE__);
		return false;
	}
	else
	{
		if(!pConfig->has("port") || !pConfig->has("cert") || !pConfig->has("privkey"))
		{
			errorf("%s, %d: HTTPServer config error, missing 'port', 'cert' or 'privkey'.", __FILE__, __LINE__);
			return false;
		}
		else
		{
			port = (UInt16)pConfig->getValue<int>("port");
			certPath = pConfig->getValue<std::string>("cert");
			keyPath = pConfig->getValue<std::string>("privkey");
		}
	}
	HTTPServerParams::Ptr pParams = new HTTPServerParams;
	pParams->setMaxQueued(100);
	pParams->setMaxThreads(16);
	Context::Ptr pContext = new Context(Context::SERVER_USE,
									keyPath,
									certPath,
									"",
									Context::VERIFY_RELAXED,
									9,
									false,
									"HIGH");
	//support ECDH
	SSL_CONF_CTX * cctx = NULL;
	cctx = SSL_CONF_CTX_new();
	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_SERVER);
	SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CMDLINE);
	SSL_CONF_CTX_set_ssl_ctx(cctx, pContext->sslContext());
	if(SSL_CONF_cmd(cctx, "-named_curve", "P-256") <= 0)
	{
		warnf("%s, %d: Error setting Ec curve.", __FILE__, __LINE__);
	}
	if(!SSL_CONF_CTX_finish(cctx))
	{
		warnf("%s, %d: Error finishing config context.", __FILE__, __LINE__);
	}
	SSL_CONF_CTX_free(cctx);
	//end of support ECDH	
	SecureServerSocket svr(port, 64, pContext);

	m_svr = new HTTPServer(new CHTTPRequestHandlerFactory(), svr, pParams);
	m_svr->start();
	m_started = true;
	infof("%s, %d: HTTP server start successfully.", __FILE__, __LINE__);
	infof("%s, %d: HTTP server port:%d", __FILE__, __LINE__, port);
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


#ifndef __SERVER_HTTP_REQUEST_HANDLER_FACTORY_H__
#define __SERVER_HTTP_REQUEST_HANDLER_FACTORY_H__
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
using namespace Poco;
using namespace Poco::Net;
class CHTTPRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	CHTTPRequestHandlerFactory();
	~CHTTPRequestHandlerFactory();
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request);
};
#endif


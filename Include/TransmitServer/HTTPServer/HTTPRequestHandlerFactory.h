#ifndef __SERVER_HTTP_REQUEST_HANDLER_FACTORY_H__
#define __SERVER_HTTP_REQUEST_HANDLER_FACTORY_H__
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
class CHTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
	CHTTPRequestHandlerFactory();
	~CHTTPRequestHandlerFactory();
	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);
};
#endif

